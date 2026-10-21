// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "FastenerEngine.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Compound.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Standard_Failure.hxx>

#include <cmath>
#include <map>
#include <algorithm>

namespace androcad {

namespace {

constexpr double M_PI_VAL = 3.14159265358979323846;

double ComputeVolume(const TopoDS_Shape& shape) {
    if (shape.IsNull()) return 0.0;
    try {
        GProp_GProps props;
        BRepGProp::VolumeProperties(shape, props);
        return props.Mass();
    } catch (...) {
        return 0.0;
    }
}

const std::map<std::string, FastenerStandard>& GetStandardsMap() {
    static const std::map<std::string, FastenerStandard> standards = {
        {"M3",  {"M3",  3.0, 0.50, 3.4,  6.0,  3.0,  6.7,  90.0, 2.5}},
        {"M4",  {"M4",  4.0, 0.70, 4.5,  8.0,  4.0,  8.8,  90.0, 3.3}},
        {"M5",  {"M5",  5.0, 0.80, 5.5,  10.0, 5.0,  10.9, 90.0, 4.2}},
        {"M6",  {"M6",  6.0, 1.00, 6.6,  11.0, 6.0,  13.0, 90.0, 5.0}},
        {"M8",  {"M8",  8.0, 1.25, 9.0,  15.0, 8.0,  17.3, 90.0, 6.8}},
        {"M10", {"M10", 10.0, 1.50, 11.0, 18.0, 10.0, 21.6, 90.0, 8.5}},
        {"M12", {"M12", 12.0, 1.75, 13.5, 20.0, 12.0, 25.8, 90.0, 10.2}}
    };
    return standards;
}

} // namespace

FastenerStandard FastenerEngine::GetStandard(const std::string& name) {
    const auto& map = GetStandardsMap();
    auto it = map.find(name);
    if (it != map.end()) {
        return it->second;
    }
    // Default fallback M6
    return map.at("M6");
}

std::vector<std::string> FastenerEngine::GetAvailableStandards() {
    return {"M3", "M4", "M5", "M6", "M8", "M10", "M12"};
}

HoleResult FastenerEngine::CreateStandardHole(
        const TopoDS_Shape& solid,
        const gp_Pnt& position,
        const gp_Dir& direction,
        const std::string& standardName,
        HoleType type,
        double depthMm) {
    HoleResult result;
    if (solid.IsNull() || depthMm <= 0.0) {
        result.success = false;
        result.errorMessage = "Invalid solid or hole depth";
        return result;
    }

    try {
        FastenerStandard stdSpec = GetStandard(standardName);
        gp_Ax2 holeAx(position, direction);

        TopoDS_Shape cutter;

        double holeRadius = stdSpec.clearanceHoleDiaMm / 2.0;
        if (type == HoleType::TAPPED) {
            holeRadius = stdSpec.tapDrillDiaMm / 2.0;
        }

        // Base hole cylinder
        TopoDS_Shape baseCyl = BRepPrimAPI_MakeCylinder(holeAx, holeRadius, depthMm).Shape();

        if (type == HoleType::COUNTERBORE) {
            double cbRadius = stdSpec.counterboreDiaMm / 2.0;
            double cbDepth = std::min(stdSpec.counterboreDepthMm, depthMm * 0.8);
            TopoDS_Shape cbCyl = BRepPrimAPI_MakeCylinder(holeAx, cbRadius, cbDepth).Shape();

            BRepAlgoAPI_Fuse fuser(baseCyl, cbCyl);
            fuser.Build();
            cutter = fuser.IsDone() ? fuser.Shape() : baseCyl;
        } else if (type == HoleType::COUNTERSINK) {
            double csTopRadius = stdSpec.countersinkDiaMm / 2.0;
            double halfAngleRad = (stdSpec.countersinkAngleDeg / 2.0) * (M_PI_VAL / 180.0);
            double coneHeight = (csTopRadius - holeRadius) / std::tan(halfAngleRad);
            coneHeight = std::min(coneHeight, depthMm * 0.8);

            TopoDS_Shape cone = BRepPrimAPI_MakeCone(holeAx, csTopRadius, holeRadius, coneHeight).Shape();

            BRepAlgoAPI_Fuse fuser(baseCyl, cone);
            fuser.Build();
            cutter = fuser.IsDone() ? fuser.Shape() : baseCyl;
        } else {
            cutter = baseCyl;
        }

        BRepAlgoAPI_Cut cutOp(solid, cutter);
        cutOp.Build();

        if (!cutOp.IsDone()) {
            result.success = false;
            result.errorMessage = "Boolean cut for hole failed";
            return result;
        }

        result.shape = cutOp.Shape();
        result.success = true;
        result.volumeMm3 = ComputeVolume(result.shape);
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = e.GetMessageString();
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

ThreadResult FastenerEngine::CreateThreadedBolt(
        const std::string& standardName,
        double shaftLengthMm,
        double threadLengthMm,
        bool hexHead) {
    ThreadResult result;
    if (shaftLengthMm <= 0.0 || threadLengthMm <= 0.0) {
        result.success = false;
        result.errorMessage = "Invalid shaft or thread length";
        return result;
    }

    try {
        FastenerStandard stdSpec = GetStandard(standardName);
        double rShaft = stdSpec.nominalDiameterMm / 2.0;
        double rHead = rShaft * 1.8;
        double headHeight = rShaft * 1.3;

        // 1. Head
        gp_Ax2 headAx(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        TopoDS_Shape head = BRepPrimAPI_MakeCylinder(headAx, rHead, headHeight).Shape();

        // 2. Shaft
        gp_Ax2 shaftAx(gp_Pnt(0, 0, headHeight), gp_Dir(0, 0, 1));
        TopoDS_Shape shaft = BRepPrimAPI_MakeCylinder(shaftAx, rShaft, shaftLengthMm).Shape();

        // Fuse head and shaft
        BRepAlgoAPI_Fuse baseFuser(head, shaft);
        baseFuser.Build();
        TopoDS_Shape blank = baseFuser.IsDone() ? baseFuser.Shape() : shaft;

        // 3. Threaded teeth/ridges along threadLength
        double pitch = stdSpec.pitchMm;
        int toothCount = static_cast<int>(threadLengthMm / pitch);
        if (toothCount < 1) toothCount = 1;

        double threadStartZ = headHeight + shaftLengthMm - threadLengthMm;
        double toothDepth = pitch * 0.45;

        // Create compound of thread ridges
        BRep_Builder builder;
        TopoDS_Compound threadRings;
        builder.MakeCompound(threadRings);

        for (int i = 0; i < toothCount; ++i) {
            double z = threadStartZ + i * pitch;
            gp_Ax2 ringAx(gp_Pnt(0, 0, z), gp_Dir(0, 0, 1));
            TopoDS_Shape ring = BRepPrimAPI_MakeCylinder(ringAx, rShaft + toothDepth, pitch * 0.5).Shape();
            builder.Add(threadRings, ring);
        }

        BRepAlgoAPI_Fuse threadFuser(blank, threadRings);
        threadFuser.Build();
        result.shape = threadFuser.IsDone() ? threadFuser.Shape() : blank;

        result.success = true;
        result.toothCount = toothCount;
        result.pitchMm = pitch;
        result.outerDiameterMm = (rShaft + toothDepth) * 2.0;
        result.volumeMm3 = ComputeVolume(result.shape);
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = e.GetMessageString();
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

ProjectionResult FastenerEngine::ProjectEdgesToPlane(
        const TopoDS_Shape& solid,
        const gp_Pln& sketchPlane,
        const std::vector<int>& targetEdgeIds) {
    ProjectionResult result;
    if (solid.IsNull()) {
        result.success = false;
        result.errorMessage = "Solid shape is null";
        return result;
    }

    try {
        gp_Pnt planeOrigin = sketchPlane.Location();
        gp_Dir planeX = sketchPlane.XAxis().Direction();
        gp_Dir planeY = sketchPlane.YAxis().Direction();

        int edgeIdx = 1;
        for (TopExp_Explorer exp(solid, TopAbs_EDGE); exp.More(); exp.Next(), ++edgeIdx) {
            if (!targetEdgeIds.empty()) {
                bool found = false;
                for (int id : targetEdgeIds) {
                    if (id == edgeIdx) { found = true; break; }
                }
                if (!found) continue;
            }

            TopoDS_Edge edge = TopoDS::Edge(exp.Current());
            if (edge.IsNull()) continue;

            BRepAdaptor_Curve curve(edge);
            gp_Pnt pStart = curve.Value(curve.FirstParameter());
            gp_Pnt pEnd = curve.Value(curve.LastParameter());

            // Project 3D points onto 2D plane coordinate system (U, V)
            gp_Vec vStart(planeOrigin, pStart);
            gp_Vec vEnd(planeOrigin, pEnd);

            ProjectedSegment seg;
            seg.startU = vStart.Dot(gp_Vec(planeX));
            seg.startV = vStart.Dot(gp_Vec(planeY));
            seg.endU = vEnd.Dot(gp_Vec(planeX));
            seg.endV = vEnd.Dot(gp_Vec(planeY));
            seg.isArc = (curve.GetType() == GeomAbs_Circle);

            if (seg.isArc) {
                gp_Pnt center = curve.Circle().Location();
                gp_Vec vCenter(planeOrigin, center);
                seg.centerU = vCenter.Dot(gp_Vec(planeX));
                seg.centerV = vCenter.Dot(gp_Vec(planeY));
                seg.radius = curve.Circle().Radius();
            }

            result.segments.push_back(seg);
        }

        result.success = true;
        result.edgeCount = static_cast<int>(result.segments.size());
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = e.GetMessageString();
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

BSplineResult FastenerEngine::GenerateCubicBSpline(
        const std::vector<SplinePoint2D>& controlPoints,
        int sampleCount) {
    BSplineResult result;
    if (controlPoints.size() < 2) {
        result.success = false;
        result.errorMessage = "At least 2 control points required for B-Spline";
        return result;
    }

    try {
        result.degree = 3;
        result.poleCount = static_cast<int>(controlPoints.size());
        result.knotCount = static_cast<int>(controlPoints.size()) + 4; // degree + 1 + poles

        result.sampledPoints.reserve(sampleCount);

        // Catmull-Rom cubic spline interpolation through control points
        double totalLength = 0.0;
        SplinePoint2D prevPoint = controlPoints[0];

        int n = static_cast<int>(controlPoints.size());
        for (int step = 0; step < sampleCount; ++step) {
            double tGlobal = (static_cast<double>(step) / (sampleCount - 1)) * (n - 1);
            int i = static_cast<int>(tGlobal);
            if (i >= n - 1) i = n - 2;
            double t = tGlobal - i;

            // 4 points for cubic interpolation: P0, P1, P2, P3
            SplinePoint2D p0 = (i > 0) ? controlPoints[i - 1] : controlPoints[i];
            SplinePoint2D p1 = controlPoints[i];
            SplinePoint2D p2 = controlPoints[i + 1];
            SplinePoint2D p3 = (i + 2 < n) ? controlPoints[i + 2] : controlPoints[i + 1];

            // Standard Catmull-Rom cubic spline basis
            double t2 = t * t;
            double t3 = t2 * t;

            double u = 0.5 * ((2.0 * p1.u) +
                             (-p0.u + p2.u) * t +
                             (2.0 * p0.u - 5.0 * p1.u + 4.0 * p2.u - p3.u) * t2 +
                             (-p0.u + 3.0 * p1.u - 3.0 * p2.u + p3.u) * t3);

            double v = 0.5 * ((2.0 * p1.v) +
                             (-p0.v + p2.v) * t +
                             (2.0 * p0.v - 5.0 * p1.v + 4.0 * p2.v - p3.v) * t2 +
                             (-p0.v + 3.0 * p1.v - 3.0 * p2.v + p3.v) * t3);

            SplinePoint2D curr{u, v};
            result.sampledPoints.push_back(curr);

            if (step > 0) {
                double du = curr.u - prevPoint.u;
                double dv = curr.v - prevPoint.v;
                totalLength += std::sqrt(du * du + dv * dv);
            }
            prevPoint = curr;
        }

        result.lengthMm = totalLength;
        result.success = true;
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

} // namespace androcad
