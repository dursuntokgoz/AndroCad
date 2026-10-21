// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "DrawingEngine.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <cassert>
#include <iostream>
#include <cmath>

using namespace androcad;

void TestGDTFeatureControlFrames() {
    std::cout << "[Test 1] Testing GD&T Feature Control Frames formatting & SVG..." << std::endl;

    FeatureControlFrame fcfPos = DrawingEngine::CreateGDTFrame(
        GDTCharacteristic::POSITION, 0.05, true, MaterialCondition::MMC,
        "A", "B", "C", 50.0, 40.0, 70.0, 60.0
    );

    std::string strPos = fcfPos.FormattedString();
    std::cout << "  Position FCF: " << strPos << std::endl;
    assert(strPos.find("POS") != std::string::npos);
    assert(strPos.find("⌀") != std::string::npos);
    assert(strPos.find("0.050") != std::string::npos);
    assert(strPos.find("A") != std::string::npos);
    assert(strPos.find("B") != std::string::npos);
    assert(strPos.find("C") != std::string::npos);

    std::string svgPos = fcfPos.ToSVG();
    assert(svgPos.find("<g id=\"gdt_frame\">") != std::string::npos);
    assert(svgPos.find("<rect") != std::string::npos);
    assert(svgPos.find("<line") != std::string::npos);

    FeatureControlFrame fcfFlat = DrawingEngine::CreateGDTFrame(
        GDTCharacteristic::FLATNESS, 0.02, false, MaterialCondition::NONE,
        "", "", "", 30.0, 20.0, 40.0, 35.0
    );
    std::string strFlat = fcfFlat.FormattedString();
    std::cout << "  Flatness FCF: " << strFlat << std::endl;
    assert(strFlat.find("FLAT") != std::string::npos);
    assert(strFlat.find("0.020") != std::string::npos);

    FeatureControlFrame fcfPerp = DrawingEngine::CreateGDTFrame(
        GDTCharacteristic::PERPENDICULARITY, 0.03, false, MaterialCondition::MMC,
        "A", "B", "", 10.0, 10.0, 20.0, 25.0
    );
    std::string strPerp = fcfPerp.FormattedString();
    std::cout << "  Perpendicularity FCF: " << strPerp << std::endl;
    assert(strPerp.find("PERP") != std::string::npos);
    assert(strPerp.find("0.030") != std::string::npos);

    std::cout << "  Passed: GD&T Feature Control Frames verified per ASME Y14.5 / ISO 1101" << std::endl;
}

void TestBOMHierarchyAndExtraction() {
    std::cout << "[Test 2] Testing Bill of Materials (BOM) Table structure & SVG..." << std::endl;

    std::vector<BOMItem> items;
    items.push_back({1, "AC-1001", "Base Mount Flange", 1, "Steel (Structural)", 420.5, 420.5});
    items.push_back({2, "AC-1002", "Bearing Bushing", 2, "Phosphor Bronze", 65.0, 130.0});
    items.push_back({3, "ISO-4017-M6", "Hex Head Bolt M6x25", 4, "Grade 8.8 Steel", 12.5, 50.0});
    items.push_back({4, "ISO-7089-6", "Flat Plain Washer 6mm", 4, "Stainless Steel", 2.0, 8.0});

    BOMTable table;
    table.originX = 260.0;
    table.originY = 20.0;
    table.items = items;

    assert(table.items.size() == 4);
    double totalAssemblyMass = 0.0;
    for (const auto& item : table.items) {
        totalAssemblyMass += item.totalMassGrams;
    }
    assert(std::abs(totalAssemblyMass - 608.5) < 1e-4);

    std::string svgTable = table.ToSVG();
    assert(svgTable.find("<g id=\"bom_table\">") != std::string::npos);
    assert(svgTable.find("ITEM") != std::string::npos);
    assert(svgTable.find("PART NO") != std::string::npos);
    assert(svgTable.find("AC-1001") != std::string::npos);
    assert(svgTable.find("ISO-4017-M6") != std::string::npos);

    std::cout << "  Passed: BOM table compiled with 4 components, total mass=" 
              << totalAssemblyMass << " g" << std::endl;
}

void TestBOMCalloutBalloons() {
    std::cout << "[Test 3] Testing BOM Leader Callout Balloons..." << std::endl;

    BOMBalloon b1{1, 120.0, 150.0, 80.0, 200.0, 6.0};
    BOMBalloon b2{2, 140.0, 170.0, 180.0, 220.0, 6.0};

    assert(b1.itemNumber == 1);
    assert(b2.itemNumber == 2);
    assert(b1.radius == 6.0);

    std::string svgB1 = b1.ToSVG();
    assert(svgB1.find("<circle") != std::string::npos);
    assert(svgB1.find("<line") != std::string::npos);
    assert(svgB1.find(">1<") != std::string::npos);

    std::string svgB2 = b2.ToSVG();
    assert(svgB2.find(">2<") != std::string::npos);

    std::cout << "  Passed: BOM leader balloons verified with circular badge and component pointer" << std::endl;
}

