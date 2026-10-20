// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_KERNEL_H
#define ANDROCAD_KERNEL_H

#include <cstdint>
#include <vector>
#include <string>

namespace androcad {

#pragma pack(push, 1)
struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    uint32_t faceId;
};

struct EdgeVertex {
    float x, y, z;
    uint32_t edgeId;
};
#pragma pack(pop)

struct BoundingBox {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
};

struct Ray {
    float ox, oy, oz;
    float dx, dy, dz;
};

struct PickResult {
    bool hit;
    uint32_t entityId; // faceId (1..N) or edgeId (1..M)
    bool isEdge;       // true if edge was picked, false if face
    float distance;
    float hitPoint[3];
};

struct MeshPayload {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<EdgeVertex> edgeVertices;
    BoundingBox bounds;
    double volume;
    int32_t faceCount;
    int32_t edgeCount;
    int32_t triangleCount;
    bool success;
    std::string errorMessage;
};

class AndroCadKernel {
public:
    AndroCadKernel() = default;
    ~AndroCadKernel() = default;

    // Creates an analytical solid box, measures volume, extracts boundary edges,
    // and tessellates its boundary representation into GPU-ready vertex & index buffers.
    MeshPayload CreateBox(double dx, double dy, double dz, 
                          double linearDeflection = 0.1, 
                          double angularDeflection = 0.5);

    // CPU-side ray-casting picker fallback for O(N) geometric picking when GPU readback is deferred
    static PickResult PickEntityByRay(const MeshPayload& payload, const Ray& ray, float edgeTolerance = 1.5f);
};

} // namespace androcad

#endif // ANDROCAD_KERNEL_H
