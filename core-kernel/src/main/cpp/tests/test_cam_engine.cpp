// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "CAMEngine.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <cassert>
#include <iostream>
#include <cmath>

using namespace androcad;

void TestToolLibraryAndStockSetup() {
    std::cout << "[Test 1] Testing Tool Library and Auto Stock Setup..." << std::endl;

    CAMEngine cam;
    const auto& library = cam.GetToolLibrary();
    assert(library.size() >= 5);

    ToolDefinition t1 = cam.GetTool(1);
    assert(t1.diameterMm == 10.0);
    assert(t1.type == ToolType::FLAT_ENDMILL);
    assert(t1.spindleRpm == 8000.0);
    assert(t1.cuttingFeedrateMmMin == 1200.0);

    ToolDefinition t5 = cam.GetTool(5);
    assert(t5.diameterMm == 5.0);
    assert(t5.type == ToolType::TWIST_DRILL);

    // Auto-stock from OCCT B-Rep solid
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 50.0, 20.0).Shape();
    cam.AutoStockFromShape(box, 5.0, 1.0);

    const auto& stock = cam.GetStock();
    assert(stock.minX == -5.0 && stock.maxX == 105.0);
    assert(stock.minY == -5.0 && stock.maxY == 55.0);
    assert(stock.minZ == 0.0 && stock.maxZ == 21.0);
    assert(stock.stockTopZ == 21.0);
    assert(stock.clearanceZ == 31.0);

    std::cout << "  Passed: Tools=" << library.size()
              << ", Stock Box=[" << stock.minX << ", " << stock.maxX << "]x["
              << stock.minY << ", " << stock.maxY << "]x[" << stock.minZ << ", " << stock.maxZ << "]" << std::endl;
}

void TestFacingOperation() {
    std::cout << "[Test 2] Testing Facing toolpath generation..." << std::endl;

    CAMEngine cam;
    StockSetup stock;
    stock.minX = 0.0; stock.maxX = 100.0;
    stock.minY = 0.0; stock.maxY = 50.0;
    stock.minZ = 0.0; stock.maxZ = 21.0;
    stock.stockTopZ = 21.0; stock.retractZ = 23.0; stock.clearanceZ = 30.0;
    cam.SetStock(stock);

    ToolDefinition tool = cam.GetTool(1); // 10mm Flat
    CAMOperation facing = cam.GenerateFacingOperation(1, "Face Top", tool, 7.0, 21.0, 20.0);

    assert(!facing.pathPoints.empty());
    assert(facing.type == OperationType::FACING);

    // First point must be at clearance Z
    assert(facing.pathPoints.front().motion == MotionType::RAPID);
    assert(facing.pathPoints.front().z == stock.clearanceZ);

    // Last point must retract to clearance Z
    assert(facing.pathPoints.back().motion == MotionType::RAPID);
    assert(facing.pathPoints.back().z == stock.clearanceZ);

    // Lead-in points should be off the stock in X (outside [0, 100])
    bool hasLeadInAir = false;
    for (const auto& pt : facing.pathPoints) {
        if (pt.x < 0.0 || pt.x > 100.0) {
            hasLeadInAir = true;
            break;
        }
    }
    assert(hasLeadInAir);

    std::cout << "  Passed: Facing generated " << facing.pathPoints.size() << " trajectory points" << std::endl;
}

void TestPocketAndContourToolpaths() {
    std::cout << "[Test 3] Testing 2D Pocket & 2D Contour toolpath generation..." << std::endl;

    CAMEngine cam;
    StockSetup stock;
    stock.minX = 0.0; stock.maxX = 100.0;
    stock.minY = 0.0; stock.maxY = 60.0;
    stock.minZ = 0.0; stock.maxZ = 20.0;
    stock.stockTopZ = 20.0; stock.retractZ = 22.0; stock.clearanceZ = 30.0;
    cam.SetStock(stock);

    ToolDefinition tool10 = cam.GetTool(1); // 10mm Flat
    ToolDefinition tool6 = cam.GetTool(2);  // 6mm Flat

    // 2D Pocket inside [20, 80] x [15, 45], depth = 10mm (stepdown = 2.5mm -> 4 levels)
    CAMOperation pocket = cam.GeneratePocket2DOperation(2, "Cavity", tool10, 20.0, 80.0, 15.0, 45.0, 20.0, 10.0, 2.5, 6.0);
    assert(pocket.pathPoints.size() > 20);

    // Verify depth passes exist at Z = 17.5, 15.0, 12.5, 10.0
    bool reachedBottomZ = false;
    for (const auto& pt : pocket.pathPoints) {
        if (std::abs(pt.z - 10.0) < 1e-3) {
            reachedBottomZ = true;
            break;
        }
    }
    assert(reachedBottomZ);

    // 2D Contour perimeter cut [0, 100] x [0, 60] with 6mm tool (radius = 3mm)
    CAMOperation contour = cam.GenerateContour2DOperation(3, "Perimeter", tool6, 0.0, 100.0, 0.0, 60.0, 20.0, 0.0, 5.0);
    assert(contour.pathPoints.size() > 10);

    // Verify tool centerline offset is 3mm outside part boundary
    bool hasOffset = false;
    for (const auto& pt : contour.pathPoints) {
        if (std::abs(pt.x - (-3.0)) < 1e-3 || std::abs(pt.x - 103.0) < 1e-3) {
            hasOffset = true;
            break;
        }
    }
    assert(hasOffset);

    std::cout << "  Passed: Pocket points=" << pocket.pathPoints.size() << ", Contour points=" << contour.pathPoints.size() << std::endl;
}

