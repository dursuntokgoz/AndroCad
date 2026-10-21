// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_SIMULATION_ENGINE_H
#define ANDROCAD_SIMULATION_ENGINE_H

#include "AndroCadKernel.h"
#include "MaterialEngine.h"

#include <TopoDS_Shape.hxx>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace androcad {

enum class ConstraintType {
    FIXED,       // All 3 DOFs locked (Ux = Uy = Uz = 0)
    PIN,         // Translation locked, rotation allowed
    ROLLER_X,    // Ux locked, Uy/Uz free
    ROLLER_Y,    // Uy locked, Ux/Uz free
    ROLLER_Z     // Uz locked, Ux/Uy free
};

struct BoundaryConstraint {
    ConstraintType type{ConstraintType::FIXED};
    int faceId{-1};          // Target face index, or -1 for bounding volume
    double minX{-1e9}, maxX{1e9};
    double minY{-1e9}, maxY{1e9};
    double minZ{-1e9}, maxZ{1e9};
};

enum class LoadType {
    TOTAL_FORCE,     // Total force in Newtons distributed over selected nodes
    PRESSURE_NORMAL  // Pressure in Pascals (N/m²) normal to surface
};

struct StructuralLoad {
    LoadType type{LoadType::TOTAL_FORCE};
    int faceId{-1};
    double forceX{0.0};      // Force X in Newtons
    double forceY{0.0};      // Force Y in Newtons
    double forceZ{0.0};      // Force Z in Newtons
    double pressurePa{0.0};  // Pressure in Pa
    double minX{-1e9}, maxX{1e9};
    double minY{-1e9}, maxY{1e9};
    double minZ{-1e9}, maxZ{1e9};
};

struct FEANode {
    uint32_t id{0};
    double x{0.0}, y{0.0}, z{0.0};             // Undisplaced position in meters
    double uX{0.0}, uY{0.0}, uZ{0.0};          // Displacements in meters
    double vonMisesStressPa{0.0};             // Von Mises stress at node in Pascals
    bool isFixedX{false};
    bool isFixedY{false};
    bool isFixedZ{false};
};

struct FEATet4 {
    uint32_t id{0};
    uint32_t nodes[4]{0, 0, 0, 0};             // 4 node indices
    double volumeM3{0.0};                      // Element volume in m³
    double vonMisesStressPa{0.0};             // Element Von Mises stress in Pascals
    double sigmaXX{0.0}, sigmaYY{0.0}, sigmaZZ{0.0};
    double tauXY{0.0}, tauYZ{0.0}, tauZX{0.0};
};

struct SimulationMeshPayload {
    std::vector<float> positions;     // 3 floats per vertex (X, Y, Z in mm, deformed)
    std::vector<float> colors;        // 3 floats per vertex (R, G, B in [0, 1])
    std::vector<float> stressesMPa;   // 1 float per vertex (Von Mises stress in MPa)
    std::vector<float> displacementsMm; // 1 float per vertex (displacement magnitude in mm)
    std::vector<uint32_t> indices;    // 3 indices per boundary triangle
};

struct SimulationResult {
    bool success{false};
    std::string errorMessage;
    uint32_t nodeCount{0};
    uint32_t elementCount{0};
    double maxVonMisesStressMPa{0.0};
    double minVonMisesStressMPa{0.0};
    double maxDisplacementMm{0.0};
    double minSafetyFactor{0.0};
    SimulationMeshPayload visualMesh;
};

class SimulationEngine {
public:
    SimulationEngine();
    ~SimulationEngine() = default;

    // Discretization methods
    void GenerateBeamMesh(double lengthMm, double widthMm, double heightMm,
                          int nx = 20, int ny = 4, int nz = 4);

    void DiscretizeShape(const TopoDS_Shape& shape, double targetElementSizeMm = 10.0);

    // Setup boundary conditions and loading
    void AddConstraint(const BoundaryConstraint& constraint);
    void AddLoad(const StructuralLoad& load);
    void SetMaterial(const Material& material);
    void SetMaterialProperties(double youngModulusGPa, double poissonRatio, double yieldStrengthMPa);

    // Solving
    SimulationResult Solve(double deformationScale = 1.0);

    // High-level helper for standard cantilever beam analysis
    static SimulationResult SolveCantileverBeam(double lengthMm, double widthMm, double heightMm,
                                                double pointLoadForceN,
                                                const Material& material,
                                                double deformationScale = 1.0,
                                                int nx = 25, int ny = 5, int nz = 5);

    void Clear();

    const std::vector<FEANode>& GetNodes() const { return nodes_; }
    const std::vector<FEATet4>& GetElements() const { return elements_; }

private:
    std::vector<FEANode> nodes_;
    std::vector<FEATet4> elements_;
    std::vector<BoundaryConstraint> constraints_;
    std::vector<StructuralLoad> loads_;

    double youngModulusPa_{2.0e11};      // Default Steel: 200 GPa = 2.0e11 Pa
    double poissonRatio_{0.30};
    double yieldStrengthPa_{2.5e8};       // Default Steel: 250 MPa = 2.5e8 Pa

    void ApplyBoundaryConditions(std::vector<double>& F,
                                 std::vector<std::vector<std::pair<int, double>>>& K_rows);

    SimulationMeshPayload BuildVisualSurfaceMesh(double deformationScale,
                                                double minStressMPa,
                                                double maxStressMPa);
};

} // namespace androcad

#endif // ANDROCAD_SIMULATION_ENGINE_H
