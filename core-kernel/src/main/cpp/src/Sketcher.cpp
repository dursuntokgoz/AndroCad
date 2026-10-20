// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "Sketcher.h"

#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <GC_MakeSegment.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>

#include <iostream>
#include <cmath>

namespace androcad {

Sketcher::Sketcher(const SketchPlane& plane) : plane(plane) {}

void Sketcher::Clear() {
    solver.Clear();
    curves.clear();
}

gp_Pnt Sketcher::ToWorldPoint(double u, double v) const {
    double wx = plane.ox + u * plane.ux + v * plane.vx;
    double wy = plane.oy + u * plane.uy + v * plane.vy;
    double wz = plane.oz + u * plane.uz + v * plane.vz;
    return gp_Pnt(wx, wy, wz);
}

int Sketcher::AddPoint(double u, double v, bool isFixed) {
    return solver.AddPoint(u, v, isFixed);
}

int Sketcher::AddLine(int pStart, int pEnd) {
    int id = static_cast<int>(curves.size());
    curves.push_back(SketchCurve{id, CurveType::Line, pStart, pEnd, -1, 0.0});
    return id;
}

int Sketcher::AddCircle(int pCenter, double radius) {
    auto centerPt = solver.GetPoint(pCenter);
    int pOnCirc = solver.AddPoint(centerPt.x + radius, centerPt.y, false);
    solver.AddConstraint(gcs::ConstraintType::CircleRadius, pCenter, pOnCirc, -1, -1, radius);

    int id = static_cast<int>(curves.size());
    curves.push_back(SketchCurve{id, CurveType::Circle, pCenter, pOnCirc, -1, radius});
    return id;
}

int Sketcher::AddRectangle(double u, double v, double width, double height) {
    int p0 = solver.AddPoint(u, v, false);
    int p1 = solver.AddPoint(u + width, v, false);
    int p2 = solver.AddPoint(u + width, v + height, false);
    int p3 = solver.AddPoint(u, v + height, false);

    int l0 = AddLine(p0, p1);
    int l1 = AddLine(p1, p2);
    int l2 = AddLine(p2, p3);
    int l3 = AddLine(p3, p0);

    // Standard rectangle constraints
    solver.AddConstraint(gcs::ConstraintType::Horizontal, p0, p1);
    solver.AddConstraint(gcs::ConstraintType::Vertical, p1, p2);
    solver.AddConstraint(gcs::ConstraintType::Horizontal, p3, p2);
    solver.AddConstraint(gcs::ConstraintType::Vertical, p0, p3);
    solver.AddConstraint(gcs::ConstraintType::Distance, p0, p1, -1, -1, width);
    solver.AddConstraint(gcs::ConstraintType::Distance, p1, p2, -1, -1, height);

    return l0;
}

int Sketcher::AddCoincident(int p1, int p2) {
    return solver.AddConstraint(gcs::ConstraintType::Coincident, p1, p2);
}

int Sketcher::AddHorizontal(int p1, int p2) {
    return solver.AddConstraint(gcs::ConstraintType::Horizontal, p1, p2);
}

int Sketcher::AddVertical(int p1, int p2) {
    return solver.AddConstraint(gcs::ConstraintType::Vertical, p1, p2);
}

int Sketcher::AddDistance(int p1, int p2, double distance) {
    return solver.AddConstraint(gcs::ConstraintType::Distance, p1, p2, -1, -1, distance);
}

int Sketcher::AddParallel(int l1_p1, int l1_p2, int l2_p1, int l2_p2) {
    return solver.AddConstraint(gcs::ConstraintType::Parallel, l1_p1, l1_p2, l2_p1, l2_p2);
}

int Sketcher::AddPerpendicular(int l1_p1, int l1_p2, int l2_p1, int l2_p2) {
    return solver.AddConstraint(gcs::ConstraintType::Perpendicular, l1_p1, l1_p2, l2_p1, l2_p2);
}

int Sketcher::AddEqualLength(int l1_p1, int l1_p2, int l2_p1, int l2_p2) {
    return solver.AddConstraint(gcs::ConstraintType::EqualLength, l1_p1, l1_p2, l2_p1, l2_p2);
}

gcs::SolveStatus Sketcher::Solve() {
    return solver.Solve();
}

SketchResult Sketcher::BuildProfileFace() {
    SketchResult result;
    result.solveStatus = solver.Solve();
    result.degreesOfFreedom = solver.GetDegreesOfFreedom();
    result.pointCount = static_cast<int>(solver.GetPoints().size());
    result.curveCount = static_cast<int>(curves.size());

    if (curves.empty()) {
        result.errorMessage = "Sketch has no curves.";
        return result;
    }

    try {
        BRepBuilderAPI_MakeWire wireMaker;

        for (const auto& curve : curves) {
            if (curve.type == CurveType::Line) {
                auto pStart = solver.GetPoint(curve.p1);
                auto pEnd = solver.GetPoint(curve.p2);

                gp_Pnt wStart = ToWorldPoint(pStart.x, pStart.y);
                gp_Pnt wEnd = ToWorldPoint(pEnd.x, pEnd.y);

                if (wStart.Distance(wEnd) < 1e-6) {
                    continue; // Skip zero-length edge
                }

                Handle(Geom_TrimmedCurve) seg = GC_MakeSegment(wStart, wEnd).Value();
                TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(seg);
                wireMaker.Add(edge);

            } else if (curve.type == CurveType::Circle) {
                auto pCenter = solver.GetPoint(curve.p1);
                auto pOnCirc = solver.GetPoint(curve.p2);

                double dx = pOnCirc.x - pCenter.x;
                double dy = pOnCirc.y - pCenter.y;
                double r = std::sqrt(dx * dx + dy * dy);
                if (r < 1e-6) r = curve.radius;

                gp_Pnt wCenter = ToWorldPoint(pCenter.x, pCenter.y);
                gp_Dir normalDir(plane.nx, plane.ny, plane.nz);
                gp_Dir uDir(plane.ux, plane.uy, plane.uz);
                gp_Ax2 axis(wCenter, normalDir, uDir);
                gp_Circ circle(axis, r);

                TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circle);
                wireMaker.Add(edge);
            }
        }

        if (!wireMaker.IsDone()) {
            result.errorMessage = "Failed to assemble sketch wire.";
            return result;
        }

        TopoDS_Wire wire = wireMaker.Wire();
        result.wire = wire;
        result.isClosedProfile = wire.Closed();

        if (!result.isClosedProfile) {
            result.errorMessage = "Sketch profile is open (not closed).";
            return result;
        }

        // Construct planar bounded face on sketch plane
        gp_Pln occtPlane(gp_Pnt(plane.ox, plane.oy, plane.oz), gp_Dir(plane.nx, plane.ny, plane.nz));
        BRepBuilderAPI_MakeFace faceMaker(occtPlane, wire, Standard_True);

        if (!faceMaker.IsDone()) {
            result.errorMessage = "Failed to create face from sketch wire.";
            return result;
        }

        TopoDS_Face face = faceMaker.Face();
        BRepCheck_Analyzer checker(face);
        if (!checker.IsValid()) {
            result.errorMessage = "Constructed face failed topological validation.";
            return result;
        }

        // Calculate surface area
        GProp_GProps props;
        BRepGProp::SurfaceProperties(face, props);
        result.profileArea = props.Mass();
        result.face = face;
        result.success = true;

    } catch (const Standard_Failure& e) {
        result.errorMessage = std::string("OCCT Standard_Failure: ") + e.GetMessageString();
        result.success = false;
    } catch (const std::exception& e) {
        result.errorMessage = std::string("C++ Exception: ") + e.what();
        result.success = false;
    }

    return result;
}

} // namespace androcad
