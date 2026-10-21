// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_FASTENER_ENGINE_H
#define ANDROCAD_FASTENER_ENGINE_H

#include "AndroCadKernel.h"
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pln.hxx>

#include <vector>
#include <string>

namespace androcad {

enum class HoleType {
    SIMPLE = 0,
    COUNTERBORE = 1,
    COUNTERSINK = 2,
    TAPPED = 3
};

struct FastenerStandard {
    std::string threadName; // "M3", "M4", "M5", "M6", "M8", "M10", "M12"
    double nominalDiameterMm;
    double pitchMm;
    double clearanceHoleDiaMm;
    double counterboreDiaMm;
    double counterboreDepthMm;
    double countersinkDiaMm;
    double countersinkAngleDeg; // Typically 90.0 deg
    double tapDrillDiaMm;
};

struct HoleResult {
    bool success{false};
    TopoDS_Shape shape;
    double volumeMm3{0.0};
    std::string errorMessage;
};

struct ThreadResult {
    bool success{false};
    TopoDS_Shape shape;
    int toothCount{0};
    double pitchMm{0.0};
    double outerDiameterMm{0.0};
    double volumeMm3{0.0};
    std::string errorMessage;
};

struct ProjectedSegment {
    double startU{0.0}, startV{0.0};
    double endU{0.0}, endV{0.0};
    bool isArc{false};
    double centerU{0.0}, centerV{0.0};
    double radius{0.0};
};

struct ProjectionResult {
    bool success{false};
    std::vector<ProjectedSegment> segments;
    int edgeCount{0};
    std::string errorMessage;
};

struct SplinePoint2D {
    double u{0.0}, v{0.0};
};

struct BSplineResult {
    bool success{false};
    int degree{3};
    int knotCount{0};
    int poleCount{0};
    double lengthMm{0.0};
    std::vector<SplinePoint2D> sampledPoints;
    std::string errorMessage;
};

class FastenerEngine {
public:
    FastenerEngine() = default;
    ~FastenerEngine() = default;

    // ISO 273 / ISO 262 standard fastener lookup
    static FastenerStandard GetStandard(const std::string& name);
    static std::vector<std::string> GetAvailableStandards();

    // 1. Hole Wizard: Counterbore, Countersink, Clearance Hole Cut
    static HoleResult CreateStandardHole(
        const TopoDS_Shape& solid,
        const gp_Pnt& position,
        const gp_Dir& direction,
        const std::string& standardName,
        HoleType type,
        double depthMm
    );

    // 2. Modeled 3D Metric Helical Threaded Bolt / Stud
    static ThreadResult CreateThreadedBolt(
        const std::string& standardName,
        double shaftLengthMm,
        double threadLengthMm,
        bool hexHead = true
    );

    // 3. Associative 3D Edge Projection onto 2D Sketch Plane
    static ProjectionResult ProjectEdgesToPlane(
        const TopoDS_Shape& solid,
        const gp_Pln& sketchPlane,
        const std::vector<int>& targetEdgeIds = {}
    );

    // 4. Cubic B-Spline Curve Generator with C2 continuity
    static BSplineResult GenerateCubicBSpline(
        const std::vector<SplinePoint2D>& controlPoints,
        int sampleCount = 50
    );
};

} // namespace androcad

#endif // ANDROCAD_FASTENER_ENGINE_H
