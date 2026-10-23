// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "SurfaceOps.h"
#include "FeatureOps.h"
#include "AndroCadKernel.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <gp_Pln.hxx>

#include <iostream>
#include <cassert>
#include <cmath>

using namespace androcad;

void TestLoftSolid() {
    std::cout << "[Test 1/5] Testing Multi-Section Loft Solid..." << std::endl;

    // Bottom section: 100 x 50 rectangle at Z = 0
    TopoDS_Wire wireBottom = SurfaceOps::CreateRectangleWire(0.0, 0.0, 0.0, 100.0, 50.0);

    // Top section: circle radius 20 at center (50, 25), Z = 40
    TopoDS_Wire wireTop = SurfaceOps::CreateCircleWire(50.0, 25.0, 40.0, 20.0);

    std::vector<TopoDS_Wire> sections = {wireBottom, wireTop};
    TopoDS_Shape loftSolid;
    MeshPayload payload = SurfaceOps::MakeLoft(sections, true, false, loftSolid);

    assert(payload.success && "Loft should succeed");
    assert(payload.volume > 0.0 && "Loft solid volume must be positive");
    assert(payload.faceCount >= 2 && "Loft solid must have bounding faces");

    BRepCheck_Analyzer analyzer(loftSolid);
    assert(analyzer.IsValid() && "Loft solid must pass BRep topology validation");

    std::cout << "  ✓ Loft volume: " << payload.volume << " mm³, Faces: " << payload.faceCount
              << ", Triangles: " << payload.triangleCount << std::endl;
}

void TestSweepPipe() {
    std::cout << "[Test 2/5] Testing Sweep along Spine Path..." << std::endl;

    // Spine: vertical straight trajectory from (0,0,0) to (0,0,50)
    TopoDS_Wire spine = SurfaceOps::CreateRectangleWire(0.0, 0.0, 0.0, 10.0, 10.0);
    // Profile: circle radius 5 at Z = 0
    TopoDS_Wire profile = SurfaceOps::CreateCircleWire(0.0, 0.0, 0.0, 5.0, 1.0, 0.0, 0.0);

    TopoDS_Shape sweepShape;
    MeshPayload payload = SurfaceOps::MakeSweep(spine, profile, true, sweepShape);

    assert(payload.success && "Sweep should succeed");
    assert(payload.faceCount > 0 && "Sweep must have generated faces");
    std::cout << "  ✓ Sweep generated " << payload.faceCount << " faces and "
              << payload.triangleCount << " triangles." << std::endl;
}

void TestThickenSurface() {
    std::cout << "[Test 3/5] Testing Surface Thicken to Solid..." << std::endl;

    // Create a 100x50 planar face at Z = 0
    TopoDS_Wire wire = SurfaceOps::CreateRectangleWire(0.0, 0.0, 0.0, 100.0, 50.0);
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire).Face();

    TopoDS_Shape thickenedSolid;
    MeshPayload payload = SurfaceOps::MakeThicken(face, 5.0, thickenedSolid);

    assert(payload.success && "Thicken should succeed");
    // Analytical expected volume: 100 * 50 * 5 = 25,000 mm³
    assert(std::abs(payload.volume - 25000.0) < 50.0 && "Thickened volume must match expected 25,000 mm³");
    std::cout << "  ✓ Thickened solid volume: " << payload.volume << " mm³ (Target: 25,000 mm³)" << std::endl;
}

void TestSewingToSolid() {
    std::cout << "[Test 4/5] Testing Surface Sheet Sewing to Watertight Solid..." << std::endl;

    // Construct 6 faces of a 10 x 10 x 10 cube independently
    TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
    std::vector<TopoDS_Shape> faceSheets;
    for (TopExp_Explorer exp(box, TopAbs_FACE); exp.More(); exp.Next()) {
        faceSheets.push_back(exp.Current());
    }
    assert(faceSheets.size() == 6);

    TopoDS_Shape sewedShape;
    MeshPayload payload = SurfaceOps::SewSurfaces(faceSheets, 1.0e-3, true, sewedShape);

    assert(payload.success && "Sewing should succeed");
    // Analytical expected cube volume: 10^3 = 1,000 mm³
    assert(std::abs(payload.volume - 1000.0) < 5.0 && "Sewed solid volume must match 1,000 mm³");
    std::cout << "  ✓ Sewed watertight solid volume: " << payload.volume << " mm³" << std::endl;
}

void TestPressPullFace() {
    std::cout << "[Test 5/5] Testing Direct Modeling Press-Pull Face..." << std::endl;

    // Initial box: 100 x 50 x 20 (Volume: 100,000 mm³)
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 50.0, 20.0).Shape();

    // Find top face (Z = 20)
    int targetFaceId = 1;
    int currentId = 1;
    for (TopExp_Explorer exp(box, TopAbs_FACE); exp.More(); exp.Next(), ++currentId) {
        TopoDS_Face f = TopoDS::Face(exp.Current());
        BRepAdaptor_Surface adaptor(f);
        if (adaptor.GetType() == GeomAbs_Plane) {
            gp_Pln pln = adaptor.Plane();
            if (std::abs(pln.Location().Z() - 20.0) < 1e-4) {
                targetFaceId = currentId;
                break;
            }
        }
    }

    // Offset top face outwards by +10.0 mm -> New height = 30.0 mm (Volume = 150,000 mm³)
    TopoDS_Shape offsetSolid;
    MeshPayload payload = SurfaceOps::PressPullFace(box, targetFaceId, 10.0, offsetSolid);

    assert(payload.success && "Press-Pull outward should succeed");
    assert(std::abs(payload.volume - 150000.0) < 100.0 && "Press-Pull outward volume must match 150,000 mm³");
    std::cout << "  ✓ Press-Pull expanded volume: " << payload.volume << " mm³ (Target: 150,000 mm³)" << std::endl;

    // Offset top face inwards by -5.0 mm -> New height = 15.0 mm (Volume = 75,000 mm³)
    TopoDS_Shape cutSolid;
    MeshPayload cutPayload = SurfaceOps::PressPullFace(box, targetFaceId, -5.0, cutSolid);

    assert(cutPayload.success && "Press-Pull inward cut should succeed");
    assert(std::abs(cutPayload.volume - 75000.0) < 100.0 && "Press-Pull inward cut volume must match 75,000 mm³");
    std::cout << "  ✓ Press-Pull inward cut volume: " << cutPayload.volume << " mm³ (Target: 75,000 mm³)" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running AndroCAD Surface Operations Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        TestLoftSolid();
        TestSweepPipe();
        TestThickenSurface();
        TestSewingToSolid();
        TestPressPullFace();

        std::cout << "\n>>> ALL 5 SURFACE & DIRECT MODELING TESTS PASSED! <<<" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAILED: Exception caught: " << e.what() << std::endl;
        return 1;
    }
}