void TestDetailAndAuxiliaryViews() {
    std::cout << "[Test 4] Testing Enlarged Circular Detail View & Oblique Auxiliary View..." << std::endl;

    TopoDS_Shape box = BRepPrimAPI_MakeBox(80.0, 40.0, 30.0).Shape();

    DrawingView frontView = DrawingEngine::ProjectView(box, ViewType::Front, 100.0, 100.0, 1.0);
    assert(!frontView.segments.empty());

    // Create 2:1 enlarged detail view of corner
    DrawingView detailView = DrawingEngine::CreateDetailView(
        frontView, 100.0, 100.0, 15.0, 2.0, 250.0, 180.0, "B"
    );

    assert(detailView.type == ViewType::Detail);
    assert(detailView.name == "DETAIL B (2:1)");
    assert(!detailView.segments.empty());
    assert(detailView.scale == 2.0);
    assert(detailView.width > 20.0);

    // Create oblique auxiliary view
    DrawingView auxView = DrawingEngine::CreateAuxiliaryView(
        box, 1.0, 1.0, 1.0, 250.0, 80.0, 1.0
    );

    assert(auxView.type == ViewType::Auxiliary);
    assert(!auxView.segments.empty());
    assert(auxView.width > 0.0);
    assert(auxView.height > 0.0);

    std::cout << "  Passed: Detail view segments=" << detailView.segments.size()
              << ", Auxiliary view segments=" << auxView.segments.size() << std::endl;
}

void TestMultiSheetDocumentExport() {
    std::cout << "[Test 5] Testing Multi-Sheet Production Drafting Document Generation..." << std::endl;

    TopoDS_Shape bracket = BRepPrimAPI_MakeBox(100.0, 50.0, 25.0).Shape();

    std::vector<BOMItem> bom;
    bom.push_back({1, "AC-MAIN-BODY", "Main Bracket Body", 1, "Aluminum 6061-T6", 320.0, 320.0});
    bom.push_back({2, "AC-FASTENER-M8", "Socket Cap Screw M8", 4, "High Tensile Steel", 18.0, 72.0});

    DrawingDocument doc = DrawingEngine::GenerateMultiSheetDocument(bracket, bom, "Precision Angle Bracket");

    assert(doc.sheets.size() == 3);

    // Sheet 1 checks
    const auto& s1 = doc.sheets[0];
    assert(s1.sheetIndex == 1);
    assert(!s1.views.empty());
    assert(s1.views[0].type == ViewType::Isometric);
    assert(!s1.views[0].balloons.empty());
    assert(s1.bomTable.items.size() == 2);

    // Sheet 2 checks
    const auto& s2 = doc.sheets[1];
    assert(s2.sheetIndex == 2);
    assert(s2.views.size() >= 3);
    assert(!s2.views[0].gdtFrames.empty());

    // Sheet 3 checks
    const auto& s3 = doc.sheets[2];
    assert(s3.sheetIndex == 3);
    assert(s3.views.size() == 3); // Section, Detail, Auxiliary

    std::string multiSvg = doc.ToMultiSheetSVG();
    assert(multiSvg.find("SHEET 1:") != std::string::npos);
    assert(multiSvg.find("SHEET 2:") != std::string::npos);
    assert(multiSvg.find("SHEET 3:") != std::string::npos);
    assert(multiSvg.find("bom_table") != std::string::npos);
    assert(multiSvg.find("gdt_frame") != std::string::npos);

    std::string multiDxf = doc.ToMultiSheetDXF();
    assert(multiDxf.find("SECTION") != std::string::npos);
    assert(multiDxf.find("DIMENSIONS") != std::string::npos);

    std::cout << "  Passed: Multi-sheet document successfully compiled 3 sheets with complete SVG and DXF output" << std::endl;
}

int main() {
    std::cout << "======================================================" << std::endl;
    std::cout << "Running AndroCAD GD&T, BOM & Multi-Sheet Drafting Tests" << std::endl;
    std::cout << "======================================================" << std::endl;

    TestGDTFeatureControlFrames();
    TestBOMHierarchyAndExtraction();
    TestBOMCalloutBalloons();
    TestDetailAndAuxiliaryViews();
    TestMultiSheetDocumentExport();

    std::cout << "======================================================" << std::endl;
    std::cout << "ALL 5 GD&T, BOM & MULTI-SHEET DRAFTING TESTS PASSED!" << std::endl;
    std::cout << "======================================================" << std::endl;

    return 0;
}
