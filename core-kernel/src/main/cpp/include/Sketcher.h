// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_SKETCHER_H
#define ANDROCAD_SKETCHER_H

#include "GCS.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

#include <vector>
#include <string>
#include <memory>

namespace androcad {

struct SketchPlane {
    double ox = 0.0, oy = 0.0, oz = 0.0;
    double ux = 1.0, uy = 0.0, uz = 0.0;
    double vx = 0.0, vy = 1.0, vz = 0.0;
    double nx = 0.0, ny = 0.0, nz = 1.0;

    static SketchPlane XY() {
        return SketchPlane{0,0,0, 1,0,0, 0,1,0, 0,0,1};
    }
    static SketchPlane XZ() {
        return SketchPlane{0,0,0, 1,0,0, 0,0,1, 0,-1,0};
    }
    static SketchPlane YZ() {
        return SketchPlane{0,0,0, 0,1,0, 0,0,1, 1,0,0};
    }
};

enum class CurveType {
    Line,
    Circle,
    Arc
};

struct SketchCurve {
    int id;
    CurveType type;
    int p1; // Start point or Center point for circle
    int p2; // End point or On-Circle point
    int p3; // Center point (for Arc)
    double radius;
};

struct SketchResult {
    bool success = false;
    bool isClosedProfile = false;
    int degreesOfFreedom = 0;
    gcs::SolveStatus solveStatus = gcs::SolveStatus::UnderConstrained;
    double profileArea = 0.0;
    int pointCount = 0;
    int curveCount = 0;
    std::string errorMessage;
    TopoDS_Face face;
    TopoDS_Wire wire;
};

class Sketcher {
public:
    Sketcher(const SketchPlane& plane = SketchPlane::XY());
    ~Sketcher() = default;

    void SetPlane(const SketchPlane& plane) { this->plane = plane; }
    const SketchPlane& GetPlane() const { return plane; }

    // 1. Primitive creation
    int AddPoint(double u, double v, bool isFixed = false);
    int AddLine(int pStart, int pEnd);
    int AddCircle(int pCenter, double radius);
    int AddRectangle(double u, double v, double width, double height);

    // 2. Constraints (delegates to GCS)
    int AddCoincident(int p1, int p2);
    int AddHorizontal(int p1, int p2);
    int AddVertical(int p1, int p2);
    int AddDistance(int p1, int p2, double distance);
    int AddParallel(int l1_p1, int l1_p2, int l2_p1, int l2_p2);
    int AddPerpendicular(int l1_p1, int l1_p2, int l2_p1, int l2_p2);
    int AddEqualLength(int l1_p1, int l1_p2, int l2_p1, int l2_p2);

    // 3. Solve & Evaluation
    gcs::SolveStatus Solve();
    int GetDegreesOfFreedom() const { return solver.GetDegreesOfFreedom(); }

    // 4. Closed profile loop detection and OCCT Face construction
    SketchResult BuildProfileFace();

    gcs::Point2D GetPoint(int id) const { return solver.GetPoint(id); }
    const std::vector<SketchCurve>& GetCurves() const { return curves; }
    void Clear();

private:
    SketchPlane plane;
    gcs::System solver;
    std::vector<SketchCurve> curves;

    gp_Pnt ToWorldPoint(double u, double v) const;
};

} // namespace androcad

#endif // ANDROCAD_SKETCHER_H
