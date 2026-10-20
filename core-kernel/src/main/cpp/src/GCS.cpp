// Copyright (c) 2026 dursuntokgoz. All rights reserved.
// Portions Copyright (c) FreeCAD contributors.
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "GCS.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace androcad {
namespace gcs {

int System::AddPoint(double x, double y, bool isFixed) {
    int id = static_cast<int>(points.size());
    points.push_back(Point2D{id, x, y, isFixed});
    return id;
}

void System::SetPoint(int id, double x, double y) {
    if (id >= 0 && id < static_cast<int>(points.size())) {
        points[id].x = x;
        points[id].y = y;
    }
}

void System::SetPointFixed(int id, bool isFixed) {
    if (id >= 0 && id < static_cast<int>(points.size())) {
        points[id].isFixed = isFixed;
    }
}

Point2D System::GetPoint(int id) const {
    if (id >= 0 && id < static_cast<int>(points.size())) {
        return points[id];
    }
    return Point2D{-1, 0.0, 0.0, false};
}

int System::AddConstraint(ConstraintType type, int p1, int p2, int p3, int p4, double value) {
    int id = static_cast<int>(constraints.size());
    constraints.push_back(Constraint{type, p1, p2, p3, p4, value});
    return id;
}

void System::Clear() {
    points.clear();
    constraints.clear();
    dof = 0;
    maxResidual = 0.0;
}

std::vector<double> System::ComputeResiduals(const std::vector<double>& xParams) const {
    // Unpack xParams into temporary point array
    std::vector<Point2D> pts = points;
    size_t paramIdx = 0;
    for (auto& pt : pts) {
        if (!pt.isFixed) {
            pt.x = xParams[paramIdx++];
            pt.y = xParams[paramIdx++];
        }
    }

    std::vector<double> residuals;
    for (const auto& c : constraints) {
        if (c.p1 < 0 || c.p1 >= static_cast<int>(pts.size())) continue;
        const Point2D& p1 = pts[c.p1];

        switch (c.type) {
            case ConstraintType::Coincident: {
                if (c.p2 >= 0 && c.p2 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    residuals.push_back(p1.x - p2.x);
                    residuals.push_back(p1.y - p2.y);
                }
                break;
            }
            case ConstraintType::Horizontal: {
                if (c.p2 >= 0 && c.p2 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    residuals.push_back(p2.y - p1.y);
                }
                break;
            }
            case ConstraintType::Vertical: {
                if (c.p2 >= 0 && c.p2 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    residuals.push_back(p2.x - p1.x);
                }
                break;
            }
            case ConstraintType::Distance: {
                if (c.p2 >= 0 && c.p2 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    double dx = p2.x - p1.x;
                    double dy = p2.y - p1.y;
                    double currentDist = std::sqrt(dx * dx + dy * dy);
                    residuals.push_back(currentDist - c.value);
                }
                break;
            }
            case ConstraintType::DistanceX: {
                if (c.p2 >= 0 && c.p2 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    residuals.push_back(std::abs(p2.x - p1.x) - c.value);
                }
                break;
            }
            case ConstraintType::DistanceY: {
                if (c.p2 >= 0 && c.p2 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    residuals.push_back(std::abs(p2.y - p1.y) - c.value);
                }
                break;
            }
            case ConstraintType::Parallel: {
                if (c.p2 >= 0 && c.p3 >= 0 && c.p4 >= 0 &&
                    c.p4 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    const Point2D& p3 = pts[c.p3];
                    const Point2D& p4 = pts[c.p4];
                    double dx1 = p2.x - p1.x, dy1 = p2.y - p1.y;
                    double dx2 = p4.x - p3.x, dy2 = p4.y - p3.y;
                    // Cross product: dx1*dy2 - dy1*dx2 == 0
                    residuals.push_back(dx1 * dy2 - dy1 * dx2);
                }
                break;
            }
            case ConstraintType::Perpendicular: {
                if (c.p2 >= 0 && c.p3 >= 0 && c.p4 >= 0 &&
                    c.p4 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    const Point2D& p3 = pts[c.p3];
                    const Point2D& p4 = pts[c.p4];
                    double dx1 = p2.x - p1.x, dy1 = p2.y - p1.y;
                    double dx2 = p4.x - p3.x, dy2 = p4.y - p3.y;
                    // Dot product: dx1*dx2 + dy1*dy2 == 0
                    residuals.push_back(dx1 * dx2 + dy1 * dy2);
                }
                break;
            }
            case ConstraintType::EqualLength: {
                if (c.p2 >= 0 && c.p3 >= 0 && c.p4 >= 0 &&
                    c.p4 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    const Point2D& p3 = pts[c.p3];
                    const Point2D& p4 = pts[c.p4];
                    double l1Sq = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
                    double l2Sq = (p4.x - p3.x) * (p4.x - p3.x) + (p4.y - p3.y) * (p4.y - p3.y);
                    residuals.push_back(std::sqrt(l1Sq) - std::sqrt(l2Sq));
                }
                break;
            }
            case ConstraintType::PointOnLine: {
                if (c.p2 >= 0 && c.p3 >= 0 && c.p3 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    const Point2D& p3 = pts[c.p3];
                    double dx = p3.x - p2.x, dy = p3.y - p2.y;
                    double len = std::sqrt(dx * dx + dy * dy);
                    if (len > 1e-9) {
                        residuals.push_back(((p1.x - p2.x) * dy - (p1.y - p2.y) * dx) / len);
                    }
                }
                break;
            }
            case ConstraintType::CircleRadius: {
                if (c.p2 >= 0 && c.p2 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    double dx = p2.x - p1.x, dy = p2.y - p1.y;
                    residuals.push_back(std::sqrt(dx * dx + dy * dy) - c.value);
                }
                break;
            }
            case ConstraintType::TangentLineCircle: {
                if (c.p2 >= 0 && c.p3 >= 0 && c.p3 < static_cast<int>(pts.size())) {
                    const Point2D& p2 = pts[c.p2];
                    const Point2D& center = pts[c.p3];
                    double a = -(p2.y - p1.y);
                    double b = p2.x - p1.x;
                    double cConst = -(a * p1.x + b * p1.y);
                    double norm = std::sqrt(a * a + b * b);
                    if (norm > 1e-9) {
                        double dist = std::abs(a * center.x + b * center.y + cConst) / norm;
                        residuals.push_back(dist - c.value);
                    }
                }
                break;
            }
        }
    }

    return residuals;
}

std::vector<std::vector<double>> System::ComputeJacobian(const std::vector<double>& xParams, const std::vector<double>& r0) const {
    size_t numParams = xParams.size();
    size_t numResiduals = r0.size();
    std::vector<std::vector<double>> J(numResiduals, std::vector<double>(numParams, 0.0));

    const double h = 1e-7;
    std::vector<double> xPerturbed = xParams;

    for (size_t j = 0; j < numParams; ++j) {
        xPerturbed[j] += h;
        std::vector<double> rPerturbed = ComputeResiduals(xPerturbed);
        xPerturbed[j] = xParams[j];

        for (size_t i = 0; i < numResiduals; ++i) {
            J[i][j] = (rPerturbed[i] - r0[i]) / h;
        }
    }

    return J;
}

bool System::SolveCholesky(const std::vector<std::vector<double>>& A, const std::vector<double>& b, std::vector<double>& outDelta) const {
    size_t n = A.size();
    outDelta.assign(n, 0.0);
    std::vector<std::vector<double>> L(n, std::vector<double>(n, 0.0));

    // Cholesky-Banachiewicz: A = L L^T
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j <= i; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < j; ++k) {
                sum += L[i][k] * L[j][k];
            }
            if (i == j) {
                double val = A[i][i] - sum;
                if (val <= 1e-12) {
                    val = 1e-12; // Numerical jitter regularizer
                }
                L[i][j] = std::sqrt(val);
            } else {
                L[i][j] = (A[i][j] - sum) / L[j][j];
            }
        }
    }

    // Forward substitution: L y = b
    std::vector<double> y(n, 0.0);
    for (size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (size_t k = 0; k < i; ++k) {
            sum += L[i][k] * y[k];
        }
        y[i] = (b[i] - sum) / L[i][i];
    }

    // Back substitution: L^T delta = y
    for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
        double sum = 0.0;
        for (size_t k = static_cast<size_t>(i + 1); k < n; ++k) {
            sum += L[k][i] * outDelta[k];
        }
        outDelta[i] = (y[i] - sum) / L[i][i];
    }

    return true;
}

