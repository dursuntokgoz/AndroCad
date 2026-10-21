// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "FastenerEngine.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <cassert>
#include <iostream>
#include <cmath>

using namespace androcad;

void TestCounterboreHole() {
    std::cout << "[Test 1] Testing ISO Counterbore Hole Wizard..." << std::endl;

    TopoDS_Shape block = BRepPrimAPI_MakeBox(50.0, 50.0, 20.0).Shape();
    assert(!block.IsNull());
    double initialVol = 50.0 * 50.0 * 20.0; // 50,000 mm³

    // M6 counterbore hole at center (25, 25, 20) pointing in -Z direction
    gp_Pnt pos(25.0, 25.0, 20.0);
    gp_Dir dir(0, 0, -1);
    HoleResult res = FastenerEngine::CreateStandardHole(
        block, pos, dir, "M6", HoleType::COUNTERBORE, 20.0
    );

    assert(res.success);
    assert(!res.shape.IsNull());
    assert(res.volumeMm3 < initialVol);

    // M6 CB: clearance dia 6.6mm (r=3.3), CB dia 11.0mm (r=5.5), CB depth 6.0mm
    // Expected volume cut: PI * 5.5^2 * 6.0 + PI * 3.3^2 * 14.0 = ~570 + ~478 = ~1048 mm³
    double cutVol = initialVol - res.volumeMm3;
    assert(cutVol > 800.0 && cutVol < 1300.0);

    std::cout << "  Passed: M6 Counterbore hole cut successfully, cut volume=" << cutVol << " mm³" << std::endl;
}

void TestCountersinkHole() {
    std::cout << "[Test 2] Testing ISO Countersink Hole..." << std::endl;

    TopoDS_Shape block = BRepPrimAPI_MakeBox(40.0, 40.0, 20.0).Shape();
    gp_Pnt pos(20.0, 20.0, 20.0);
    gp_Dir dir(0, 0, -1);

    HoleResult res = FastenerEngine::CreateStandardHole(
        block, pos, dir, "M5", HoleType::COUNTERSINK, 20.0
    );

    assert(res.success);
    assert(!res.shape.IsNull());
    double cutVol = 32000.0 - res.volumeMm3;
    assert(cutVol > 400.0 && cutVol < 1000.0);

    std::cout << "  Passed: M5 Countersink hole cut successfully, cut volume=" << cutVol << " mm³" << std::endl;
}

void TestThreadedBolt() {
    std::cout << "[Test 3] Testing Modeled 3D Metric Helical Threaded Bolt..." << std::endl;

    // M8 bolt: shaft length 30mm, thread length 20mm, pitch 1.25mm
    ThreadResult res = FastenerEngine::CreateThreadedBolt("M8", 30.0, 20.0, true);

    assert(res.success);
    assert(!res.shape.IsNull());
    assert(res.toothCount == 16); // 20 / 1.25 = 16 teeth
    assert(std::abs(res.pitchMm - 1.25) < 1e-4);
    assert(res.outerDiameterMm > 8.0);
    assert(res.volumeMm3 > 1500.0);

    std::cout << "  Passed: M8 Threaded bolt generated, toothCount=" << res.toothCount 
              << ", outerDia=" << res.outerDiameterMm << " mm, vol=" << res.volumeMm3 << " mm³" << std::endl;
}

void TestEdgeProjection() {
    std::cout << "[Test 4] Testing Associative 3D Edge Projection onto Sketch Plane..." << std::endl;

    TopoDS_Shape box = BRepPrimAPI_MakeBox(30.0, 20.0, 10.0).Shape();
    gp_Pln xyPlane(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));

    ProjectionResult res = FastenerEngine::ProjectEdgesToPlane(box, xyPlane);

    assert(res.success);
    assert(res.edgeCount == 12); // Box has 12 boundary edges
    assert(res.segments.size() == 12);

    // Verify projected segment coordinates lie within [0, 30] x [0, 20]
    for (const auto& seg : res.segments) {
        assert(seg.startU >= -1e-4 && seg.startU <= 30.0 + 1e-4);
        assert(seg.startV >= -1e-4 && seg.startV <= 20.0 + 1e-4);
        assert(seg.endU >= -1e-4 && seg.endU <= 30.0 + 1e-4);
        assert(seg.endV >= -1e-4 && seg.endV <= 20.0 + 1e-4);
    }

    std::cout << "  Passed: 12 3D edges projected onto 2D plane coordinate space" << std::endl;
}

void TestCubicBSpline() {
    std::cout << "[Test 5] Testing Cubic B-Spline Curve Generation..." << std::endl;

    std::vector<SplinePoint2D> controlPoints = {
        {0.0, 0.0},
        {10.0, 15.0},
        {25.0, 5.0},
        {40.0, 20.0},
        {50.0, 0.0}
    };

    BSplineResult res = FastenerEngine::GenerateCubicBSpline(controlPoints, 50);

    assert(res.success);
    assert(res.degree == 3);
    assert(res.poleCount == 5);
    assert(res.knotCount == 9);
    assert(res.sampledPoints.size() == 50);
    assert(res.lengthMm > 50.0);

    // First and last points should match endpoints
    assert(std::abs(res.sampledPoints.front().u - 0.0) < 1e-3);
    assert(std::abs(res.sampledPoints.front().v - 0.0) < 1e-3);
    assert(std::abs(res.sampledPoints.back().u - 50.0) < 1e-3);
    assert(std::abs(res.sampledPoints.back().v - 0.0) < 1e-3);

    std::cout << "  Passed: Cubic B-Spline interpolated with 50 samples, arc length=" << res.lengthMm << " mm" << std::endl;
}

int main() {
    std::cout << "=== AndroCAD Fastener, Thread & Projection Test Suite ===" << std::endl;

    try {
        TestCounterboreHole();
        TestCountersinkHole();
        TestThreadedBolt();
        TestEdgeProjection();
        TestCubicBSpline();

        std::cout << "\nAll 5 Fastener & Projection tests PASSED successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fastener Test FAILED with exception: " << e.what() << std::endl;
        return 1;
    }
}
