// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "AndroCadKernel.h"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    std::cout << "[AndroCAD Test] Running Kernel Smoke Test (Box Creation & Tessellation)..." << std::endl;

    androcad::AndroCadKernel kernel;
    double dx = 100.0;
    double dy = 50.0;
    double dz = 20.0;

    androcad::MeshPayload box = kernel.CreateBox(dx, dy, dz, 0.1, 0.5);

    if (!box.success) {
        std::cerr << "[FAIL] Kernel failed to create box: " << box.errorMessage << std::endl;
        return 1;
    }

    // 1. Verify analytical volume
    double expectedVolume = dx * dy * dz;
    double volumeError = std::abs(box.volume - expectedVolume);
    std::cout << "  Calculated Volume: " << box.volume << " mm³ (Expected: " << expectedVolume << " mm³)" << std::endl;
    if (volumeError > 1e-3) {
        std::cerr << "[FAIL] Volume mismatch! Error: " << volumeError << std::endl;
        return 1;
    }

    // 2. Verify topological face count (standard rectangular prism has 6 faces)
    std::cout << "  Boundary Face Count: " << box.faceCount << " (Expected: 6)" << std::endl;
    if (box.faceCount != 6) {
        std::cerr << "[FAIL] Face count mismatch! Expected 6, got " << box.faceCount << std::endl;
        return 1;
    }

    // 3. Verify mesh tessellation output
    std::cout << "  Triangles Generated: " << box.triangleCount << std::endl;
    std::cout << "  Vertices Generated: " << box.vertices.size() << std::endl;
    std::cout << "  Indices Generated: " << box.indices.size() << std::endl;

    if (box.triangleCount < 12) {
        std::cerr << "[FAIL] Box tessellation must have at least 12 triangles (2 per face)." << std::endl;
        return 1;
    }

    if (box.indices.size() != static_cast<size_t>(box.triangleCount * 3)) {
        std::cerr << "[FAIL] Index count does not equal triangleCount * 3." << std::endl;
        return 1;
    }

    // 4. Validate vertex normal integrity and face ID tagging
    for (size_t i = 0; i < box.vertices.size(); ++i) {
        const auto& v = box.vertices[i];
        if (v.faceId < 1 || v.faceId > 6) {
            std::cerr << "[FAIL] Vertex " << i << " has invalid FaceID: " << v.faceId << std::endl;
            return 1;
        }

        float normLen = std::sqrt(v.nx * v.nx + v.ny * v.ny + v.nz * v.nz);
        if (normLen < 0.9f || normLen > 1.1f) {
            std::cerr << "[FAIL] Vertex " << i << " has unnormalized normal vector: " << normLen << std::endl;
            return 1;
        }
    }

    std::cout << "[PASS] AndroCAD Kernel Smoke Test passed successfully!" << std::endl;
    return 0;
}
