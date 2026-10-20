// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "TimelineDAG.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>

#include <iostream>

namespace androcad {

TimelineDAG::TimelineDAG() {
    // Set standard unit length constants
    evaluatedParameters["PI"] = 3.14159265358979323846;
    evaluatedParameters["E"] = 2.71828182845904523536;
}

void TimelineDAG::SetParameter(const std::string& name, const std::string& expression) {
    userParameters[name] = expression;
}

bool TimelineDAG::HasParameter(const std::string& name) const {
    return userParameters.find(name) != userParameters.end();
}

double TimelineDAG::GetParameterValue(const std::string& name) const {
    auto it = evaluatedParameters.find(name);
    return it != evaluatedParameters.end() ? it->second : 0.0;
}

std::string TimelineDAG::GetParameterExpression(const std::string& name) const {
    auto it = userParameters.find(name);
    return it != userParameters.end() ? it->second : "";
}

bool TimelineDAG::EvaluateAllParameters() {
    auto results = ExpressionParser::EvaluateAll(userParameters, evaluatedParameters);
    bool allValid = true;
    for (const auto& [name, res] : results) {
        if (res.success) {
            evaluatedParameters[name] = res.value;
        } else {
            allValid = false;
        }
    }
    return allValid;
}

uint32_t TimelineDAG::AddBoxNode(const std::string& name, const std::string& dx, const std::string& dy, const std::string& dz) {
    FeatureNode node;
    node.id = nextNodeId++;
    node.name = name;
    node.type = FeatureNodeType::Box;
    node.paramExpressions["dx"] = dx;
    node.paramExpressions["dy"] = dy;
    node.paramExpressions["dz"] = dz;
    nodes.push_back(node);
    return node.id;
}

uint32_t TimelineDAG::AddSketchNode(const std::string& name, std::shared_ptr<Sketcher> sketcher) {
    FeatureNode node;
    node.id = nextNodeId++;
    node.name = name;
    node.type = FeatureNodeType::Sketch;
    node.sketcher = sketcher;
    nodes.push_back(node);
    return node.id;
}

uint32_t TimelineDAG::AddExtrudeNode(const std::string& name, uint32_t sketchNodeId, const std::string& dx, const std::string& dy, const std::string& dz) {
    FeatureNode node;
    node.id = nextNodeId++;
    node.name = name;
    node.type = FeatureNodeType::Extrude;
    node.paramExpressions["dx"] = dx;
    node.paramExpressions["dy"] = dy;
    node.paramExpressions["dz"] = dz;
    node.paramExpressions["sketchId"] = std::to_string(sketchNodeId);
    nodes.push_back(node);
    return node.id;
}

uint32_t TimelineDAG::AddFilletNode(const std::string& name, const TopologicalReference& edgeRef, const std::string& radius) {
    FeatureNode node;
    node.id = nextNodeId++;
    node.name = name;
    node.type = FeatureNodeType::Fillet;
    node.paramExpressions["radius"] = radius;
    node.references["edge"] = edgeRef;
    nodes.push_back(node);
    return node.id;
}

uint32_t TimelineDAG::AddChamferNode(const std::string& name, const TopologicalReference& edgeRef, const std::string& distance) {
    FeatureNode node;
    node.id = nextNodeId++;
    node.name = name;
    node.type = FeatureNodeType::Chamfer;
    node.paramExpressions["distance"] = distance;
    node.references["edge"] = edgeRef;
    nodes.push_back(node);
    return node.id;
}

uint32_t TimelineDAG::AddHoleNode(const std::string& name, const TopologicalReference& faceRef,
                                const std::string& px, const std::string& py, const std::string& pz,
                                const std::string& dx, const std::string& dy, const std::string& dz,
                                const std::string& radius, const std::string& depth) {
    FeatureNode node;
    node.id = nextNodeId++;
    node.name = name;
    node.type = FeatureNodeType::Hole;
    node.paramExpressions["px"] = px;
    node.paramExpressions["py"] = py;
    node.paramExpressions["pz"] = pz;
    node.paramExpressions["dx"] = dx;
    node.paramExpressions["dy"] = dy;
    node.paramExpressions["dz"] = dz;
    node.paramExpressions["radius"] = radius;
    node.paramExpressions["depth"] = depth;
    node.references["face"] = faceRef;
    nodes.push_back(node);
    return node.id;
}

uint32_t TimelineDAG::AddShellNode(const std::string& name, const TopologicalReference& removeFaceRef, const std::string& thickness) {
    FeatureNode node;
    node.id = nextNodeId++;
    node.name = name;
    node.type = FeatureNodeType::Shell;
    node.paramExpressions["thickness"] = thickness;
    node.references["removeFace"] = removeFaceRef;
    nodes.push_back(node);
    return node.id;
}

uint32_t TimelineDAG::AddCombineNode(const std::string& name, uint32_t bodyNodeA, uint32_t bodyNodeB, BooleanType type) {
    FeatureNode node;
    node.id = nextNodeId++;
    node.name = name;
    node.type = FeatureNodeType::Combine;
    node.paramExpressions["bodyA"] = std::to_string(bodyNodeA);
    node.paramExpressions["bodyB"] = std::to_string(bodyNodeB);
    node.paramExpressions["op"] = std::to_string(static_cast<int>(type));
    nodes.push_back(node);
    return node.id;
}

void TimelineDAG::SetRollbackIndex(int index) {
    if (index < -1) index = -1;
    if (index >= static_cast<int>(nodes.size())) index = static_cast<int>(nodes.size()) - 1;
    rollbackIndex = index;
}

bool TimelineDAG::Rebuild() {
    EvaluateAllParameters();

    size_t effectiveCount = nodes.size();
    if (rollbackIndex >= 0 && rollbackIndex < static_cast<int>(nodes.size())) {
        effectiveCount = static_cast<size_t>(rollbackIndex + 1);
    }

    TopoDS_Shape activeModel;
    ExpressionParser parser;
    for (const auto& [name, val] : evaluatedParameters) {
        parser.SetVariable(name, val);
    }

    for (size_t i = 0; i < effectiveCount; ++i) {
        FeatureNode& node = nodes[i];

        // Evaluate all parameters for this node
        for (const auto& [pName, pExpr] : node.paramExpressions) {
            auto res = parser.Evaluate(pExpr);
            if (res.success) {
                node.evaluatedParams[pName] = res.value;
            } else {
                node.state = NodeState::ExecutionFailed;
                node.errorMessage = "Failed to evaluate parameter '" + pName + "': " + res.errorMessage;
                return false;
            }
        }

        switch (node.type) {
            case FeatureNodeType::Box: {
                double dx = node.evaluatedParams["dx"];
                double dy = node.evaluatedParams["dy"];
                double dz = node.evaluatedParams["dz"];
                BRepPrimAPI_MakeBox boxMaker(dx, dy, dz);
                boxMaker.Build();
                if (boxMaker.IsDone()) {
                    node.outputShape = boxMaker.Shape();
                    node.outputMesh = FeatureOps::ShapeToMeshPayload(node.outputShape);
                    node.state = NodeState::Valid;
                    activeModel = node.outputShape;
                } else {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = "BRepPrimAPI_MakeBox failed.";
                    return false;
                }
                break;
            }
            case FeatureNodeType::Sketch: {
                if (node.sketcher) {
                    node.sketchResult = node.sketcher->BuildProfileFace();
                    if (node.sketchResult.success) {
                        node.outputShape = node.sketchResult.face;
                        node.state = NodeState::Valid;
                    } else {
                        node.state = NodeState::ExecutionFailed;
                        node.errorMessage = node.sketchResult.errorMessage;
                        return false;
                    }
                }
                break;
            }
            case FeatureNodeType::Extrude: {
                uint32_t sketchId = static_cast<uint32_t>(node.evaluatedParams["sketchId"]);
                const FeatureNode* skNode = GetNode(sketchId);
                if (!skNode || skNode->outputShape.IsNull()) {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = "Invalid sketch reference.";
                    return false;
                }

                double dx = node.evaluatedParams["dx"];
                double dy = node.evaluatedParams["dy"];
                double dz = node.evaluatedParams["dz"];
                TopoDS_Face profileFace = TopoDS::Face(skNode->outputShape);
                node.outputMesh = FeatureOps::MakeExtrude(profileFace, dx, dy, dz, node.outputShape);
                if (node.outputMesh.success) {
                    node.state = NodeState::Valid;
                    activeModel = node.outputShape;
                } else {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = node.outputMesh.errorMessage;
                    return false;
                }
                break;
            }
            case FeatureNodeType::Fillet: {
                if (activeModel.IsNull()) {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = "No active solid to fillet.";
                    return false;
                }

                // Resolve target edge via TNP
                TopoDS_Edge resolvedEdge;
                auto tnpRes = TNPResolver::ResolveEdge(activeModel, node.references["edge"], resolvedEdge);
                if (tnpRes.status == TNPResolutionStatus::BrokenReference) {
                    node.state = NodeState::BrokenReference;
                    node.errorMessage = tnpRes.diagnosticMessage;
                    return false;
                }

                // Find 1-based edgeId in activeModel
                TopTools_IndexedMapOfShape edgeMap;
                TopExp::MapShapes(activeModel, TopAbs_EDGE, edgeMap);
                int edgeIdx = -1;
                for (int e = 1; e <= edgeMap.Extent(); ++e) {
                    if (edgeMap(e).IsSame(resolvedEdge)) {
                        edgeIdx = e;
                        break;
                    }
                }

                double radius = node.evaluatedParams["radius"];
                node.outputMesh = FeatureOps::MakeFillet(activeModel, radius, edgeIdx, node.outputShape);
                if (node.outputMesh.success) {
                    node.state = NodeState::Valid;
                    activeModel = node.outputShape;
                } else {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = node.outputMesh.errorMessage;
                    return false;
                }
                break;
            }
            case FeatureNodeType::Chamfer: {
                if (activeModel.IsNull()) {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = "No active solid to chamfer.";
                    return false;
                }

                TopoDS_Edge resolvedEdge;
                auto tnpRes = TNPResolver::ResolveEdge(activeModel, node.references["edge"], resolvedEdge);
                if (tnpRes.status == TNPResolutionStatus::BrokenReference) {
                    node.state = NodeState::BrokenReference;
                    node.errorMessage = tnpRes.diagnosticMessage;
                    return false;
                }

                TopTools_IndexedMapOfShape edgeMap;
                TopExp::MapShapes(activeModel, TopAbs_EDGE, edgeMap);
                int edgeIdx = 1;
                for (int e = 1; e <= edgeMap.Extent(); ++e) {
                    if (edgeMap(e).IsSame(resolvedEdge)) {
                        edgeIdx = e;
                        break;
                    }
                }

                double distance = node.evaluatedParams["distance"];
                node.outputMesh = FeatureOps::MakeChamfer(activeModel, distance, edgeIdx, node.outputShape);
                if (node.outputMesh.success) {
                    node.state = NodeState::Valid;
                    activeModel = node.outputShape;
                } else {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = node.outputMesh.errorMessage;
                    return false;
                }
                break;
            }
            case FeatureNodeType::Hole: {
                if (activeModel.IsNull()) {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = "No active solid to drill hole.";
                    return false;
                }

                double px = node.evaluatedParams["px"];
                double py = node.evaluatedParams["py"];
                double pz = node.evaluatedParams["pz"];
                double dx = node.evaluatedParams["dx"];
                double dy = node.evaluatedParams["dy"];
                double dz = node.evaluatedParams["dz"];
                double radius = node.evaluatedParams["radius"];
                double depth = node.evaluatedParams["depth"];

                node.outputMesh = FeatureOps::MakeHole(activeModel, px, py, pz, dx, dy, dz, radius, depth, node.outputShape);
                if (node.outputMesh.success) {
                    node.state = NodeState::Valid;
                    activeModel = node.outputShape;
                } else {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = node.outputMesh.errorMessage;
                    return false;
                }
                break;
            }
            case FeatureNodeType::Shell: {
                if (activeModel.IsNull()) {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = "No active solid to hollow.";
                    return false;
                }

                int removeFaceIdx = 1;
                if (node.references.find("removeFace") != node.references.end()) {
                    TopoDS_Face resolvedFace;
                    auto tnpRes = TNPResolver::ResolveFace(activeModel, node.references["removeFace"], resolvedFace);
                    if (tnpRes.status != TNPResolutionStatus::BrokenReference) {
                        TopTools_IndexedMapOfShape faceMap;
                        TopExp::MapShapes(activeModel, TopAbs_FACE, faceMap);
                        for (int f = 1; f <= faceMap.Extent(); ++f) {
                            if (faceMap(f).IsSame(resolvedFace)) {
                                removeFaceIdx = f;
                                break;
                            }
                        }
                    }
                }

                double thickness = node.evaluatedParams["thickness"];
                node.outputMesh = FeatureOps::MakeShell(activeModel, thickness, removeFaceIdx, node.outputShape);
                if (node.outputMesh.success) {
                    node.state = NodeState::Valid;
                    activeModel = node.outputShape;
                } else {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = node.outputMesh.errorMessage;
                    return false;
                }
                break;
            }
            case FeatureNodeType::Combine: {
                uint32_t bodyAId = static_cast<uint32_t>(node.evaluatedParams["bodyA"]);
                uint32_t bodyBId = static_cast<uint32_t>(node.evaluatedParams["bodyB"]);
                const FeatureNode* nodeA = GetNode(bodyAId);
                const FeatureNode* nodeB = GetNode(bodyBId);

                if (!nodeA || !nodeB || nodeA->outputShape.IsNull() || nodeB->outputShape.IsNull()) {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = "Invalid body references for Combine operation.";
                    return false;
                }

                BooleanType op = static_cast<BooleanType>(static_cast<int>(node.evaluatedParams["op"]));
                node.outputMesh = FeatureOps::MakeBoolean(nodeA->outputShape, nodeB->outputShape, op, 1e-5, node.outputShape);
                if (node.outputMesh.success) {
                    node.state = NodeState::Valid;
                    activeModel = node.outputShape;
                } else {
                    node.state = NodeState::ExecutionFailed;
                    node.errorMessage = node.outputMesh.errorMessage;
                    return false;
                }
                break;
            }
        }
    }

    return true;
}

const FeatureNode* TimelineDAG::GetNode(uint32_t id) const {
    for (const auto& node : nodes) {
        if (node.id == id) return &node;
    }
    return nullptr;
}

const FeatureNode* TimelineDAG::GetActiveNode() const {
    if (nodes.empty()) return nullptr;
    if (rollbackIndex >= 0 && rollbackIndex < static_cast<int>(nodes.size())) {
        return &nodes[rollbackIndex];
    }
    return &nodes.back();
}

TopoDS_Shape TimelineDAG::GetActiveShape() const {
    const FeatureNode* node = GetActiveNode();
    return node ? node->outputShape : TopoDS_Shape();
}

MeshPayload TimelineDAG::GetActiveMesh() const {
    const FeatureNode* node = GetActiveNode();
    return node ? node->outputMesh : MeshPayload();
}

bool TimelineDAG::IsAllValid() const {
    size_t effectiveCount = nodes.size();
    if (rollbackIndex >= 0 && rollbackIndex < static_cast<int>(nodes.size())) {
        effectiveCount = static_cast<size_t>(rollbackIndex + 1);
    }
    for (size_t i = 0; i < effectiveCount; ++i) {
        if (nodes[i].state != NodeState::Valid) return false;
    }
    return true;
}

} // namespace androcad
