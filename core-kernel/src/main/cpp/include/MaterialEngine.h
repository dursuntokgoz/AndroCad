// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_MATERIAL_ENGINE_H
#define ANDROCAD_MATERIAL_ENGINE_H

#include "AndroCadKernel.h"
#include <TopoDS_Shape.hxx>

#include <string>
#include <vector>

namespace androcad {

struct PBRAppearance {
    float r{0.8f};
    float g{0.8f};
    float b{0.8f};
    float metallic{0.5f};
    float roughness{0.3f};
    std::string hexColor{"#CCCCCC"};
};

struct Material {
    int id{0};
    std::string name{"Steel (Structural)"};
    std::string category{"Metal"};
    double density{7.85};         // Density in g/cm³
    double youngModulusGPa{210.0}; // Elastic Modulus in GPa
    double poissonRatio{0.30};    // Poisson's ratio
    double yieldStrengthMPa{250.0};// Yield Strength in MPa
    PBRAppearance appearance;
};

struct PhysicalProperties {
    bool success{false};
    std::string materialName;
    double densityGcm3{0.0};
    double volumeMm3{0.0};
    double surfaceAreaMm2{0.0};
    double massGrams{0.0};
    double massKg{0.0};
    double cogX{0.0}; // Center of Gravity X (mm)
    double cogY{0.0}; // Center of Gravity Y (mm)
    double cogZ{0.0}; // Center of Gravity Z (mm)
    // Mass Moments of Inertia (kg * mm²)
    double ixx{0.0};
    double iyy{0.0};
    double izz{0.0};
    double ixy{0.0};
    double ixz{0.0};
    double iyz{0.0};
    std::string errorMessage;
};

class MaterialEngine {
public:
    // 1. Returns standard engineering materials library
    static std::vector<Material> GetDefaultMaterials();

    // 2. Looks up material by ID or Name
    static Material GetMaterial(int materialId);
    static Material GetMaterialByName(const std::string& name);

    // 3. Computes exact physical mass properties using OCCT BRepGProp
    static PhysicalProperties ComputePhysicalProperties(const TopoDS_Shape& shape,
                                                        const Material& material);

    // 4. Formats physical properties into a professional inspection report string
    static std::string FormatInspectionReport(const PhysicalProperties& props);
};

} // namespace androcad

#endif // ANDROCAD_MATERIAL_ENGINE_H