void TestPeckDrillingCycles() {
    std::cout << "[Test 4] Testing Peck Drilling deep-hole cycle..." << std::endl;

    CAMEngine cam;
    StockSetup stock;
    stock.minX = 0.0; stock.maxX = 100.0;
    stock.minY = 0.0; stock.maxY = 50.0;
    stock.minZ = 0.0; stock.maxZ = 20.0;
    stock.stockTopZ = 20.0; stock.retractZ = 22.0; stock.clearanceZ = 30.0;
    cam.SetStock(stock);

    ToolDefinition drill5 = cam.GetTool(5); // 5mm Twist Drill
    std::vector<std::pair<double, double>> holes = {{25.0, 25.0}, {75.0, 25.0}};

    // Hole depth = 22mm (depth > 2.5 * 5mm = 12.5mm -> triggers peck drilling)
    CAMOperation peck = cam.GenerateDrillOperation(4, "Peck Drill", drill5, holes, 20.0, 22.0, 3.0);

    assert(!peck.pathPoints.empty());
    // Verify chip-breaking retracts to retract plane Z=22.0
    int retractCount = 0;
    for (const auto& pt : peck.pathPoints) {
        if (std::abs(pt.z - 22.0) < 1e-3 && pt.motion == MotionType::RAPID) {
            retractCount++;
        }
    }
    assert(retractCount >= 4); // Multiple pecks per hole

    std::cout << "  Passed: Peck drilling generated " << peck.pathPoints.size()
              << " cycle points with " << retractCount << " chip-break retracts" << std::endl;
}

void TestCompleteGCodePostProcessor() {
    std::cout << "[Test 5] Testing complete G-Code post-processor & multi-dialect output..." << std::endl;

    // Test GRBL / ISO dialect
    GCodeResult resGrbl = CAMEngine::GenerateCompleteBracketSetup(100.0, 50.0, 20.0, 10.0, PostProcessorDialect::ISO_GRBL);
    assert(resGrbl.success);
    assert(resGrbl.lineCount > 100);
    assert(resGrbl.totalCuttingTimeSec > 10.0);
    assert(resGrbl.totalRapidTimeSec > 1.0);
    assert(resGrbl.totalLengthMm > 200.0);
    assert(resGrbl.materialVolumeRemovedCm3 > 0.0);

    // Verify standard G-Code commands in GRBL
    assert(resGrbl.gcode.find("G21") != std::string::npos);
    assert(resGrbl.gcode.find("G90") != std::string::npos);
    assert(resGrbl.gcode.find("G00") != std::string::npos);
    assert(resGrbl.gcode.find("G01") != std::string::npos);
    assert(resGrbl.gcode.find("M03") != std::string::npos);
    assert(resGrbl.gcode.find("M05") != std::string::npos);
    assert(resGrbl.gcode.find("M30") != std::string::npos);

    // Test Haas CNC dialect
    GCodeResult resHaas = CAMEngine::GenerateCompleteBracketSetup(100.0, 50.0, 20.0, 10.0, PostProcessorDialect::HAAS_CNC);
    assert(resHaas.success);
    assert(resHaas.gcode.find("O1001") != std::string::npos);
    assert(resHaas.gcode.find("G43 H1") != std::string::npos);
    assert(resHaas.gcode.find("G28 G91 Z0.") != std::string::npos);

    // Test Fanuc CNC dialect
    GCodeResult resFanuc = CAMEngine::GenerateCompleteBracketSetup(100.0, 50.0, 20.0, 10.0, PostProcessorDialect::FANUC_CNC);
    assert(resFanuc.success);
    assert(resFanuc.gcode.find("O1001") != std::string::npos);

    // Verify Visual Lines for GLES Viewport
    const auto& visual = resGrbl.visualPath;
    assert(!visual.lineVertices.empty());
    assert(visual.rapidCount > 0);
    assert(visual.cutCount > 0);
    assert(visual.plungeCount > 0);
    assert(visual.lineVertices.size() % 12 == 0); // 2 vertices per line, 6 floats each (X, Y, Z, R, G, B)

    std::cout << "  Passed: Lines=" << resGrbl.lineCount
              << ", Cutting Time=" << resGrbl.totalCuttingTimeSec << " s"
              << ", Rapid Time=" << resGrbl.totalRapidTimeSec << " s"
              << ", Volume Removed=" << resGrbl.materialVolumeRemovedCm3 << " cm³"
              << ", Visual Vertex Count=" << visual.lineVertices.size() / 6 << std::endl;
}

int main() {
    std::cout << "=== AndroCAD CAM & G-Code Post-Processor Test Suite ===" << std::endl;

    try {
        TestToolLibraryAndStockSetup();
        TestFacingOperation();
        TestPocketAndContourToolpaths();
        TestPeckDrillingCycles();
        TestCompleteGCodePostProcessor();

        std::cout << "\nAll 5 CAM Engine tests PASSED successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "CAM Test FAILED with exception: " << e.what() << std::endl;
        return 1;
    }
}
