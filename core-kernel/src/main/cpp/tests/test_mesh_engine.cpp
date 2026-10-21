// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "MeshEngine.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace androcad;

void TestMeshTopologyAndEuler() {
    std::cout << "[Test 1/5] Testing Mesh Topology & Euler Characteristic (V - E + F = 2)..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 50.0, 20.0).Shape();

    RawMeshData mesh = MeshEngine::ExtractFromShape(box, 0.1, 0.5);
    assert(!mesh.positions.empty());
    assert(!mesh.indices.empty());

    MeshStats stats = MeshEngine::AnalyzeMesh(mesh);
    assert(stats.vertexCount > 0);
    assert(stats.triangleCount >= 12);
    assert(stats.edgeCount > 0);
    assert(stats.isClosedManifold);

    // Euler characteristic for sphere-equivalent closed manifold solid is 2
    assert(stats.eulerCharacteristic == 2);

    // Volume of 100 x 50 x 20 is 100,000 mm³
    assert(std::abs(stats.enclosedVolume - 100000.0) < 100.0);

    std::cout << "  ✓ Vertices: " << stats.vertexCount << ", Triangles: " << stats.triangleCount
              << ", Edges: " << stats.edgeCount << ", Euler: " << stats.eulerCharacteristic
              << ", Enclosed Volume: " << stats.enclosedVolume << " mm³" << std::endl;
}

void TestMeshRepairAndWeld() {
    std::cout << "[Test 2/5] Testing Vertex Welding & Degenerate Triangle Removal..." << std::endl;
    RawMeshData badMesh;
    // 4 points with 2 duplicates:
    // P0: (0,0,0), P1: (10,0,0), P2: (0,10,0) -> valid triangle
    // P3: (0,0,0) (duplicate of P0)
    badMesh.positions = {
        0.0f, 0.0f, 0.0f,  // 0
        10.0f, 0.0f, 0.0f, // 1
        0.0f, 10.0f, 0.0f, // 2
        0.0f, 0.0f, 0.0f,  // 3 (duplicate)
        10.0f, 0.0f, 0.0f  // 4 (duplicate)
    };
    // Triangle 1: (0, 1, 2) valid
    // Triangle 2: (0, 3, 2) degenerate after weld because 0 == 3
    // Triangle 3: (1, 1, 2) collapsed edge
    badMesh.indices = {
        0, 1, 2,
        0, 3, 2,
        1, 1, 2
    };

    RawMeshData repaired = MeshEngine::RepairMesh(badMesh, 1e-4);
    assert(repaired.positions.size() / 3 == 3); // 3 unique vertices
    assert(repaired.indices.size() / 3 == 1);   // 1 valid triangle remained
    assert(repaired.degenerateTriangles >= 2);

    std::cout << "  ✓ Welded 5 vertices to 3 unique, removed " << repaired.degenerateTriangles
              << " degenerate triangles" << std::endl;
}

void TestLaplacianSmooth() {
    std::cout << "[Test 3/5] Testing Laplacian Mesh Smoothing Filter..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(40.0, 40.0, 40.0).Shape();
    RawMeshData mesh = MeshEngine::ExtractFromShape(box, 0.1, 0.5);

    uint32_t origVerts = static_cast<uint32_t>(mesh.positions.size() / 3);
    uint32_t origTris = static_cast<uint32_t>(mesh.indices.size() / 3);

    RawMeshData smoothed = MeshEngine::SmoothMesh(mesh, 3, 0.3f);
    assert(smoothed.positions.size() / 3 == origVerts);
    assert(smoothed.indices.size() / 3 == origTris);

    MeshStats stats = MeshEngine::AnalyzeMesh(smoothed);
    assert(stats.enclosedVolume > 0.0);

    std::cout << "  ✓ Smooth filter preserved topology (" << origVerts << " verts, "
              << origTris << " tris), volume: " << stats.enclosedVolume << " mm³" << std::endl;
}

void TestMeshDecimate() {
    std::cout << "[Test 4/5] Testing Mesh Decimation & Simplification..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(50.0, 50.0, 50.0).Shape();
    // Fine mesh
    RawMeshData fineMesh = MeshEngine::ExtractFromShape(box, 0.02, 0.1);

    RawMeshData decimated = MeshEngine::DecimateMesh(fineMesh, 0.5f);
    assert(!decimated.positions.empty());
    assert(!decimated.indices.empty());

    std::cout << "  ✓ Fine mesh: " << (fineMesh.indices.size() / 3) << " tris -> Decimated: "
              << (decimated.indices.size() / 3) << " tris" << std::endl;
}

void TestMeshToSolidConversion() {
    std::cout << "[Test 5/5] Testing Mesh-to-BREP Solid Conversion & Volume Verification..." << std::endl;
    TopoDS_Shape origBox = BRepPrimAPI_MakeBox(60.0, 40.0, 30.0).Shape();
    RawMeshData mesh = MeshEngine::ExtractFromShape(origBox, 0.1, 0.5);

    TopoDS_Shape convertedSolid = MeshEngine::ConvertMeshToSolid(mesh, 1e-2);
    assert(!convertedSolid.IsNull());

    GProp_GProps volProps;
    BRepGProp::VolumeProperties(convertedSolid, volProps);
    double convertedVol = volProps.Mass();

    // Original volume = 60 * 40 * 30 = 72,000 mm³
    assert(std::abs(convertedVol - 72000.0) < 500.0);

    std::cout << "  ✓ Converted to valid B-Rep solid, Volume: " << convertedVol
              << " mm³ (expected 72,000 mm³)" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running AndroCAD Mesh Engine Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    TestMeshTopologyAndEuler();
    TestMeshRepairAndWeld();
    TestLaplacianSmooth();
    TestMeshDecimate();
    TestMeshToSolidConversion();

    std::cout << "========================================" << std::endl;
    std::cout << "All 5 Mesh Engine tests PASSED!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
