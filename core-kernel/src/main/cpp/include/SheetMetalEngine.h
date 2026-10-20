// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_SHEET_METAL_ENGINE_H
#define ANDROCAD_SHEET_METAL_ENGINE_H

#include "AndroCadKernel.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <vector>
#include <string>

namespace androcad {

struct SheetMetalRule {
    double thickness{2.0};    // Material sheet thickness (mm)
    double bendRadius{2.0};   // Inner bend radius (mm)
    double kFactor{0.44};      // Neutral axis location factor (typical 0.40 - 0.45 for steel/alum)

    // Analytical bend calculations
    double CalculateBendAllowance(double angleRad) const;
    double CalculateBendDeduction(double angleRad) const;
    double CalculateFlatLength(double leg1, double leg2, double angleRad) const;
};

struct BendLine {
    int id;
    double startX;
    double startY;
    double endX;
    double endY;
    double angleDeg;
    double radius;
    bool isBendUp;
};

struct FlatPattern {
    bool success{false};
    double flatLength{0.0};
    double flatWidth{0.0};
    double flatArea{0.0};
    std::vector<BendLine> bends;
    TopoDS_Shape flatSheetShape;
    std::string dxfContent;
    std::string errorMessage;
};

class SheetMetalEngine {
public:
    // 1. Creates a planar base sheet plate (width x length x thickness)
    static TopoDS_Shape CreateBasePlate(double width,
                                        double length,
                                        const SheetMetalRule& rule);

    // 2. Adds a 90-degree flange with realistic inner cylindrical fillet bend
    static TopoDS_Shape AddFlange(const TopoDS_Shape& basePlate,
                                  double flangeHeight,
                                  double angleRad,
                                  const SheetMetalRule& rule);

    // 3. Creates a standard 3D U-Bracket sheet metal part (Base + 2 side flanges + 2 bends)
    static TopoDS_Shape CreateUBracket(double baseLength,
                                       double flangeHeight,
                                       double width,
                                       const SheetMetalRule& rule);

    // 4. Computes 2D unwrapped flat pattern layout with bend deduction and bend lines
    static FlatPattern UnfoldUBracket(double baseLength,
                                      double flangeHeight,
                                      double width,
                                      const SheetMetalRule& rule);

    // 5. Generates standard 2D DXF text file for laser and CNC waterjet manufacturing
    static std::string GenerateFlatPatternDXF(const FlatPattern& pattern);

    // 6. Converts sheet metal shape to GPU-ready mesh payload
    static MeshPayload ToMeshPayload(const TopoDS_Shape& shape,
                                     double linearDeflection = 0.1,
                                     double angularDeflection = 0.5);
};

} // namespace androcad

#endif // ANDROCAD_SHEET_METAL_ENGINE_H