int System::EstimateRank(const std::vector<std::vector<double>>& J) const {
    size_t m = J.size();
    if (m == 0) return 0;
    size_t n = J[0].size();
    if (n == 0) return 0;

    // Build J^T J
    std::vector<std::vector<double>> JTJ(n, std::vector<double>(n, 0.0));
    for (size_t j1 = 0; j1 < n; ++j1) {
        for (size_t j2 = j1; j2 < n; ++j2) {
            double sum = 0.0;
            for (size_t i = 0; i < m; ++i) {
                sum += J[i][j1] * J[i][j2];
            }
            JTJ[j1][j2] = sum;
            JTJ[j2][j1] = sum;
        }
    }

    // Jacobi eigenvalue algorithm to find rank
    std::vector<double> eigenvalues(n);
    for (size_t i = 0; i < n; ++i) eigenvalues[i] = JTJ[i][i];

    // Diagonalize via Jacobi rotations
    for (int sweep = 0; sweep < 30; ++sweep) {
        double offDiag = 0.0;
        for (size_t p = 0; p < n; ++p) {
            for (size_t q = p + 1; q < n; ++q) {
                offDiag += std::abs(JTJ[p][q]);
            }
        }
        if (offDiag < 1e-8) break;

        for (size_t p = 0; p < n; ++p) {
            for (size_t q = p + 1; q < n; ++q) {
                if (std::abs(JTJ[p][q]) < 1e-12) continue;
                double h = eigenvalues[q] - eigenvalues[p];
                double t;
                if (std::abs(h) < 1e-12) {
                    t = 1.0;
                } else {
                    double theta = 0.5 * h / JTJ[p][q];
                    t = 1.0 / (std::abs(theta) + std::sqrt(theta * theta + 1.0));
                    if (theta < 0.0) t = -t;
                }
                double c = 1.0 / std::sqrt(t * t + 1.0);
                double s = t * c;
                double tau = s / (1.0 + c);
                double delta = t * JTJ[p][q];
                eigenvalues[p] -= delta;
                eigenvalues[q] += delta;
                JTJ[p][q] = 0.0;

                for (size_t j = 0; j < p; ++j) {
                    double g = JTJ[j][p], hVal = JTJ[j][q];
                    JTJ[j][p] = g - s * (hVal + g * tau);
                    JTJ[j][q] = hVal + s * (g - hVal * tau);
                }
                for (size_t j = p + 1; j < q; ++j) {
                    double g = JTJ[p][j], hVal = JTJ[j][q];
                    JTJ[p][j] = g - s * (hVal + g * tau);
                    JTJ[j][q] = hVal + s * (g - hVal * tau);
                }
                for (size_t j = q + 1; j < n; ++j) {
                    double g = JTJ[p][j], hVal = JTJ[q][j];
                    JTJ[p][j] = g - s * (hVal + g * tau);
                    JTJ[q][j] = hVal + s * (g - hVal * tau);
                }
            }
        }
    }

    int rank = 0;
    for (size_t i = 0; i < n; ++i) {
        if (eigenvalues[i] > 1e-4) {
            rank++;
        }
    }

    return rank;
}

