// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "SheetMetalEngine.h"
#include <BRepCheck_Analyzer.hxx>
#include <iostream>
#include <cassert>

using namespace androcad;

void TestBendCalculations() {
    std::cout << "[Test 1/5] Testing K-Factor Bend Deduction & Flat Length calculations..." << std::endl;
    SheetMetalRule rule;
    rule.thickness = 2.0;
    rule.bendRadius = 2.0;
    rule.kFactor = 0.44;

    double angle90 = M_PI / 2.0;
    double ba = rule.CalculateBendAllowance(angle90);
    // Neutral radius = 2.0 + 0.44 * 2.0 = 2.88 mm.
    // BA = (PI / 2) * 2.88 ≈ 4.5238934 mm.
    assert(std::abs(ba - 4.5238934) < 0.001);

    double bd = rule.CalculateBendDeduction(angle90);
    // OSSB = tan(45 deg) * (2.0 + 2.0) = 4.0 mm.
    // BD = 2 * 4.0 - BA = 8.0 - 4.5238934 ≈ 3.4761066 mm.
    assert(std::abs(bd - 3.4761066) < 0.001);

    double leg1 = 40.0;
    double leg2 = 100.0;
    double flatLen = rule.CalculateFlatLength(leg1, leg2, angle90);
    // Flat length = 40 + 100 - 3.4761066 = 136.5238934 mm.
    assert(std::abs(flatLen - 136.5238934) < 0.001);

    std::cout << "  ✓ BA: " << ba << " mm, BD: " << bd << " mm, Flat Length: " << flatLen << " mm" << std::endl;
}

void TestCreateBasePlate() {
    std::cout << "[Test 2/5] Testing Sheet Metal Base Plate Generation..." << std::endl;
    SheetMetalRule rule;
    rule.thickness = 2.0;

    TopoDS_Shape plate = SheetMetalEngine::CreateBasePlate(50.0, 100.0, rule);
    BRepCheck_Analyzer analyzer(plate);
    assert(analyzer.IsValid());

    MeshPayload payload = SheetMetalEngine::ToMeshPayload(plate);
    assert(payload.success);
    // Expected volume: 100 * 50 * 2 = 10,000 mm³
    assert(std::abs(payload.volume - 10000.0) < 1.0);
    assert(payload.triangleCount > 0);
    std::cout << "  ✓ Base plate valid, volume: " << payload.volume << " mm³" << std::endl;
}

void TestCreateUBracket3D() {
    std::cout << "[Test 3/5] Testing 3D U-Bracket Solid Construction with Cylindrical Fillets..." << std::endl;
    SheetMetalRule rule;
    rule.thickness = 2.0;
    rule.bendRadius = 2.0;

    double baseLength = 100.0;
    double flangeHeight = 40.0;
    double width = 50.0;

    TopoDS_Shape uBracket = SheetMetalEngine::CreateUBracket(baseLength, flangeHeight, width, rule);
    BRepCheck_Analyzer analyzer(uBracket);
    assert(analyzer.IsValid());

    MeshPayload payload = SheetMetalEngine::ToMeshPayload(uBracket);
    assert(payload.success);
    assert(payload.volume > 0.0);
    assert(payload.faceCount >= 8); // At least top, bottom, 2 outer flanges, 2 inner flanges, 2 ends, bend faces
    std::cout << "  ✓ U-Bracket solid valid, mesh volume: " << payload.volume << " mm³, faces: " << payload.faceCount << std::endl;
}

void TestUnfoldUBracket() {
    std::cout << "[Test 4/5] Testing U-Bracket Unfolding to Flat Pattern & Bend Line Layout..." << std::endl;
    SheetMetalRule rule;
    rule.thickness = 2.0;
    rule.bendRadius = 2.0;
    rule.kFactor = 0.44;

    double baseLength = 100.0;
    double flangeHeight = 40.0;
    double width = 50.0;

    FlatPattern pattern = SheetMetalEngine::UnfoldUBracket(baseLength, flangeHeight, width, rule);
    assert(pattern.success);

    double bd = rule.CalculateBendDeduction(M_PI / 2.0);
    double expectedFlatLength = baseLength + 2.0 * flangeHeight - 2.0 * bd;
    assert(std::abs(pattern.flatLength - expectedFlatLength) < 0.001);
    assert(std::abs(pattern.flatWidth - 50.0) < 0.001);
    assert(std::abs(pattern.flatArea - (expectedFlatLength * 50.0)) < 0.01);

    // Verify 2 bend lines
    assert(pattern.bends.size() == 2);
    const auto& b1 = pattern.bends[0];
    const auto& b2 = pattern.bends[1];

    assert(b1.angleDeg == 90.0);
    assert(b1.isBendUp == true);
    assert(b2.angleDeg == 90.0);
    assert(b2.isBendUp == true);

    // Check symmetry and distance between bend lines
    double distBetweenBends = b2.startX - b1.startX;
    double expectedDist = baseLength - bd;
    assert(std::abs(distBetweenBends - expectedDist) < 0.001);

    std::cout << "  ✓ Flat length: " << pattern.flatLength << " mm, Width: " << pattern.flatWidth
              << " mm, Bend distance: " << distBetweenBends << " mm" << std::endl;
}

void TestDXFGeneration() {
    std::cout << "[Test 5/5] Testing Flat Pattern 2D DXF Export (CNC Laser Cut & Bend Layers)..." << std::endl;
    SheetMetalRule rule;
    FlatPattern pattern = SheetMetalEngine::UnfoldUBracket(100.0, 40.0, 50.0, rule);
    assert(pattern.success);

    std::string dxf = pattern.dxfContent;
    assert(!dxf.empty());
    assert(dxf.find("SECTION") != std::string::npos);
    assert(dxf.find("ENTITIES") != std::string::npos);
    assert(dxf.find("CUT") != std::string::npos);
    assert(dxf.find("BEND") != std::string::npos);
    assert(dxf.find("EOF") != std::string::npos);

    std::cout << "  ✓ Valid DXF file generated (" << dxf.size() << " bytes), contains CUT and BEND layers" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running AndroCAD Sheet Metal Engine Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    TestBendCalculations();
    TestCreateBasePlate();
    TestCreateUBracket3D();
    TestUnfoldUBracket();
    TestDXFGeneration();

    std::cout << "========================================" << std::endl;
    std::cout << "All 5 Sheet Metal Engine tests PASSED!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
