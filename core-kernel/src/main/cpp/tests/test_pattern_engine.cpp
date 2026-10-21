// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "PatternEngine.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <cassert>
#include <iostream>
#include <cmath>

using namespace androcad;

void TestRectangularPattern() {
    std::cout << "[Test 1] Testing 2D Rectangular Pattern Grid..." << std::endl;

    TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
    assert(!box.IsNull());

    // 3 x 4 grid (12 instances), spacing 25mm in X, 20mm in Y
    PatternResult res = PatternEngine::RectangularPattern(
        box, 3, 25.0, gp_Vec(1, 0, 0), 4, 20.0, gp_Vec(0, 1, 0), false
    );

    assert(res.success);
    assert(res.instanceCount == 12);

    // Each box is 1000 mm³, total non-overlapping volume is 12000 mm³
    assert(std::abs(res.totalVolumeMm3 - 12000.0) < 1.0);

    // Verify bounding box extent: X in [0, 2*25 + 10] = [0, 60], Y in [0, 3*20 + 10] = [0, 70]
    Bnd_Box bbox;
    BRepBndLib::Add(res.shape, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    assert(std::abs(xmin - 0.0) < 1e-3);
    assert(std::abs(xmax - 60.0) < 1e-3);
    assert(std::abs(ymin - 0.0) < 1e-3);
    assert(std::abs(ymax - 70.0) < 1e-3);

    std::cout << "  Passed: 12 instances, total volume=" << res.totalVolumeMm3 << " mm³" << std::endl;
}

void TestCircularPattern() {
    std::cout << "[Test 2] Testing Polar Circular Pattern..." << std::endl;

    // Pin cylinder off-center: center at (30, 0, 0), radius 5, height 15
    gp_Ax2 pinAx(gp_Pnt(30.0, 0.0, 0.0), gp_Dir(0, 0, 1));
    TopoDS_Shape pin = BRepPrimAPI_MakeCylinder(pinAx, 5.0, 15.0).Shape();

    // 6 pins evenly distributed 360 deg around Z axis
    gp_Ax1 rotAxis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    PatternResult res = PatternEngine::CircularPattern(pin, rotAxis, 6, 360.0, false);

    assert(res.success);
    assert(res.instanceCount == 6);

    double singleVol = 3.141592653589793 * 25.0 * 15.0; // ~1178.1 mm³
    assert(std::abs(res.totalVolumeMm3 - (6 * singleVol)) < 5.0);

    // Circular symmetry check: X and Y bounds should be approximately [-35, +35]
    Bnd_Box bbox;
    BRepBndLib::Add(res.shape, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    assert(std::abs(xmax - 35.0) < 0.5);
    assert(std::abs(xmin - (-35.0)) < 0.5);
    assert(std::abs(ymax - 35.0) < 0.5);
    assert(std::abs(ymin - (-35.0)) < 0.5);

    std::cout << "  Passed: 6 circular instances, radial radius 30mm confirmed" << std::endl;
}

void TestMirrorSymmetry() {
    std::cout << "[Test 3] Testing Mirror symmetry across datum plane..." << std::endl;

    // Solid located at X in [10, 30]
    gp_Ax2 boxAx(gp_Pnt(10.0, 0.0, 0.0), gp_Dir(0, 0, 1));
    TopoDS_Shape box = BRepPrimAPI_MakeBox(boxAx, 20.0, 15.0, 10.0).Shape();

    // Mirror across X=0 plane (normal = X)
    gp_Ax2 mirrorPln(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
    PatternResult res = PatternEngine::Mirror(box, mirrorPln, true, false);

    assert(res.success);
    assert(res.instanceCount == 2);

    // Volume of 2 boxes: 2 * (20 * 15 * 10) = 6000 mm³
    assert(std::abs(res.totalVolumeMm3 - 6000.0) < 1.0);

    // Bounding box should be symmetric in X: [-30, +30]
    Bnd_Box bbox;
    BRepBndLib::Add(res.shape, bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    assert(std::abs(xmin - (-30.0)) < 1e-3);
    assert(std::abs(xmax - 30.0) < 1e-3);

    std::cout << "  Passed: Mirrored pair symmetric across X=0 ([-30, +30] mm)" << std::endl;
}

void TestDraftAngleTaper() {
    std::cout << "[Test 4] Testing Draft Angle taper on faces..." << std::endl;

    TopoDS_Shape box = BRepPrimAPI_MakeBox(20.0, 20.0, 30.0).Shape();
    assert(!box.IsNull());

    // Pull direction +Z, draft angle 3.0 degrees on first vertical face
    PatternResult res = PatternEngine::ApplyDraft(box, gp_Dir(0, 0, 1), 3.0, {1});
    assert(res.success);
    assert(!res.shape.IsNull());
    assert(res.totalVolumeMm3 > 0.0);

    std::cout << "  Passed: Draft angle applied, volume=" << res.totalVolumeMm3 << " mm³" << std::endl;
}

void TestPatternFusedSolid() {
    std::cout << "[Test 5] Testing Pattern with Boolean Union Fusion..." << std::endl;

    TopoDS_Shape box = BRepPrimAPI_MakeBox(20.0, 20.0, 20.0).Shape();

    // 3 blocks shifted by 10mm (overlapping by 10mm), fused into single solid
    PatternResult res = PatternEngine::RectangularPattern(
        box, 3, 10.0, gp_Vec(1, 0, 0), 1, 0.0, gp_Vec(0, 1, 0), true
    );

    assert(res.success);
    assert(!res.shape.IsNull());

    // Single contiguous solid has total length 20 + 10 + 10 = 40mm
    // Volume = 40 * 20 * 20 = 16,000 mm³
    assert(std::abs(res.totalVolumeMm3 - 16000.0) < 5.0);

    // Verify fused shape contains faces
    int faceCount = 0;
    for (TopExp_Explorer exp(res.shape, TopAbs_FACE); exp.More(); exp.Next()) {
        faceCount++;
    }
    assert(faceCount >= 6);

    std::cout << "  Passed: Fused manifold solid volume=" << res.totalVolumeMm3 << " mm³, faces=" << faceCount << std::endl;
}

int main() {
    std::cout << "=== AndroCAD Pattern Engine Test Suite ===" << std::endl;

    try {
        TestRectangularPattern();
        TestCircularPattern();
        TestMirrorSymmetry();
        TestDraftAngleTaper();
        TestPatternFusedSolid();

        std::cout << "\nAll 5 Pattern Engine tests PASSED successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Pattern Test FAILED with exception: " << e.what() << std::endl;
        return 1;
    }
}