SolveStatus System::Solve(int maxIterations, double convergenceTol) {
    // 1. Pack free parameters
    std::vector<double> xParams;
    for (const auto& pt : points) {
        if (!pt.isFixed) {
            xParams.push_back(pt.x);
            xParams.push_back(pt.y);
        }
    }

    size_t numParams = xParams.size();
    if (numParams == 0 || constraints.empty()) {
        dof = static_cast<int>(numParams);
        maxResidual = 0.0;
        return (dof == 0) ? SolveStatus::FullyConstrained : SolveStatus::UnderConstrained;
    }

    double lambda = 1e-3;
    std::vector<double> r = ComputeResiduals(xParams);

    auto computeError = [](const std::vector<double>& res) {
        double sum = 0.0;
        for (double val : res) sum += val * val;
        return sum * 0.5;
    };

    double currentError = computeError(r);
    maxResidual = 0.0;
    for (double val : r) maxResidual = std::max(maxResidual, std::abs(val));

    if (maxResidual < convergenceTol) {
        auto J = ComputeJacobian(xParams, r);
        int rank = EstimateRank(J);
        dof = std::max(0, static_cast<int>(numParams) - rank);
        return (dof == 0) ? SolveStatus::FullyConstrained : SolveStatus::UnderConstrained;
    }

    // Levenberg-Marquardt optimization loop
    bool converged = false;
    for (int iter = 0; iter < maxIterations; ++iter) {
        auto J = ComputeJacobian(xParams, r);
        size_t m = r.size();

        // Normal equations: A = J^T J + lambda I, b = -J^T r
        std::vector<std::vector<double>> A(numParams, std::vector<double>(numParams, 0.0));
        std::vector<double> b(numParams, 0.0);

        for (size_t j1 = 0; j1 < numParams; ++j1) {
            for (size_t j2 = 0; j2 < numParams; ++j2) {
                double sum = 0.0;
                for (size_t i = 0; i < m; ++i) {
                    sum += J[i][j1] * J[i][j2];
                }
                if (j1 == j2) sum += lambda;
                A[j1][j2] = sum;
            }
            double rhs = 0.0;
            for (size_t i = 0; i < m; ++i) {
                rhs -= J[i][j1] * r[i];
            }
            b[j1] = rhs;
        }

        std::vector<double> delta;
        SolveCholesky(A, b, delta);

        // Candidate parameters
        std::vector<double> xCandidate = xParams;
        for (size_t j = 0; j < numParams; ++j) {
            xCandidate[j] += delta[j];
        }

        std::vector<double> rCandidate = ComputeResiduals(xCandidate);
        double candidateError = computeError(rCandidate);

        if (candidateError < currentError) {
            // Accept step
            xParams = xCandidate;
            r = rCandidate;
            currentError = candidateError;
            lambda = std::max(1e-7, lambda * 0.2);

            maxResidual = 0.0;
            for (double val : r) maxResidual = std::max(maxResidual, std::abs(val));

            if (maxResidual < convergenceTol) {
                converged = true;
                break;
            }
        } else {
            // Reject step, increase damping
            lambda = std::min(1e7, lambda * 10.0);
        }
    }

    // Write solved parameters back to points
    size_t paramIdx = 0;
    for (auto& pt : points) {
        if (!pt.isFixed) {
            pt.x = xParams[paramIdx++];
            pt.y = xParams[paramIdx++];
        }
    }

    maxResidual = 0.0;
    for (double val : r) maxResidual = std::max(maxResidual, std::abs(val));

    // Calculate Degrees of Freedom
    auto finalJ = ComputeJacobian(xParams, r);
    int rank = EstimateRank(finalJ);
    dof = std::max(0, static_cast<int>(numParams) - rank);

    if (maxResidual > 1e-2) {
        return SolveStatus::OverConstrained;
    }

    if (dof == 0) {
        return SolveStatus::FullyConstrained;
    } else {
        return SolveStatus::UnderConstrained;
    }
}

} // namespace gcs
} // namespace androcad
