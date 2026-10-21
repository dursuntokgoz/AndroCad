// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "MaterialEngine.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace androcad;

void TestMaterialLibrary() {
    std::cout << "[Test 1/5] Testing Engineering Material Database & PBR Properties..." << std::endl;
    auto mats = MaterialEngine::GetDefaultMaterials();
    assert(mats.size() >= 7);

    Material steel = MaterialEngine::GetMaterialByName("Steel (Structural)");
    assert(std::abs(steel.density - 7.85) < 0.01);
    assert(steel.youngModulusGPa == 210.0);
    assert(steel.appearance.metallic > 0.8f);

    Material alum = MaterialEngine::GetMaterialByName("Aluminum 6061-T6");
    assert(std::abs(alum.density - 2.70) < 0.01);
    assert(alum.youngModulusGPa == 68.9);

    Material brass = MaterialEngine::GetMaterialByName("Brass (Yellow)");
    assert(std::abs(brass.density - 8.53) < 0.01);

    Material titanium = MaterialEngine::GetMaterialByName("Titanium (Ti-6Al-4V)");
    assert(std::abs(titanium.density - 4.43) < 0.01);

    std::cout << "  ✓ Verified 7 materials with physical & PBR optical parameters" << std::endl;
}

void TestMassAndVolumeAccuracy() {
    std::cout << "[Test 2/5] Testing Volume, Surface Area and Mass Accuracy (Steel vs Alum)..." << std::endl;
    // Box: 100 x 50 x 20 mm
    // Volume = 100,000 mm³ = 100 cm³
    // Surface Area = 2 * (100*50 + 100*20 + 50*20) = 16,000 mm²
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 50.0, 20.0).Shape();

    Material steel = MaterialEngine::GetMaterial(0);
    PhysicalProperties propsSteel = MaterialEngine::ComputePhysicalProperties(box, steel);
    assert(propsSteel.success);

    assert(std::abs(propsSteel.volumeMm3 - 100000.0) < 1.0);
    assert(std::abs(propsSteel.surfaceAreaMm2 - 16000.0) < 1.0);
    // Steel mass: 100 cm³ * 7.85 g/cm³ = 785.0 g = 0.785 kg
    assert(std::abs(propsSteel.massGrams - 785.0) < 0.1);
    assert(std::abs(propsSteel.massKg - 0.785) < 0.001);

    Material alum = MaterialEngine::GetMaterial(1);
    PhysicalProperties propsAlum = MaterialEngine::ComputePhysicalProperties(box, alum);
    assert(propsAlum.success);
    // Aluminum mass: 100 cm³ * 2.70 g/cm³ = 270.0 g = 0.270 kg
    assert(std::abs(propsAlum.massGrams - 270.0) < 0.1);
    assert(std::abs(propsAlum.massKg - 0.270) < 0.001);

    std::cout << "  ✓ Volume: " << propsSteel.volumeMm3 << " mm³, Area: " << propsSteel.surfaceAreaMm2
              << " mm², Steel Mass: " << propsSteel.massGrams << " g, Alum Mass: " << propsAlum.massGrams << " g" << std::endl;
}

void TestCenterOfGravity() {
    std::cout << "[Test 3/5] Testing Center of Gravity (CoG) 3D Centroid..." << std::endl;
    // Box [0, 100] x [0, 50] x [0, 20]
    // CoG must be at (50.0, 25.0, 10.0)
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 50.0, 20.0).Shape();
    Material mat = MaterialEngine::GetMaterial(0);

    PhysicalProperties props = MaterialEngine::ComputePhysicalProperties(box, mat);
    assert(props.success);

    assert(std::abs(props.cogX - 50.0) < 1e-3);
    assert(std::abs(props.cogY - 25.0) < 1e-3);
    assert(std::abs(props.cogZ - 10.0) < 1e-3);

    std::cout << "  ✓ CoG Centroid accurately located at (" << props.cogX << ", "
              << props.cogY << ", " << props.cogZ << ") mm" << std::endl;
}

void TestMomentsOfInertia() {
    std::cout << "[Test 4/5] Testing Mass Moments of Inertia Matrix..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100.0, 50.0, 20.0).Shape();
    Material steel = MaterialEngine::GetMaterial(0);

    PhysicalProperties props = MaterialEngine::ComputePhysicalProperties(box, steel);
    assert(props.success);

    assert(props.ixx > 0.0);
    assert(props.iyy > 0.0);
    assert(props.izz > 0.0);

    // Diagonal elements dominate
    assert(props.izz > props.iyy);
    assert(props.iyy > props.ixx);

    std::cout << "  ✓ Inertia (kg·mm²): Ixx=" << props.ixx << ", Iyy=" << props.iyy
              << ", Izz=" << props.izz << std::endl;
}

void TestInspectionReportFormat() {
    std::cout << "[Test 5/5] Testing Formatted Physical Properties Inspection Report..." << std::endl;
    TopoDS_Shape box = BRepPrimAPI_MakeBox(60.0, 40.0, 30.0).Shape();
    Material titanium = MaterialEngine::GetMaterial(3);

    PhysicalProperties props = MaterialEngine::ComputePhysicalProperties(box, titanium);
    assert(props.success);

    std::string report = MaterialEngine::FormatInspectionReport(props);
    assert(!report.empty());
    assert(report.find("PHYSICAL PROPERTIES INSPECTION") != std::string::npos);
    assert(report.find("Titanium") != std::string::npos);
    assert(report.find("Volume:") != std::string::npos);
    assert(report.find("Mass:") != std::string::npos);
    assert(report.find("Center of Mass") != std::string::npos);
    assert(report.find("Moments of Inertia") != std::string::npos);

    std::cout << "  ✓ Report generated successfully:\n" << report << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running AndroCAD Materials & Mass Properties Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    TestMaterialLibrary();
    TestMassAndVolumeAccuracy();
    TestCenterOfGravity();
    TestMomentsOfInertia();
    TestInspectionReportFormat();

    std::cout << "========================================" << std::endl;
    std::cout << "All 5 Materials & Mass Properties tests PASSED!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
