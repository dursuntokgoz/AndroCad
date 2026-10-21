// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_PATTERN_ENGINE_H
#define ANDROCAD_PATTERN_ENGINE_H

#include "AndroCadKernel.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <gp_Pln.hxx>

#include <vector>
#include <string>

namespace androcad {

struct PatternResult {
    bool success{false};
    TopoDS_Shape shape;
    int instanceCount{0};
    double totalVolumeMm3{0.0};
    std::string errorMessage;
};

class PatternEngine {
public:
    PatternEngine() = default;
    ~PatternEngine() = default;

    // 1. Rectangular Pattern (2D Linear Grid)
    static PatternResult RectangularPattern(
        const TopoDS_Shape& baseShape,
        int countU, double spacingU, const gp_Vec& dirU,
        int countV = 1, double spacingV = 0.0, const gp_Vec& dirV = gp_Vec(0, 1, 0),
        bool fuseInstances = false
    );

    // 2. Circular Pattern (Polar Rotation Array)
    static PatternResult CircularPattern(
        const TopoDS_Shape& baseShape,
        const gp_Ax1& axis,
        int count,
        double totalAngleDeg = 360.0,
        bool fuseInstances = false
    );

    // 3. Mirror Feature & Body
    static PatternResult Mirror(
        const TopoDS_Shape& baseShape,
        const gp_Ax2& mirrorPlane,
        bool keepOriginal = true,
        bool fuseInstances = false
    );

    // 4. Draft Angle (Taper on faces for injection molding)
    static PatternResult ApplyDraft(
        const TopoDS_Shape& baseShape,
        const gp_Dir& pullDirection,
        double draftAngleDeg,
        const std::vector<int>& targetFaceIds
    );

    // Helper: transform a shape
    static TopoDS_Shape TransformShape(const TopoDS_Shape& shape, const gp_Trsf& trsf);
};

} // namespace androcad

#endif // ANDROCAD_PATTERN_ENGINE_H
