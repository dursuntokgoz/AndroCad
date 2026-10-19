// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "AndroCadKernel.h"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    std::cout << "[AndroCAD Test] Running Kernel Smoke Test (Box Creation, Edges & Ray Picking)..." << std::endl;

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

    // 5. Verify topological boundary edge extraction (Milestone 2)
    std::cout << "  Boundary Edge Count: " << box.edgeCount << " (Expected: 12)" << std::endl;
    if (box.edgeCount != 12) {
        std::cerr << "[FAIL] Edge count mismatch! Expected 12, got " << box.edgeCount << std::endl;
        return 1;
    }

    std::cout << "  Discretized Edge Vertices: " << box.edgeVertices.size() << " (Expected >= 24)" << std::endl;
    if (box.edgeVertices.size() < 24) {
        std::cerr << "[FAIL] Edge vertex count is too low! Expected >= 24, got " << box.edgeVertices.size() << std::endl;
        return 1;
    }

    for (const auto& ev : box.edgeVertices) {
        if (ev.edgeId < 1 || ev.edgeId > 12) {
            std::cerr << "[FAIL] Edge vertex has invalid edgeId: " << ev.edgeId << std::endl;
            return 1;
        }
    }

    // 6. Verify Bounding Box
    std::cout << "  Bounding Box: [" 
              << box.bounds.minX << ", " << box.bounds.minY << ", " << box.bounds.minZ << "] to ["
              << box.bounds.maxX << ", " << box.bounds.maxY << ", " << box.bounds.maxZ << "]" << std::endl;
    if (std::abs(box.bounds.maxX - 100.0f) > 0.1f ||
        std::abs(box.bounds.maxY - 50.0f) > 0.1f ||
        std::abs(box.bounds.maxZ - 20.0f) > 0.1f) {
        std::cerr << "[FAIL] Bounding box dimensions incorrect!" << std::endl;
        return 1;
    }

    // 7. Verify Ray-Casting Entity Picking (Milestone 2)
    // Ray 1: Downward at top face (x=50, y=25, z=100) -> should hit face at z=20 (dist=80)
    androcad::Ray topRay{50.0f, 25.0f, 100.0f, 0.0f, 0.0f, -1.0f};
    androcad::PickResult pickFace = androcad::AndroCadKernel::PickEntityByRay(box, topRay, 1.0f);
    if (!pickFace.hit) {
        std::cerr << "[FAIL] Ray aimed at top face center failed to hit!" << std::endl;
        return 1;
    }
    std::cout << "  Ray Pick Face Result: hit=" << pickFace.hit 
              << " entityId=" << pickFace.entityId 
              << " isEdge=" << pickFace.isEdge 
              << " dist=" << pickFace.distance << std::endl;
    if (pickFace.isEdge) {
        std::cerr << "[FAIL] Ray aimed at face interior incorrectly picked an edge!" << std::endl;
        return 1;
    }
    if (std::abs(pickFace.distance - 80.0f) > 0.1f) {
        std::cerr << "[FAIL] Ray pick distance incorrect! Expected 80.0, got " << pickFace.distance << std::endl;
        return 1;
    }

    // Ray 2: Ray directly grazing edge at x=100, y=25, z=100 -> should hit edge
    androcad::Ray edgeRay{100.0f, 25.0f, 100.0f, 0.0f, 0.0f, -1.0f};
    androcad::PickResult pickEdge = androcad::AndroCadKernel::PickEntityByRay(box, edgeRay, 1.5f);
    if (!pickEdge.hit || !pickEdge.isEdge) {
        std::cerr << "[FAIL] Ray aimed directly at edge failed to pick edge! (hit=" << pickEdge.hit << ", isEdge=" << pickEdge.isEdge << ")" << std::endl;
        return 1;
    }
    std::cout << "  Ray Pick Edge Result: hit=" << pickEdge.hit 
              << " edgeId=" << pickEdge.entityId 
              << " dist=" << pickEdge.distance << std::endl;

    // Ray 2b: Ray slightly outside model boundary at x=100.5, y=25, z=100 -> should pick edge via tolerance
    androcad::Ray nearEdgeRay{100.5f, 25.0f, 100.0f, 0.0f, 0.0f, -1.0f};
    androcad::PickResult pickNearEdge = androcad::AndroCadKernel::PickEntityByRay(box, nearEdgeRay, 1.0f);
    if (!pickNearEdge.hit || !pickNearEdge.isEdge) {
        std::cerr << "[FAIL] Near-edge ray failed to pick edge within tolerance!" << std::endl;
        return 1;
    }
    std::cout << "  Near-Edge Pick Result: hit=" << pickNearEdge.hit 
              << " edgeId=" << pickNearEdge.entityId << std::endl;

    // Ray 3: Missing ray -> should not hit
    androcad::Ray missRay{250.0f, 250.0f, 100.0f, 0.0f, 0.0f, -1.0f};
    androcad::PickResult pickMiss = androcad::AndroCadKernel::PickEntityByRay(box, missRay, 1.5f);
    if (pickMiss.hit) {
        std::cerr << "[FAIL] Missing ray falsely reported a hit!" << std::endl;
        return 1;
    }

    std::cout << "[PASS] AndroCAD Milestone 2 Kernel & Picking Tests passed successfully!" << std::endl;
    return 0;
}
