// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "MaterialEngine.h"

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <gp_Pnt.hxx>
#include <gp_Mat.hxx>
#include <Standard_Failure.hxx>

#include <sstream>
#include <iomanip>
#include <algorithm>

namespace androcad {

std::vector<Material> MaterialEngine::GetDefaultMaterials() {
    std::vector<Material> list;

    // 0. Steel (Structural S235 / AISI 1018)
    Material steel;
    steel.id = 0;
    steel.name = "Steel (Structural)";
    steel.category = "Metal";
    steel.density = 7.85; // g/cm³
    steel.youngModulusGPa = 210.0;
    steel.poissonRatio = 0.30;
    steel.yieldStrengthMPa = 250.0;
    steel.appearance = {0.62f, 0.65f, 0.68f, 0.85f, 0.35f, "#9EA5AD"};
    list.push_back(steel);

    // 1. Aluminum 6061-T6
    Material alum;
    alum.id = 1;
    alum.name = "Aluminum 6061-T6";
    alum.category = "Metal";
    alum.density = 2.70;
    alum.youngModulusGPa = 68.9;
    alum.poissonRatio = 0.33;
    alum.yieldStrengthMPa = 276.0;
    alum.appearance = {0.84f, 0.86f, 0.90f, 0.90f, 0.25f, "#D6DCE5"};
    list.push_back(alum);

    // 2. Brass (Cartridge C26000)
    Material brass;
    brass.id = 2;
    brass.name = "Brass (Yellow)";
    brass.category = "Metal";
    brass.density = 8.53;
    brass.youngModulusGPa = 110.0;
    brass.poissonRatio = 0.35;
    brass.yieldStrengthMPa = 345.0;
    brass.appearance = {0.90f, 0.72f, 0.29f, 0.88f, 0.30f, "#E5B84A"};
    list.push_back(brass);

    // 3. Titanium Ti-6Al-4V (Grade 5)
    Material titanium;
    titanium.id = 3;
    titanium.name = "Titanium (Ti-6Al-4V)";
    titanium.category = "Metal";
    titanium.density = 4.43;
    titanium.youngModulusGPa = 114.0;
    titanium.poissonRatio = 0.34;
    titanium.yieldStrengthMPa = 880.0;
    titanium.appearance = {0.60f, 0.62f, 0.62f, 0.80f, 0.40f, "#9A9E9F"};
    list.push_back(titanium);

    // 4. ABS Polymer (Engineering Plastic)
    Material abs;
    abs.id = 4;
    abs.name = "ABS Plastic";
    abs.category = "Plastic";
    abs.density = 1.04;
    abs.youngModulusGPa = 2.3;
    abs.poissonRatio = 0.38;
    abs.yieldStrengthMPa = 40.0;
    abs.appearance = {0.88f, 0.88f, 0.88f, 0.05f, 0.50f, "#E0E0E0"};
    list.push_back(abs);

    // 5. Polycarbonate (Clear)
    Material pc;
    pc.id = 5;
    pc.name = "Polycarbonate (Clear)";
    pc.category = "Plastic";
    pc.density = 1.20;
    pc.youngModulusGPa = 2.4;
    pc.poissonRatio = 0.37;
    pc.yieldStrengthMPa = 65.0;
    pc.appearance = {0.78f, 0.90f, 0.98f, 0.02f, 0.15f, "#C8E6FA"};
    list.push_back(pc);

    // 6. Oak Wood (Hardwood)
    Material oak;
    oak.id = 6;
    oak.name = "Oak Wood";
    oak.category = "Wood";
    oak.density = 0.75;
    oak.youngModulusGPa = 12.0;
    oak.poissonRatio = 0.30;
    oak.yieldStrengthMPa = 45.0;
    oak.appearance = {0.63f, 0.44f, 0.25f, 0.00f, 0.80f, "#A07040"};
    list.push_back(oak);

    return list;
}

Material MaterialEngine::GetMaterial(int materialId) {
    auto list = GetDefaultMaterials();
    for (const auto& mat : list) {
        if (mat.id == materialId) return mat;
    }
    return list[0]; // Fallback to Steel
}

Material MaterialEngine::GetMaterialByName(const std::string& name) {
    auto list = GetDefaultMaterials();
    for (const auto& mat : list) {
        if (mat.name == name) return mat;
    }
    return list[0];
}

PhysicalProperties MaterialEngine::ComputePhysicalProperties(const TopoDS_Shape& shape,
                                                            const Material& material) {
    PhysicalProperties props;
    props.materialName = material.name;
    props.densityGcm3 = material.density;

    if (shape.IsNull()) {
        props.success = false;
        props.errorMessage = "Input shape is null";
        return props;
    }

    try {
        // 1. Volume & Center of Mass & Volumetric Inertia
        GProp_GProps volProps;
        BRepGProp::VolumeProperties(shape, volProps);

        double volMm3 = volProps.Mass();
        if (volMm3 <= 0.0) {
            props.success = false;
            props.errorMessage = "Shape has non-positive volume";
            return props;
        }

        props.volumeMm3 = volMm3;

        gp_Pnt cog = volProps.CentreOfMass();
        props.cogX = cog.X();
        props.cogY = cog.Y();
        props.cogZ = cog.Z();

        // Mass: 1 mm³ = 1e-3 cm³. Mass (g) = vol (mm³) * density (g/cm³) * 1e-3
        props.massGrams = volMm3 * material.density * 1e-3;
        props.massKg = props.massGrams * 1e-3;

        // Mass Moments of Inertia:
        // volProps matrix has unit mm⁵.
        // Multiply by (density in g/cm³ * 1e-6 kg/mm³) gives kg * mm²
        double inertiaFactor = material.density * 1e-6;
        gp_Mat I = volProps.MatrixOfInertia();
        props.ixx = I(1, 1) * inertiaFactor;
        props.iyy = I(2, 2) * inertiaFactor;
        props.izz = I(3, 3) * inertiaFactor;
        props.ixy = -I(1, 2) * inertiaFactor;
        props.ixz = -I(1, 3) * inertiaFactor;
        props.iyz = -I(2, 3) * inertiaFactor;

        // 2. Surface Area
        GProp_GProps surfProps;
        BRepGProp::SurfaceProperties(shape, surfProps);
        props.surfaceAreaMm2 = surfProps.Mass();

        props.success = true;
    } catch (Standard_Failure& e) {
        props.success = false;
        props.errorMessage = std::string("OCCT mass property failure: ") + e.GetMessageString();
    } catch (const std::exception& e) {
        props.success = false;
        props.errorMessage = e.what();
    }

    return props;
}

std::string MaterialEngine::FormatInspectionReport(const PhysicalProperties& props) {
    if (!props.success) {
        return "Inspection Failed: " + props.errorMessage;
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "=== PHYSICAL PROPERTIES INSPECTION ===\n";
    ss << "Material: " << props.materialName << " (" << props.densityGcm3 << " g/cm³)\n";
    ss << "------------------------------------\n";
    ss << "Volume:       " << props.volumeMm3 << " mm³ (" << (props.volumeMm3 * 1e-3) << " cm³)\n";
    ss << "Surface Area: " << props.surfaceAreaMm2 << " mm² (" << (props.surfaceAreaMm2 * 1e-2) << " cm²)\n";
    ss << "Mass:         " << props.massGrams << " g (" << props.massKg << " kg)\n";
    ss << "------------------------------------\n";
    ss << "Center of Mass (CoG):\n";
    ss << "  X: " << props.cogX << " mm\n";
    ss << "  Y: " << props.cogY << " mm\n";
    ss << "  Z: " << props.cogZ << " mm\n";
    ss << "------------------------------------\n";
    ss << "Moments of Inertia (kg·mm²):\n";
    ss << "  Ixx: " << props.ixx << "   Iyy: " << props.iyy << "   Izz: " << props.izz << "\n";
    ss << "  Ixy: " << props.ixy << "   Ixz: " << props.ixz << "   Iyz: " << props.iyz << "\n";
    ss << "====================================\n";
    return ss.str();
}

} // namespace androcad