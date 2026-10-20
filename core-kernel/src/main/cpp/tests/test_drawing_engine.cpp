// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "DrawingEngine.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace androcad;

void TestOrthographicProjections() {
    std::cout << "[Test 1/5] Testing 2D Multi-View Orthographic & Isometric Projections..." << std::endl;
    // Box: 100 x 50 x 30 mm
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 50.0, 30.0).Shape();

    DrawingView front = DrawingEngine::ProjectView(box, ViewType::Front, 130.0, 95.0, 1.0);
    assert(front.type == ViewType::Front);
    assert(!front.segments.empty());
    assert(std::abs(front.width - 100.0) < 0.1);
    assert(std::abs(front.height - 30.0) < 0.1);

    DrawingView top = DrawingEngine::ProjectView(box, ViewType::Top, 130.0, 205.0, 1.0);
    assert(top.type == ViewType::Top);
    assert(!top.segments.empty());
    assert(std::abs(top.width - 100.0) < 0.1);
    assert(std::abs(top.height - 50.0) < 0.1);

    DrawingView right = DrawingEngine::ProjectView(box, ViewType::Right, 255.0, 95.0, 1.0);
    assert(right.type == ViewType::Right);
    assert(!right.segments.empty());
    assert(std::abs(right.width - 50.0) < 0.1);
    assert(std::abs(right.height - 30.0) < 0.1);

    DrawingView iso = DrawingEngine::ProjectView(box, ViewType::Isometric, 275.0, 205.0, 1.0);
    assert(iso.type == ViewType::Isometric);
    assert(!iso.segments.empty());
    assert(iso.width > 0.0 && iso.height > 0.0);

    std::cout << "  ✓ Front: " << front.width << "x" << front.height
              << " mm, Top: " << top.width << "x" << top.height
              << " mm, Right: " << right.width << "x" << right.height << " mm" << std::endl;
}

void TestHiddenLineClassification() {
    std::cout << "[Test 2/5] Testing Hidden Line Removal & Silhouette Line Classification..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(80.0, 40.0, 20.0).Shape();

    DrawingView front = DrawingEngine::ProjectView(box, ViewType::Front, 100.0, 100.0, 1.0);
    int visibleCount = 0;
    int hiddenCount = 0;

    for (const auto& seg : front.segments) {
        if (seg.isHidden) hiddenCount++;
        else visibleCount++;
    }

    assert(visibleCount > 0);
    std::cout << "  ✓ Classified Front View: " << visibleCount << " visible segments, "
              << hiddenCount << " hidden/occluded segments" << std::endl;
}

void TestSectionCutView() {
    std::cout << "[Test 3/5] Testing Section Cut Plane Slicing & 45° Cross-Hatching..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 60.0, 40.0).Shape();

    DrawingView section = DrawingEngine::CreateSectionView(box, 30.0, 150.0, 100.0, 1.0);
    assert(section.type == ViewType::Section);
    assert(!section.segments.empty());
    assert(std::abs(section.width - 100.0) < 0.1);
    assert(std::abs(section.height - 40.0) < 0.1);

    // Verify hatching lines exist (more segments than just the 4 boundary edges)
    assert(section.segments.size() > 4);

    std::cout << "  ✓ Section View at Y=30.0 created: " << section.segments.size()
              << " segments (boundary + cross-hatching)" << std::endl;
}

void TestDimensionAnnotations() {
    std::cout << "[Test 4/5] Testing Dimension Callouts & Leader Lines..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(120.0, 70.0, 35.0).Shape();

    DrawingSheet sheet = DrawingEngine::GenerateStandardDrawingSheet(box, "Bracket1", 1.0);
    assert(sheet.views.size() == 4);

    bool foundWidth = false;
    bool foundHeight = false;

    for (const auto& v : sheet.views) {
        for (const auto& dim : v.dimensions) {
            if (dim.text.find("120.0") != std::string::npos) foundWidth = true;
            if (dim.text.find("35.0") != std::string::npos) foundHeight = true;
        }
    }

    assert(foundWidth);
    assert(foundHeight);
    std::cout << "  ✓ Dimension callouts accurately placed for Width (120.0 mm) and Height (35.0 mm)" << std::endl;
}

void TestVectorExportSVGandDXF() {
    std::cout << "[Test 5/5] Testing Vector SVG & AutoCAD DXF Sheet Export..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 50.0, 30.0).Shape();
    DrawingSheet sheet = DrawingEngine::GenerateStandardDrawingSheet(box, "FlangeMount", 1.0);

    // 1. SVG
    std::string svg = sheet.ToSVG();
    assert(!svg.empty());
    assert(svg.find("<svg") != std::string::npos);
    assert(svg.find("</svg>") != std::string::npos);
    assert(svg.find("ANDROCAD") != std::string::npos);
    assert(svg.find("FlangeMount") != std::string::npos);

    // 2. DXF
    std::string dxf = sheet.ToDXF();
    assert(!dxf.empty());
    assert(dxf.find("SECTION") != std::string::npos);
    assert(dxf.find("BORDER") != std::string::npos);
    assert(dxf.find("VISIBLE") != std::string::npos);
    assert(dxf.find("DIMENSIONS") != std::string::npos);
    assert(dxf.find("EOF") != std::string::npos);

    std::cout << "  ✓ Vector SVG generated (" << svg.size() << " bytes), DXF generated ("
              << dxf.size() << " bytes)" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running AndroCAD Drawing Engine Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    TestOrthographicProjections();
    TestHiddenLineClassification();
    TestSectionCutView();
    TestDimensionAnnotations();
    TestVectorExportSVGandDXF();

    std::cout << "========================================" << std::endl;
    std::cout << "All 5 Drawing Engine tests PASSED!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
