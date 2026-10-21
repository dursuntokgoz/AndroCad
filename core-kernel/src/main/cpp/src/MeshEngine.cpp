// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "MeshEngine.h"
#include "FeatureOps.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Standard_Failure.hxx>

#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace androcad {

namespace {

struct VertexKey {
    int64_t x, y, z;
    bool operator==(const VertexKey& o) const {
        return x == o.x && y == o.y && z == o.z;
    }
};

struct VertexKeyHash {
    std::size_t operator()(const VertexKey& k) const {
        std::size_t h1 = std::hash<int64_t>()(k.x);
        std::size_t h2 = std::hash<int64_t>()(k.y);
        std::size_t h3 = std::hash<int64_t>()(k.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct EdgeKey {
    uint32_t v1, v2;
    EdgeKey(uint32_t a, uint32_t b) {
        if (a < b) { v1 = a; v2 = b; }
        else { v1 = b; v2 = a; }
    }
    bool operator==(const EdgeKey& o) const {
        return v1 == o.v1 && v2 == o.v2;
    }
};

struct EdgeKeyHash {
    std::size_t operator()(const EdgeKey& e) const {
        return std::hash<uint64_t>()((static_cast<uint64_t>(e.v1) << 32) | e.v2);
    }
};

} // namespace

MeshStats MeshEngine::AnalyzeMesh(const RawMeshData& mesh) {
    MeshStats stats;
    uint32_t numVerts = static_cast<uint32_t>(mesh.positions.size() / 3);
    uint32_t numTris = static_cast<uint32_t>(mesh.indices.size() / 3);

    stats.vertexCount = numVerts;
    stats.triangleCount = numTris;

    std::unordered_map<EdgeKey, int, EdgeKeyHash> edgeCountMap;
    double totalArea = 0.0;
    double signedVolume = 0.0;

    for (uint32_t t = 0; t < numTris; ++t) {
        uint32_t i1 = mesh.indices[t * 3];
        uint32_t i2 = mesh.indices[t * 3 + 1];
        uint32_t i3 = mesh.indices[t * 3 + 2];

        if (i1 >= numVerts || i2 >= numVerts || i3 >= numVerts) continue;

        edgeCountMap[EdgeKey(i1, i2)]++;
        edgeCountMap[EdgeKey(i2, i3)]++;
        edgeCountMap[EdgeKey(i3, i1)]++;

        // Vertices
        gp_Pnt p1(mesh.positions[i1 * 3], mesh.positions[i1 * 3 + 1], mesh.positions[i1 * 3 + 2]);
        gp_Pnt p2(mesh.positions[i2 * 3], mesh.positions[i2 * 3 + 1], mesh.positions[i2 * 3 + 2]);
        gp_Pnt p3(mesh.positions[i3 * 3], mesh.positions[i3 * 3 + 1], mesh.positions[i3 * 3 + 2]);

        gp_Vec v12(p1, p2);
        gp_Vec v13(p1, p3);
        gp_Vec normal = v12.Crossed(v13);

        totalArea += 0.5 * normal.Magnitude();

        // Signed volume using Divergence theorem
        signedVolume += (p1.X() * (p2.Y() * p3.Z() - p3.Y() * p2.Z()) +
                         p2.X() * (p3.Y() * p1.Z() - p1.Y() * p3.Z()) +
                         p3.X() * (p1.Y() * p2.Z() - p2.Y() * p1.Z())) / 6.0;
    }

    stats.edgeCount = static_cast<uint32_t>(edgeCountMap.size());
    stats.surfaceArea = totalArea;
    stats.enclosedVolume = std::abs(signedVolume);

    // Euler characteristic: V - E + F
    stats.eulerCharacteristic = static_cast<int>(stats.vertexCount) - static_cast<int>(stats.edgeCount) + static_cast<int>(stats.triangleCount);

    // Check manifoldness: every edge must have exactly 2 incident triangles
    bool manifold = (numTris > 0);
    for (const auto& pair : edgeCountMap) {
        if (pair.second != 2) {
            manifold = false;
            break;
        }
    }
    stats.isClosedManifold = manifold;

    return stats;
}

RawMeshData MeshEngine::RepairMesh(const RawMeshData& mesh, double weldTolerance) {
    RawMeshData result;
    if (mesh.positions.empty() || mesh.indices.empty()) {
        return result;
    }

    if (weldTolerance <= 0.0) weldTolerance = 1e-5;
    double invTol = 1.0 / weldTolerance;

    std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexMap;
    std::vector<uint32_t> oldToNewIndex(mesh.positions.size() / 3);

    uint32_t oldVertCount = static_cast<uint32_t>(mesh.positions.size() / 3);
    for (uint32_t i = 0; i < oldVertCount; ++i) {
        float x = mesh.positions[i * 3];
        float y = mesh.positions[i * 3 + 1];
        float z = mesh.positions[i * 3 + 2];

        VertexKey key{
            static_cast<int64_t>(std::round(x * invTol)),
            static_cast<int64_t>(std::round(y * invTol)),
            static_cast<int64_t>(std::round(z * invTol))
        };

        auto it = vertexMap.find(key);
        if (it != vertexMap.end()) {
            oldToNewIndex[i] = it->second;
        } else {
            uint32_t newIdx = static_cast<uint32_t>(result.positions.size() / 3);
            vertexMap[key] = newIdx;
            oldToNewIndex[i] = newIdx;
            result.positions.push_back(x);
            result.positions.push_back(y);
            result.positions.push_back(z);
        }
    }

    uint32_t numTris = static_cast<uint32_t>(mesh.indices.size() / 3);
    int degenerateCount = 0;

    for (uint32_t t = 0; t < numTris; ++t) {
        uint32_t i1 = oldToNewIndex[mesh.indices[t * 3]];
        uint32_t i2 = oldToNewIndex[mesh.indices[t * 3 + 1]];
        uint32_t i3 = oldToNewIndex[mesh.indices[t * 3 + 2]];

        // Discard collapsed edges
        if (i1 == i2 || i2 == i3 || i3 == i1) {
            degenerateCount++;
            continue;
        }

        // Check for zero triangle area
        gp_Pnt p1(result.positions[i1 * 3], result.positions[i1 * 3 + 1], result.positions[i1 * 3 + 2]);
        gp_Pnt p2(result.positions[i2 * 3], result.positions[i2 * 3 + 1], result.positions[i2 * 3 + 2]);
        gp_Pnt p3(result.positions[i3 * 3], result.positions[i3 * 3 + 1], result.positions[i3 * 3 + 2]);

        gp_Vec v12(p1, p2);
        gp_Vec v13(p1, p3);
        if (v12.Crossed(v13).SquareMagnitude() < 1e-12) {
            degenerateCount++;
            continue;
        }

        result.indices.push_back(i1);
        result.indices.push_back(i2);
        result.indices.push_back(i3);
    }

    result.degenerateTriangles = degenerateCount;
    auto stats = AnalyzeMesh(result);
    result.isWatertight = stats.isClosedManifold;

    return result;
}

RawMeshData MeshEngine::SmoothMesh(const RawMeshData& mesh, int iterations, float lambda) {
    if (iterations <= 0) return mesh;
    lambda = std::clamp(lambda, 0.05f, 0.95f);

    RawMeshData current = mesh;
    uint32_t numVerts = static_cast<uint32_t>(current.positions.size() / 3);
    uint32_t numTris = static_cast<uint32_t>(current.indices.size() / 3);

    // Build vertex adjacency map
    std::vector<std::vector<uint32_t>> adj(numVerts);
    for (uint32_t t = 0; t < numTris; ++t) {
        uint32_t i1 = current.indices[t * 3];
        uint32_t i2 = current.indices[t * 3 + 1];
        uint32_t i3 = current.indices[t * 3 + 2];

        adj[i1].push_back(i2); adj[i1].push_back(i3);
        adj[i2].push_back(i1); adj[i2].push_back(i3);
        adj[i3].push_back(i1); adj[i3].push_back(i2);
    }

    // Deduplicate neighbors
    for (uint32_t v = 0; v < numVerts; ++v) {
        std::sort(adj[v].begin(), adj[v].end());
        adj[v].erase(std::unique(adj[v].begin(), adj[v].end()), adj[v].end());
    }

    std::vector<float> nextPositions = current.positions;

    for (int iter = 0; iter < iterations; ++iter) {
        for (uint32_t v = 0; v < numVerts; ++v) {
            const auto& neighbors = adj[v];
            if (neighbors.empty()) continue;

            float avgX = 0.0f, avgY = 0.0f, avgZ = 0.0f;
            for (uint32_t n : neighbors) {
                avgX += current.positions[n * 3];
                avgY += current.positions[n * 3 + 1];
                avgZ += current.positions[n * 3 + 2];
            }
            float count = static_cast<float>(neighbors.size());
            avgX /= count; avgY /= count; avgZ /= count;

            float curX = current.positions[v * 3];
            float curY = current.positions[v * 3 + 1];
            float curZ = current.positions[v * 3 + 2];

            nextPositions[v * 3]     = curX + lambda * (avgX - curX);
            nextPositions[v * 3 + 1] = curY + lambda * (avgY - curY);
            nextPositions[v * 3 + 2] = curZ + lambda * (avgZ - curZ);
        }
        current.positions = nextPositions;
    }

    return current;
}

RawMeshData MeshEngine::DecimateMesh(const RawMeshData& mesh, float targetReductionRatio) {
    if (targetReductionRatio <= 0.0f || targetReductionRatio >= 1.0f) {
        return mesh;
    }

    // Grid clustering / vertex quantization for rapid decimation
    uint32_t numVerts = static_cast<uint32_t>(mesh.positions.size() / 3);
    if (numVerts < 12) return mesh;

    // Determine bounding box
    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;
    float minZ = 1e9f, maxZ = -1e9f;

    for (uint32_t i = 0; i < numVerts; ++i) {
        minX = std::min(minX, mesh.positions[i * 3]);
        maxX = std::max(maxX, mesh.positions[i * 3]);
        minY = std::min(minY, mesh.positions[i * 3 + 1]);
        maxY = std::max(maxY, mesh.positions[i * 3 + 1]);
        minZ = std::min(minZ, mesh.positions[i * 3 + 2]);
        maxZ = std::max(maxZ, mesh.positions[i * 3 + 2]);
    }

    float diag = std::sqrt((maxX - minX)*(maxX - minX) + (maxY - minY)*(maxY - minY) + (maxZ - minZ)*(maxZ - minZ));
    double cellTolerance = (diag * 0.05) * targetReductionRatio;

    return RepairMesh(mesh, cellTolerance);
}

TopoDS_Shape MeshEngine::ConvertMeshToSolid(const RawMeshData& mesh, double tolerance) {
    if (mesh.positions.empty() || mesh.indices.empty()) {
        throw std::invalid_argument("Cannot convert empty mesh to solid");
    }

    uint32_t numTris = static_cast<uint32_t>(mesh.indices.size() / 3);
    uint32_t numVerts = static_cast<uint32_t>(mesh.positions.size() / 3);

    BRepBuilderAPI_Sewing sewing(tolerance);

    for (uint32_t t = 0; t < numTris; ++t) {
        uint32_t i1 = mesh.indices[t * 3];
        uint32_t i2 = mesh.indices[t * 3 + 1];
        uint32_t i3 = mesh.indices[t * 3 + 2];

        if (i1 >= numVerts || i2 >= numVerts || i3 >= numVerts) continue;

        gp_Pnt p1(mesh.positions[i1 * 3], mesh.positions[i1 * 3 + 1], mesh.positions[i1 * 3 + 2]);
        gp_Pnt p2(mesh.positions[i2 * 3], mesh.positions[i2 * 3 + 1], mesh.positions[i2 * 3 + 2]);
        gp_Pnt p3(mesh.positions[i3 * 3], mesh.positions[i3 * 3 + 1], mesh.positions[i3 * 3 + 2]);

        if (p1.Distance(p2) < 1e-6 || p2.Distance(p3) < 1e-6 || p3.Distance(p1) < 1e-6) {
            continue;
        }

        try {
            BRepBuilderAPI_MakeEdge e1(p1, p2);
            BRepBuilderAPI_MakeEdge e2(p2, p3);
            BRepBuilderAPI_MakeEdge e3(p3, p1);

            if (!e1.IsDone() || !e2.IsDone() || !e3.IsDone()) continue;

            BRepBuilderAPI_MakeWire wireMaker;
            wireMaker.Add(e1.Edge());
            wireMaker.Add(e2.Edge());
            wireMaker.Add(e3.Edge());

            if (!wireMaker.IsDone()) continue;

            BRepBuilderAPI_MakeFace faceMaker(wireMaker.Wire(), Standard_True);
            if (faceMaker.IsDone()) {
                sewing.Add(faceMaker.Face());
            }
        } catch (...) {
            continue;
        }
    }

    sewing.Perform();
    TopoDS_Shape sewedShape = sewing.SewedShape();

    // Try converting sewed shell into a closed solid
    if (sewedShape.ShapeType() == TopAbs_SHELL) {
        try {
            BRepBuilderAPI_MakeSolid solidMaker(TopoDS::Shell(sewedShape));
            if (solidMaker.IsDone()) {
                return solidMaker.Solid();
            }
        } catch (...) {}
    }

    return sewedShape;
}

RawMeshData MeshEngine::ExtractFromShape(const TopoDS_Shape& shape,
                                        double linearDeflection,
                                        double angularDeflection) {
    RawMeshData result;
    if (shape.IsNull()) return result;

    BRepMesh_IncrementalMesh meshGen(shape, linearDeflection, Standard_False, angularDeflection);

    std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertMap;
    double invTol = 1e4;

    for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next()) {
        const TopoDS_Face& face = TopoDS::Face(exp.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull()) continue;

        gp_Trsf trsf = loc.Transformation();
        Standard_Integer nbNodes = tri->NbNodes();
        Standard_Integer nbTris = tri->NbTriangles();

        std::vector<uint32_t> localToGlobal(nbNodes + 1);

        for (Standard_Integer i = 1; i <= nbNodes; ++i) {
            gp_Pnt p = tri->Node(i).Transformed(trsf);

            VertexKey key{
                static_cast<int64_t>(std::round(p.X() * invTol)),
                static_cast<int64_t>(std::round(p.Y() * invTol)),
                static_cast<int64_t>(std::round(p.Z() * invTol))
            };

            auto it = vertMap.find(key);
            if (it != vertMap.end()) {
                localToGlobal[i] = it->second;
            } else {
                uint32_t newIdx = static_cast<uint32_t>(result.positions.size() / 3);
                vertMap[key] = newIdx;
                localToGlobal[i] = newIdx;
                result.positions.push_back(static_cast<float>(p.X()));
                result.positions.push_back(static_cast<float>(p.Y()));
                result.positions.push_back(static_cast<float>(p.Z()));
            }
        }

        for (Standard_Integer i = 1; i <= nbTris; ++i) {
            Standard_Integer n1, n2, n3;
            tri->Triangle(i).Get(n1, n2, n3);

            if (face.Orientation() == TopAbs_REVERSED) {
                std::swap(n2, n3);
            }

            result.indices.push_back(localToGlobal[n1]);
            result.indices.push_back(localToGlobal[n2]);
            result.indices.push_back(localToGlobal[n3]);
        }
    }

    auto stats = AnalyzeMesh(result);
    result.isWatertight = stats.isClosedManifold;
    return result;
}

MeshPayload MeshEngine::ToMeshPayload(const RawMeshData& mesh) {
    MeshPayload payload;
    uint32_t numVerts = static_cast<uint32_t>(mesh.positions.size() / 3);
    uint32_t numTris = static_cast<uint32_t>(mesh.indices.size() / 3);

    if (numVerts == 0 || numTris == 0) {
        payload.success = false;
        payload.errorMessage = "Empty mesh data";
        return payload;
    }

    // Compute vertex normals
    std::vector<gp_Vec> normals(numVerts, gp_Vec(0, 0, 0));
    for (uint32_t t = 0; t < numTris; ++t) {
        uint32_t i1 = mesh.indices[t * 3];
        uint32_t i2 = mesh.indices[t * 3 + 1];
        uint32_t i3 = mesh.indices[t * 3 + 2];

        if (i1 >= numVerts || i2 >= numVerts || i3 >= numVerts) continue;

        gp_Pnt p1(mesh.positions[i1 * 3], mesh.positions[i1 * 3 + 1], mesh.positions[i1 * 3 + 2]);
        gp_Pnt p2(mesh.positions[i2 * 3], mesh.positions[i2 * 3 + 1], mesh.positions[i2 * 3 + 2]);
        gp_Pnt p3(mesh.positions[i3 * 3], mesh.positions[i3 * 3 + 1], mesh.positions[i3 * 3 + 2]);

        gp_Vec v12(p1, p2);
        gp_Vec v13(p1, p3);
        gp_Vec norm = v12.Crossed(v13);

        normals[i1] += norm;
        normals[i2] += norm;
        normals[i3] += norm;
    }

    payload.bounds.minX = 1e9f; payload.bounds.maxX = -1e9f;
    payload.bounds.minY = 1e9f; payload.bounds.maxY = -1e9f;
    payload.bounds.minZ = 1e9f; payload.bounds.maxZ = -1e9f;

    payload.vertices.resize(numVerts);
    for (uint32_t i = 0; i < numVerts; ++i) {
        float x = mesh.positions[i * 3];
        float y = mesh.positions[i * 3 + 1];
        float z = mesh.positions[i * 3 + 2];

        payload.vertices[i].x = x;
        payload.vertices[i].y = y;
        payload.vertices[i].z = z;
        payload.vertices[i].faceId = 1;

        if (normals[i].SquareMagnitude() > 1e-8) {
            normals[i].Normalize();
            payload.vertices[i].nx = static_cast<float>(normals[i].X());
            payload.vertices[i].ny = static_cast<float>(normals[i].Y());
            payload.vertices[i].nz = static_cast<float>(normals[i].Z());
        } else {
            payload.vertices[i].nx = 0.0f;
            payload.vertices[i].ny = 1.0f;
            payload.vertices[i].nz = 0.0f;
        }

        payload.bounds.minX = std::min(payload.bounds.minX, x);
        payload.bounds.maxX = std::max(payload.bounds.maxX, x);
        payload.bounds.minY = std::min(payload.bounds.minY, y);
        payload.bounds.maxY = std::max(payload.bounds.maxY, y);
        payload.bounds.minZ = std::min(payload.bounds.minZ, z);
        payload.bounds.maxZ = std::max(payload.bounds.maxZ, z);
    }

    payload.indices = mesh.indices;
    payload.triangleCount = static_cast<int32_t>(numTris);
    payload.faceCount = 1;
    payload.edgeCount = static_cast<int32_t>(numTris * 3 / 2);
    payload.success = true;

    auto stats = AnalyzeMesh(mesh);
    payload.volume = stats.enclosedVolume;

    return payload;
}

} // namespace androcad