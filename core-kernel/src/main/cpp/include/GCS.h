// Copyright (c) 2026 dursuntokgoz. All rights reserved.
// Portions Copyright (c) FreeCAD contributors.
// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef ANDROCAD_GCS_H
#define ANDROCAD_GCS_H

#include <vector>
#include <string>
#include <memory>
#include <cmath>

namespace androcad {
namespace gcs {

enum class SolveStatus {
    Success,
    Converged,
    UnderConstrained,
    FullyConstrained,
    OverConstrained,
    Failed
};

enum class ConstraintType {
    Coincident,       // P1 == P2 (2 equations: dx=0, dy=0)
    Horizontal,       // Line(P1, P2) dy = 0
    Vertical,         // Line(P1, P2) dx = 0
    Distance,         // Distance(P1, P2) == value
    DistanceX,        // Horizontal distance |P2.x - P1.x| == value
    DistanceY,        // Vertical distance |P2.y - P1.y| == value
    Parallel,         // Line(P1, P2) parallel to Line(P3, P4)
    Perpendicular,    // Line(P1, P2) perpendicular to Line(P3, P4)
    EqualLength,      // Length(P1, P2) == Length(P3, P4)
    PointOnLine,      // Point P1 lies on Line(P2, P3)
    CircleRadius,     // Distance(Center P1, OnCircle P2) == value
    TangentLineCircle // Line(P1, P2) tangent to Circle(Center P3, radius value)
};

struct Point2D {
    int id;
    double x;
    double y;
    bool isFixed;
};

struct Constraint {
    ConstraintType type;
    int p1; // Point ID
    int p2; // Point ID
    int p3; // Point ID (or -1 if unused)
    int p4; // Point ID (or -1 if unused)
    double value;
};

class System {
public:
    System() : dof(0), maxResidual(0.0) {}
    ~System() = default;

    int AddPoint(double x, double y, bool isFixed = false);
    void SetPoint(int id, double x, double y);
    void SetPointFixed(int id, bool isFixed);
    Point2D GetPoint(int id) const;
    const std::vector<Point2D>& GetPoints() const { return points; }

    int AddConstraint(ConstraintType type, int p1, int p2, int p3 = -1, int p4 = -1, double value = 0.0);
    const std::vector<Constraint>& GetConstraints() const { return constraints; }

    void Clear();

    // Solves the constraint equations using Levenberg-Marquardt with rank-based DoF calculation
    SolveStatus Solve(int maxIterations = 50, double convergenceTol = 1e-6);

    int GetDegreesOfFreedom() const { return dof; }
    double GetMaxResidual() const { return maxResidual; }

private:
    std::vector<Point2D> points;
    std::vector<Constraint> constraints;
    int dof;
    double maxResidual;

    // Evaluates constraint residuals
    std::vector<double> ComputeResiduals(const std::vector<double>& xParams) const;

    // Computes numerical Jacobian matrix
    std::vector<std::vector<double>> ComputeJacobian(const std::vector<double>& xParams, const std::vector<double>& r0) const;

    // Cholesky solver for (J^T J + lambda I) delta = -J^T r
    bool SolveCholesky(const std::vector<std::vector<double>>& A, const std::vector<double>& b, std::vector<double>& outDelta) const;

    // Numerical rank estimation of Jacobian
    int EstimateRank(const std::vector<std::vector<double>>& J) const;
};

} // namespace gcs
} // namespace androcad

#endif // ANDROCAD_GCS_H
