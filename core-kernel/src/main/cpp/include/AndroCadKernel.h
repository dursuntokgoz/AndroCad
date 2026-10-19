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
#pragma pack(pop)

struct MeshPayload {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    double volume;
    int32_t faceCount;
    int32_t triangleCount;
    bool success;
    std::string errorMessage;
};

class AndroCadKernel {
public:
    AndroCadKernel() = default;
    ~AndroCadKernel() = default;

    // Milestone 1 Smoke Test: Creates an analytical solid box, computes its volume,
    // and tessellates its boundary representation into GPU-ready vertex & index buffers.
    MeshPayload CreateBox(double dx, double dy, double dz, 
                          double linearDeflection = 0.1, 
                          double angularDeflection = 0.5);
};

} // namespace androcad

#endif // ANDROCAD_KERNEL_H
