// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_TIMELINE_DAG_H
#define ANDROCAD_TIMELINE_DAG_H

#include "AndroCadKernel.h"
#include "Sketcher.h"
#include "FeatureOps.h"
#include "ExpressionParser.h"
#include "TNPResolver.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace androcad {

enum class FeatureNodeType {
    Box,
    Sketch,
    Extrude,
    Revolve,
    Fillet,
    Chamfer,
    Hole,
    Shell,
    Combine
};

enum class NodeState {
    Valid,
    NeedsRecompute,
    BrokenReference,
    ExecutionFailed
};

struct FeatureNode {
    uint32_t id;
    std::string name;
    FeatureNodeType type;
    std::unordered_map<std::string, std::string> paramExpressions;
    std::unordered_map<std::string, double> evaluatedParams;
    std::unordered_map<std::string, TopologicalReference> references;

    NodeState state = NodeState::NeedsRecompute;
    std::string errorMessage;

    TopoDS_Shape outputShape;
    MeshPayload outputMesh;

    // Optional for Sketch nodes
    std::shared_ptr<Sketcher> sketcher;
    SketchResult sketchResult;
};

class TimelineDAG {
public:
    TimelineDAG();
    ~TimelineDAG() = default;

    // 1. User Parameters Management
    void SetParameter(const std::string& name, const std::string& expression);
    bool HasParameter(const std::string& name) const;
    double GetParameterValue(const std::string& name) const;
    std::string GetParameterExpression(const std::string& name) const;
    const std::unordered_map<std::string, std::string>& GetAllParameters() const { return userParameters; }

    // 2. Timeline Nodes Management
    uint32_t AddBoxNode(const std::string& name, const std::string& dx, const std::string& dy, const std::string& dz);
    uint32_t AddSketchNode(const std::string& name, std::shared_ptr<Sketcher> sketcher);
    uint32_t AddExtrudeNode(const std::string& name, uint32_t sketchNodeId, const std::string& dx, const std::string& dy, const std::string& dz);
    uint32_t AddFilletNode(const std::string& name, const TopologicalReference& edgeRef, const std::string& radius);
    uint32_t AddChamferNode(const std::string& name, const TopologicalReference& edgeRef, const std::string& distance);
    uint32_t AddHoleNode(const std::string& name, const TopologicalReference& faceRef,
                        const std::string& px, const std::string& py, const std::string& pz,
                        const std::string& dx, const std::string& dy, const std::string& dz,
                        const std::string& radius, const std::string& depth);
    uint32_t AddShellNode(const std::string& name, const TopologicalReference& removeFaceRef, const std::string& thickness);
    uint32_t AddCombineNode(const std::string& name, uint32_t bodyNodeA, uint32_t bodyNodeB, BooleanType type);

    // 3. Rollback & History Control
    int GetNodeCount() const { return static_cast<int>(nodes.size()); }
    int GetRollbackIndex() const { return rollbackIndex; }
    void SetRollbackIndex(int index); // -1 means all nodes active

    // 4. Deterministic Rebuild
    bool Rebuild();

    // 5. Query Active State
    const FeatureNode* GetNode(uint32_t id) const;
    const FeatureNode* GetActiveNode() const;
    TopoDS_Shape GetActiveShape() const;
    MeshPayload GetActiveMesh() const;
    bool IsAllValid() const;

private:
    std::unordered_map<std::string, std::string> userParameters;
    std::unordered_map<std::string, double> evaluatedParameters;
    std::vector<FeatureNode> nodes;
    int rollbackIndex = -1;
    uint32_t nextNodeId = 1;

    bool EvaluateAllParameters();
};

} // namespace androcad

#endif // ANDROCAD_TIMELINE_DAG_H
