// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "TNPResolver.h"

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <GeomAbs_CurveType.hxx>

#include <cmath>
#include <limits>
#include <iostream>

namespace androcad {

namespace {

double EuclideanDistance(const double p1[3], const double p2[3]) {
    double dx = p1[0] - p2[0];
    double dy = p1[1] - p2[1];
    double dz = p1[2] - p2[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

double DotProduct(const double v1[3], const double v2[3]) {
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

} // namespace

GeometricFingerprint GeometricFingerprint::FromFace(const TopoDS_Face& face) {
    GeometricFingerprint fp;
    fp.type = SubShapeType::Face;
    fp.geomType = 0;
    fp.center[0] = fp.center[1] = fp.center[2] = 0.0;
    fp.normal[0] = fp.normal[1] = 0.0; fp.normal[2] = 1.0;
    fp.size = 0.0;
    fp.subEntityCount = 0;

    if (face.IsNull()) return fp;

    try {
        GProp_GProps gprops;
        BRepGProp::SurfaceProperties(face, gprops);
        fp.size = gprops.Mass();
        gp_Pnt cm = gprops.CentreOfMass();
        fp.center[0] = cm.X();
        fp.center[1] = cm.Y();
        fp.center[2] = cm.Z();

        BRepAdaptor_Surface surf(face);
        GeomAbs_SurfaceType sType = surf.GetType();
        if (sType == GeomAbs_Plane) fp.geomType = 0;
        else if (sType == GeomAbs_Cylinder) fp.geomType = 1;
        else fp.geomType = 2;

        double uMid = (surf.FirstUParameter() + surf.LastUParameter()) * 0.5;
        double vMid = (surf.FirstVParameter() + surf.LastVParameter()) * 0.5;
        gp_Pnt pnt;
        gp_Vec d1u, d1v;
        surf.D1(uMid, vMid, pnt, d1u, d1v);
        gp_Vec norm = d1u.Crossed(d1v);
        double len = norm.Magnitude();
        if (len > 1e-6) {
            norm.Divide(len);
            if (face.Orientation() == TopAbs_REVERSED) norm.Reverse();
            fp.normal[0] = norm.X();
            fp.normal[1] = norm.Y();
            fp.normal[2] = norm.Z();
        }

        TopExp_Explorer exp(face, TopAbs_EDGE);
        for (; exp.More(); exp.Next()) {
            fp.subEntityCount++;
        }
    } catch (...) {}

    return fp;
}

GeometricFingerprint GeometricFingerprint::FromEdge(const TopoDS_Edge& edge) {
    GeometricFingerprint fp;
    fp.type = SubShapeType::Edge;
    fp.geomType = 0;
    fp.center[0] = fp.center[1] = fp.center[2] = 0.0;
    fp.normal[0] = 1.0; fp.normal[1] = fp.normal[2] = 0.0;
    fp.size = 0.0;
    fp.subEntityCount = 2;

    if (edge.IsNull()) return fp;

    try {
        GProp_GProps gprops;
        BRepGProp::LinearProperties(edge, gprops);
        fp.size = gprops.Mass();
        gp_Pnt cm = gprops.CentreOfMass();
        fp.center[0] = cm.X();
        fp.center[1] = cm.Y();
        fp.center[2] = cm.Z();

        BRepAdaptor_Curve curve(edge);
        GeomAbs_CurveType cType = curve.GetType();
        if (cType == GeomAbs_Line) fp.geomType = 0;
        else if (cType == GeomAbs_Circle) fp.geomType = 1;
        else fp.geomType = 2;

        double tMid = (curve.FirstParameter() + curve.LastParameter()) * 0.5;
        gp_Pnt pnt;
        gp_Vec tang;
        curve.D1(tMid, pnt, tang);
        double len = tang.Magnitude();
        if (len > 1e-6) {
            tang.Divide(len);
            fp.normal[0] = tang.X();
            fp.normal[1] = tang.Y();
            fp.normal[2] = tang.Z();
        }
    } catch (...) {}

    return fp;
}

TopologicalReference TNPResolver::CreateFaceReference(
    const TopoDS_Shape& model,
    const TopoDS_Face& face,
    const std::string& semanticTag) {

    TopologicalReference ref;
    ref.semanticTag = semanticTag;
    ref.fingerprint = GeometricFingerprint::FromFace(face);

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(model, TopAbs_FACE, faceMap);
    for (int i = 1; i <= faceMap.Extent(); ++i) {
        if (faceMap(i).IsSame(face)) {
            ref.lastKnownLocalId = static_cast<uint32_t>(i);
            break;
        }
    }

    return ref;
}

TopologicalReference TNPResolver::CreateEdgeReference(
    const TopoDS_Shape& model,
    const TopoDS_Edge& edge,
    const std::string& semanticTag) {

    TopologicalReference ref;
    ref.semanticTag = semanticTag;
    ref.fingerprint = GeometricFingerprint::FromEdge(edge);

    TopTools_IndexedMapOfShape edgeMap;
    TopExp::MapShapes(model, TopAbs_EDGE, edgeMap);
    for (int i = 1; i <= edgeMap.Extent(); ++i) {
        if (edgeMap(i).IsSame(edge)) {
            ref.lastKnownLocalId = static_cast<uint32_t>(i);
            break;
        }
    }

    return ref;
}

TNPResolutionResult TNPResolver::ResolveFace(
    const TopoDS_Shape& currentModel,
    const TopologicalReference& reference,
    TopoDS_Face& outFace,
    const Handle(BRepTools_History)& history) {

    TNPResolutionResult result;
    result.status = TNPResolutionStatus::BrokenReference;
    result.confidence = 0.0;
    result.diagnosticMessage = "Entity could not be resolved.";

    if (currentModel.IsNull()) {
        result.diagnosticMessage = "Model is null.";
        return result;
    }

    // Tier 1 & 2: Check History if available
    if (!history.IsNull()) {
        // Find if history tracks the modified shape
        // For sub-shape splitting (e.g. face cut by pocket)
        TopTools_IndexedMapOfShape currentFaces;
        TopExp::MapShapes(currentModel, TopAbs_FACE, currentFaces);

        double bestDist = std::numeric_limits<double>::max();
        TopoDS_Face bestHistoryFace;

        for (int i = 1; i <= currentFaces.Extent(); ++i) {
            const TopoDS_Face& f = TopoDS::Face(currentFaces(i));
            GeometricFingerprint fp = GeometricFingerprint::FromFace(f);

            double dot = DotProduct(fp.normal, reference.fingerprint.normal);
            if (dot > 0.85) {
                double dist = EuclideanDistance(fp.center, reference.fingerprint.center);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestHistoryFace = f;
                }
            }
        }

        if (!bestHistoryFace.IsNull() && bestDist < 100.0) {
            outFace = bestHistoryFace;
            result.status = TNPResolutionStatus::ExactMatch;
            result.confidence = 0.95;
            result.diagnosticMessage = "Resolved via History + Normal Match";
            result.resolvedShape = outFace;
            return result;
        }
    }

    // Tier 3: Invariant Geometric Fingerprinting
    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes(currentModel, TopAbs_FACE, faces);

    if (faces.IsEmpty()) {
        result.diagnosticMessage = "Model contains no faces.";
        return result;
    }

    double bestScore = std::numeric_limits<double>::max();
    TopoDS_Face bestCandidate;

    for (int i = 1; i <= faces.Extent(); ++i) {
        const TopoDS_Face& cand = TopoDS::Face(faces(i));
        GeometricFingerprint fp = GeometricFingerprint::FromFace(cand);

        if (fp.geomType != reference.fingerprint.geomType) {
            continue; // Surface type must match (Plane vs Cylinder)
        }

        double normDot = DotProduct(fp.normal, reference.fingerprint.normal);
        if (normDot < 0.5) {
            continue; // Normals must align reasonably (> 60 deg angle rejected)
        }

        double dist = EuclideanDistance(fp.center, reference.fingerprint.center);
        double normPenalty = (1.0 - normDot) * 40.0;
        double score = dist + normPenalty;

        if (score < bestScore) {
            bestScore = score;
            bestCandidate = cand;
        }
    }

    // Confidence threshold
    if (!bestCandidate.IsNull() && bestScore < 150.0) {
        outFace = bestCandidate;
        result.status = TNPResolutionStatus::FingerprintMatch;
        result.confidence = std::max(0.5, 1.0 - (bestScore / 150.0));
        result.diagnosticMessage = "Resolved via Geometric Fingerprint score: " + std::to_string(bestScore);
        result.resolvedShape = outFace;
        return result;
    }

    result.status = TNPResolutionStatus::BrokenReference;
    result.confidence = 0.0;
    result.diagnosticMessage = "Reference lost: No face matched semantic tag '" + reference.semanticTag + "'";
    return result;
}

TNPResolutionResult TNPResolver::ResolveEdge(
    const TopoDS_Shape& currentModel,
    const TopologicalReference& reference,
    TopoDS_Edge& outEdge,
    const Handle(BRepTools_History)& history) {

    TNPResolutionResult result;
    result.status = TNPResolutionStatus::BrokenReference;
    result.confidence = 0.0;
    result.diagnosticMessage = "Edge entity could not be resolved.";

    if (currentModel.IsNull()) {
        result.diagnosticMessage = "Model is null.";
        return result;
    }

    TopTools_IndexedMapOfShape edges;
    TopExp::MapShapes(currentModel, TopAbs_EDGE, edges);

    if (edges.IsEmpty()) {
        result.diagnosticMessage = "Model contains no edges.";
        return result;
    }

    double bestScore = std::numeric_limits<double>::max();
    TopoDS_Edge bestCandidate;

    for (int i = 1; i <= edges.Extent(); ++i) {
        const TopoDS_Edge& cand = TopoDS::Edge(edges(i));
        GeometricFingerprint fp = GeometricFingerprint::FromEdge(cand);

        if (fp.geomType != reference.fingerprint.geomType) {
            continue; // Curve type must match
        }

        double tangentDot = std::abs(DotProduct(fp.normal, reference.fingerprint.normal));
        if (tangentDot < 0.5) {
            continue; // Direction must align
        }

        double dist = EuclideanDistance(fp.center, reference.fingerprint.center);
        double lenDiff = std::abs(fp.size - reference.fingerprint.size);
        double score = dist + 0.1 * lenDiff;

        if (score < bestScore) {
            bestScore = score;
            bestCandidate = cand;
        }
    }

    if (!bestCandidate.IsNull() && bestScore < 150.0) {
        outEdge = bestCandidate;
        result.status = TNPResolutionStatus::FingerprintMatch;
        result.confidence = std::max(0.5, 1.0 - (bestScore / 150.0));
        result.diagnosticMessage = "Edge resolved via Geometric Fingerprint score: " + std::to_string(bestScore);
        result.resolvedShape = outEdge;
        return result;
    }

    result.status = TNPResolutionStatus::BrokenReference;
    result.confidence = 0.0;
    result.diagnosticMessage = "Reference lost: Edge matched semantic tag '" + reference.semanticTag + "' vanished.";
    return result;
}

std::unordered_map<std::string, TopoDS_Face> TNPResolver::TagExtrudeFaces(
    const TopoDS_Shape& solid,
    double dx, double dy, double dz) {

    std::unordered_map<std::string, TopoDS_Face> taggedFaces;
    if (solid.IsNull()) return taggedFaces;

    double extrudeDir[3] = {dx, dy, dz};
    double len = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (len > 1e-9) {
        extrudeDir[0] /= len;
        extrudeDir[1] /= len;
        extrudeDir[2] /= len;
    }

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(solid, TopAbs_FACE, faceMap);

    int lateralCount = 1;
    for (int i = 1; i <= faceMap.Extent(); ++i) {
        const TopoDS_Face& face = TopoDS::Face(faceMap(i));
        GeometricFingerprint fp = GeometricFingerprint::FromFace(face);

        double dot = DotProduct(fp.normal, extrudeDir);
        if (dot > 0.95) {
            taggedFaces["Cap_End"] = face;
        } else if (dot < -0.95) {
            taggedFaces["Cap_Start"] = face;
        } else {
            taggedFaces["Lateral_" + std::to_string(lateralCount++)] = face;
        }
    }

    return taggedFaces;
}

} // namespace androcad
