// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_TNP_RESOLVER_H
#define ANDROCAD_TNP_RESOLVER_H

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepTools_History.hxx>

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace androcad {

enum class SubShapeType {
    Face,
    Edge,
    Vertex
};

struct GeometricFingerprint {
    SubShapeType type;
    int geomType; // 0: Planar/Line, 1: Cylindrical/Circle, 2: Other
    double center[3]; // Center of gravity or curve midpoint
    double normal[3]; // Face normal or edge tangent
    double size;      // Area for face, length for edge
    int subEntityCount; // Number of edges for face, vertices for edge

    static GeometricFingerprint FromFace(const TopoDS_Face& face);
    static GeometricFingerprint FromEdge(const TopoDS_Edge& edge);
};

struct TopologicalReference {
    std::string semanticTag;      // Tier 1: e.g. "Extrude_1:Cap_End", "Fillet_1:Rim"
    GeometricFingerprint fingerprint; // Tier 3: Invariant geometric characteristics
    uint32_t lastKnownLocalId = 0;
};

enum class TNPResolutionStatus {
    ExactMatch,       // 100% confidence via history or unambiguous semantic tag
    FingerprintMatch, // Resolved via closest geometric fingerprint match
    BrokenReference   // Entity vanished or ambiguous beyond safety threshold
};

struct TNPResolutionResult {
    TNPResolutionStatus status;
    double confidence; // 0.0 to 1.0
    std::string diagnosticMessage;
    TopoDS_Shape resolvedShape;
};

class TNPResolver {
public:
    TNPResolver() = default;
    ~TNPResolver() = default;

    // Generates a reference for a given face in a model
    static TopologicalReference CreateFaceReference(
        const TopoDS_Shape& model,
        const TopoDS_Face& face,
        const std::string& semanticTag
    );

    // Generates a reference for a given edge in a model
    static TopologicalReference CreateEdgeReference(
        const TopoDS_Shape& model,
        const TopoDS_Edge& edge,
        const std::string& semanticTag
    );

    // Resolves a target face in a rebuilt model using the 3-tier strategy
    static TNPResolutionResult ResolveFace(
        const TopoDS_Shape& currentModel,
        const TopologicalReference& reference,
        TopoDS_Face& outFace,
        const Handle(BRepTools_History)& history = Handle(BRepTools_History)()
    );

    // Resolves a target edge in a rebuilt model using the 3-tier strategy
    static TNPResolutionResult ResolveEdge(
        const TopoDS_Shape& currentModel,
        const TopologicalReference& reference,
        TopoDS_Edge& outEdge,
        const Handle(BRepTools_History)& history = Handle(BRepTools_History)()
    );

    // Analyzes an extruded shape and assigns standard semantic tags to its faces:
    // "Cap_Start", "Cap_End", "Lateral_1", "Lateral_2", ...
    static std::unordered_map<std::string, TopoDS_Face> TagExtrudeFaces(
        const TopoDS_Shape& solid,
        double dx, double dy, double dz
    );
};

} // namespace androcad

#endif // ANDROCAD_TNP_RESOLVER_H
