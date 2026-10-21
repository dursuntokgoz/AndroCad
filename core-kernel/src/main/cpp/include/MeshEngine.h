// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_MESH_ENGINE_H
#define ANDROCAD_MESH_ENGINE_H

#include "AndroCadKernel.h"
#include <TopoDS_Shape.hxx>

#include <vector>
#include <string>
#include <cstdint>

namespace androcad {

struct RawMeshData {
    std::vector<float> positions;  // 3 floats per vertex (X, Y, Z)
    std::vector<uint32_t> indices; // 3 indices per triangle (CCW)
    bool isWatertight{false};
    int nonManifoldEdges{0};
    int degenerateTriangles{0};
};

struct MeshStats {
    uint32_t vertexCount{0};
    uint32_t triangleCount{0};
    uint32_t edgeCount{0};
    double surfaceArea{0.0};
    double enclosedVolume{0.0};
    bool isClosedManifold{false};
    int eulerCharacteristic{0}; // V - E + F
};

class MeshEngine {
public:
    // 1. Analyzes mesh topology, counts boundary loops, degenerate triangles, and computes Euler characteristic
    static MeshStats AnalyzeMesh(const RawMeshData& mesh);

    // 2. Repairs mesh: welds co-located vertices (within epsilon) and removes degenerate/collapsed triangles
    static RawMeshData RepairMesh(const RawMeshData& mesh, double weldTolerance = 1e-4);

    // 3. Smooths mesh using volume-preserving Laplacian diffusion filter
    static RawMeshData SmoothMesh(const RawMeshData& mesh, int iterations = 3, float lambda = 0.5f);

    // 4. Decimates mesh triangles while preserving sharp geometric boundaries
    static RawMeshData DecimateMesh(const RawMeshData& mesh, float targetReductionRatio = 0.5f);

    // 5. Converts a triangle mesh into a watertight OCCT TopoDS_Shape (B-Rep Shell / Solid)
    static TopoDS_Shape ConvertMeshToSolid(const RawMeshData& mesh, double tolerance = 1e-3);

    // 6. Extracts RawMeshData from an existing OCCT TopoDS_Shape
    static RawMeshData ExtractFromShape(const TopoDS_Shape& shape,
                                        double linearDeflection = 0.1,
                                        double angularDeflection = 0.5);

    // 7. Converts RawMeshData to GPU-ready MeshPayload for rendering
    static MeshPayload ToMeshPayload(const RawMeshData& mesh);
};

} // namespace androcad

#endif // ANDROCAD_MESH_ENGINE_H