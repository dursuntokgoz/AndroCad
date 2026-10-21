// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "CAMEngine.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <cassert>
#include <iostream>
#include <cmath>

using namespace androcad;

void TestAdaptiveClearing3D() {
    std::cout << "[Test 1] Testing 3D Adaptive Clearing Surfacing Toolpath..." << std::endl;

    CAMEngine cam;
    ToolDefinition roughEndmill = cam.GetTool(1); // 10mm flat endmill

    CAMOperation op = cam.GenerateAdaptiveClearing3D(
        101, "Adaptive Roughing 3D", roughEndmill,
        0.0, 60.0, 0.0, 40.0, 20.0, 10.0, 2.5, 1.5
    );

    assert(op.type == OperationType::ADAPTIVE_3D);
    assert(op.pathPoints.size() > 50);
    assert(op.totalDistanceMm > 100.0);
    assert(op.machiningTimeSec > 0.0);

    // Verify bounds of path points
    for (const auto& pt : op.pathPoints) {
        if (pt.motion == MotionType::LINEAR_CUT) {
            assert(pt.z >= 10.0 - 1e-4 && pt.z <= 20.0 + 1e-4);
            assert(pt.x >= 0.0 - 1e-4 && pt.x <= 60.0 + 1e-4);
            assert(pt.y >= 0.0 - 1e-4 && pt.y <= 40.0 + 1e-4);
        }
    }

    std::cout << "  Passed: Adaptive 3D clearing generated " << op.pathPoints.size() 
              << " points, total cut distance=" << op.totalDistanceMm 
              << " mm, time=" << op.machiningTimeSec << " s" << std::endl;
}

void TestScallop3D() {
    std::cout << "[Test 2] Testing 3D Constant Scallop Finishing Toolpath..." << std::endl;

    CAMEngine cam;
    ToolDefinition ballEndmill = cam.GetTool(2); // 6mm ball endmill

    CAMOperation op = cam.GenerateScallop3D(
        102, "Scallop 3D Finish", ballEndmill,
        0.0, 50.0, 0.0, 50.0, 20.0, 15.0, 1.0, 0.05
    );

    assert(op.type == OperationType::SCALLOP_3D);
    assert(op.pathPoints.size() >= 20);
    assert(op.totalDistanceMm > 50.0);
    assert(op.machiningTimeSec > 0.0);

    for (const auto& pt : op.pathPoints) {
        if (pt.motion == MotionType::LINEAR_CUT) {
            assert(pt.z >= 15.0 - 1e-4 && pt.z <= 20.0 + 1e-4);
        }
    }

    std::cout << "  Passed: Scallop 3D generated " << op.pathPoints.size() 
              << " points, path length=" << op.totalDistanceMm << " mm" << std::endl;
}

void TestParallel3D() {
    std::cout << "[Test 3] Testing 3D Parallel Raster Surfacing Toolpath..." << std::endl;

    CAMEngine cam;
    ToolDefinition ballEndmill = cam.GetTool(2);

    CAMOperation op = cam.GenerateParallel3D(
        103, "Parallel 3D Raster", ballEndmill,
        0.0, 80.0, 0.0, 40.0, 25.0, 20.0, 2.0, 0.0
    );

    assert(op.type == OperationType::PARALLEL_3D);
    assert(op.pathPoints.size() >= 20);
    assert(op.totalDistanceMm > 80.0);

    std::cout << "  Passed: Parallel 3D raster generated " << op.pathPoints.size() 
              << " points, path length=" << op.totalDistanceMm << " mm" << std::endl;
}

void TestFDMSlicerSolid() {
    std::cout << "[Test 4] Testing Additive FDM Slicer on OCCT Solid Shape..." << std::endl;

    TopoDS_Shape box = BRepPrimAPI_MakeBox(40.0, 30.0, 10.0).Shape();

    SlicerConfig config;
    config.layerHeightMm = 0.2;
    config.initialLayerHeightMm = 0.2;
    config.nozzleDiameterMm = 0.4;
    config.filamentDiameterMm = 1.75;
    config.infillDensityPercent = 25.0;
    config.infillPattern = InfillPattern::RECTILINEAR;

    SlicerResult res = CAMEngine::SliceSolid(box, config);

    assert(res.success);
    assert(res.totalLayers == 50);
    assert(res.layers.size() == 50);
    assert(res.totalFilamentMm > 100.0);
    assert(res.totalFilamentGrams > 0.1);
    assert(res.estimatedPrintTimeMin > 0.0);
    assert(!res.gcode.empty());
    assert(res.gcode.find("M104 S205") != std::string::npos);
    assert(res.gcode.find("LAYER:50") != std::string::npos);

    // Verify first layer segments
    const auto& l1 = res.layers.front();
    assert(l1.layerIndex == 1);
    assert(!l1.segments.empty());

    std::cout << "  Passed: FDM Slicer sliced solid into " << res.totalLayers 
              << " layers, total filament=" << res.totalFilamentMm << " mm (" 
              << res.totalFilamentGrams << " g), est time=" << res.estimatedPrintTimeMin << " min" << std::endl;
}

void TestHoneycombInfillAndCustomSlicerConfig() {
    std::cout << "[Test 5] Testing Honeycomb Infill & Custom Print Temperature Settings..." << std::endl;

    SlicerConfig config;
    config.layerHeightMm = 0.3;
    config.initialLayerHeightMm = 0.3;
    config.infillDensityPercent = 40.0;
    config.infillPattern = InfillPattern::HONEYCOMB;
    config.nozzleTempC = 220.0;
    config.bedTempC = 70.0;
    config.printSpeedMmS = 50.0;

    SlicerResult res = CAMEngine::SliceBoundingBox(0.0, 50.0, 0.0, 50.0, 0.0, 15.0, config);

    assert(res.success);
    assert(res.totalLayers == 50);
    assert(res.layers.size() == 50);
    assert(res.gcode.find("M104 S220") != std::string::npos);
    assert(res.gcode.find("M140 S70") != std::string::npos);
    assert(res.gcode.find("End G-code") != std::string::npos);

    std::cout << "  Passed: Honeycomb infill generated across 50 layers, G-code size=" 
              << res.gcode.size() << " bytes" << std::endl;
}

int main() {
    std::cout << "=== AndroCAD 3D CAM & Additive Slicer Test Suite ===" << std::endl;

    try {
        TestAdaptiveClearing3D();
        TestScallop3D();
        TestParallel3D();
        TestFDMSlicerSolid();
        TestHoneycombInfillAndCustomSlicerConfig();

        std::cout << "\nAll 5 3D CAM & Additive Slicer tests PASSED successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "CAM & Slicer Test FAILED with exception: " << e.what() << std::endl;
        return 1;
    }
}
