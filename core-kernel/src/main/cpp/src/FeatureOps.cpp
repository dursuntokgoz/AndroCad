// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "FeatureOps.h"

#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_QuasiUniformDeflection.hxx>
#include <Poly_Triangulation.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <Standard_Failure.hxx>

#include <cmath>
#include <iostream>

namespace androcad {

MeshPayload FeatureOps::ShapeToMeshPayload(const TopoDS_Shape& shape,
                                           double linearDeflection,
                                           double angularDeflection) {
    MeshPayload result;
    result.volume = 0.0;
    result.faceCount = 0;
    result.edgeCount = 0;
    result.triangleCount = 0;
    result.bounds = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    result.success = false;

    if (shape.IsNull()) {
        result.errorMessage = "Input shape is null.";
        return result;
    }

    try {
        // 1. Measure analytical volume
        GProp_GProps system;
        BRepGProp::VolumeProperties(shape, system);
        result.volume = system.Mass();

        // 2. Compute Bounding Box
        Bnd_Box aBox;
        BRepBndLib::Add(shape, aBox);
        Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
        aBox.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
        result.bounds.minX = static_cast<float>(aXmin);
        result.bounds.minY = static_cast<float>(aYmin);
        result.bounds.minZ = static_cast<float>(aZmin);
        result.bounds.maxX = static_cast<float>(aXmax);
        result.bounds.maxY = static_cast<float>(aYmax);
        result.bounds.maxZ = static_cast<float>(aZmax);

        // 3. Extract unique edges and discretize
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
        result.edgeCount = edgeMap.Extent();

        for (int i = 1; i <= edgeMap.Extent(); ++i) {
            const TopoDS_Edge& edge = TopoDS::Edge(edgeMap(i));
            uint32_t edgeId = static_cast<uint32_t>(i);

            BRepAdaptor_Curve curve(edge);
            GCPnts_QuasiUniformDeflection discretizer(curve, linearDeflection);
            if (discretizer.IsDone() && discretizer.NbPoints() >= 2) {
                for (int pIdx = 1; pIdx < discretizer.NbPoints(); ++pIdx) {
                    gp_Pnt p1 = discretizer.Value(pIdx);
                    gp_Pnt p2 = discretizer.Value(pIdx + 1);

                    EdgeVertex ev1{static_cast<float>(p1.X()), static_cast<float>(p1.Y()), static_cast<float>(p1.Z()), edgeId};
                    EdgeVertex ev2{static_cast<float>(p2.X()), static_cast<float>(p2.Y()), static_cast<float>(p2.Z()), edgeId};
                    result.edgeVertices.push_back(ev1);
                    result.edgeVertices.push_back(ev2);
                }
            }
        }

        // 4. Tessellate using BRepMesh
        BRepMesh_IncrementalMesh mesher(shape, linearDeflection, Standard_False, angularDeflection, Standard_True);
        mesher.Perform();
        if (!mesher.IsDone()) {
            result.errorMessage = "BRepMesh_IncrementalMesh failed to tessellate shape.";
            return result;
        }

        // 5. Extract faces
        uint32_t currentFaceId = 1;
        uint32_t vertexOffset = 0;

        for (TopExp_Explorer faceExp(shape, TopAbs_FACE); faceExp.More(); faceExp.Next(), ++currentFaceId) {
            const TopoDS_Face& face = TopoDS::Face(faceExp.Current());
            TopLoc_Location location;
            Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, location);

            if (tri.IsNull() || tri->NbTriangles() == 0) {
                continue;
            }

            const gp_Trsf& trsf = location.Transformation();
            bool isReversed = (face.Orientation() == TopAbs_REVERSED);

            int nbNodes = tri->NbNodes();
            std::vector<Vertex> faceVertices(nbNodes);

            for (int i = 1; i <= nbNodes; ++i) {
                gp_Pnt p = tri->Node(i).Transformed(trsf);
                Vertex v;
                v.x = static_cast<float>(p.X());
                v.y = static_cast<float>(p.Y());
                v.z = static_cast<float>(p.Z());
                v.nx = 0.0f;
                v.ny = 0.0f;
                v.nz = 0.0f;
                v.faceId = currentFaceId;
                faceVertices[i - 1] = v;
            }

            int nbTriangles = tri->NbTriangles();
            for (int i = 1; i <= nbTriangles; ++i) {
                int n1, n2, n3;
                tri->Triangle(i).Get(n1, n2, n3);

                if (isReversed) {
                    std::swap(n2, n3);
                }

                uint32_t idx1 = vertexOffset + static_cast<uint32_t>(n1 - 1);
                uint32_t idx2 = vertexOffset + static_cast<uint32_t>(n2 - 1);
                uint32_t idx3 = vertexOffset + static_cast<uint32_t>(n3 - 1);

                result.indices.push_back(idx1);
                result.indices.push_back(idx2);
                result.indices.push_back(idx3);

                const Vertex& v1 = faceVertices[n1 - 1];
                const Vertex& v2 = faceVertices[n2 - 1];
                const Vertex& v3 = faceVertices[n3 - 1];

                float ax = v2.x - v1.x, ay = v2.y - v1.y, az = v2.z - v1.z;
                float bx = v3.x - v1.x, by = v3.y - v1.y, bz = v3.z - v1.z;
                float nx = ay * bz - az * by;
                float ny = az * bx - ax * bz;
                float nz = ax * by - ay * bx;
                float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                if (len > 1e-6f) {
                    nx /= len; ny /= len; nz /= len;
                }

                faceVertices[n1 - 1].nx += nx;
                faceVertices[n1 - 1].ny += ny;
                faceVertices[n1 - 1].nz += nz;

                faceVertices[n2 - 1].nx += nx;
                faceVertices[n2 - 1].ny += ny;
                faceVertices[n2 - 1].nz += nz;

                faceVertices[n3 - 1].nx += nx;
                faceVertices[n3 - 1].ny += ny;
                faceVertices[n3 - 1].nz += nz;
            }

            for (auto& v : faceVertices) {
                float len = std::sqrt(v.nx * v.nx + v.ny * v.ny + v.nz * v.nz);
                if (len > 1e-6f) {
                    v.nx /= len; v.ny /= len; v.nz /= len;
                }
                result.vertices.push_back(v);
            }

            vertexOffset += static_cast<uint32_t>(nbNodes);
            result.triangleCount += nbTriangles;
            result.faceCount++;
        }

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

MeshPayload FeatureOps::MakeExtrude(const TopoDS_Face& profileFace,
                                   double dx, double dy, double dz,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection,
                                   double angularDeflection) {
    if (profileFace.IsNull()) {
        MeshPayload err; err.success = false; err.errorMessage = "Profile face is null.";
        return err;
    }

    double lenSq = dx * dx + dy * dy + dz * dz;
    if (lenSq < 1e-9) {
        MeshPayload err; err.success = false; err.errorMessage = "Extrude vector length must be greater than zero.";
        return err;
    }

    try {
        gp_Vec vec(dx, dy, dz);
        BRepPrimAPI_MakePrism prism(profileFace, vec);
        prism.Build();

        if (!prism.IsDone()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepPrimAPI_MakePrism failed to construct solid.";
            return err;
        }

        outSolid = prism.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Extrude Standard_Failure: ") + e.GetMessageString();
        return err;
    }
}

MeshPayload FeatureOps::MakeRevolve(const TopoDS_Face& profileFace,
                                   double ax, double ay, double az,
                                   double dx, double dy, double dz,
                                   double angleRad,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection,
                                   double angularDeflection) {
    if (profileFace.IsNull()) {
        MeshPayload err; err.success = false; err.errorMessage = "Profile face is null.";
        return err;
    }

    double dirLenSq = dx * dx + dy * dy + dz * dz;
    if (dirLenSq < 1e-9 || std::abs(angleRad) < 1e-6) {
        MeshPayload err; err.success = false; err.errorMessage = "Revolve direction vector must be non-zero and angle must be non-zero.";
        return err;
    }

    try {
        gp_Pnt pnt(ax, ay, az);
        gp_Dir dir(dx, dy, dz);
        gp_Ax1 axis(pnt, dir);

        BRepPrimAPI_MakeRevol revol(profileFace, axis, angleRad);
        revol.Build();

        if (!revol.IsDone()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepPrimAPI_MakeRevol failed to revolve face.";
            return err;
        }

        outSolid = revol.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Revolve Standard_Failure: ") + e.GetMessageString();
        return err;
    }
}

MeshPayload FeatureOps::MakeFillet(const TopoDS_Shape& solid,
                                  double radius,
                                  int targetEdgeId,
                                  TopoDS_Shape& outSolid,
                                  double linearDeflection,
                                  double angularDeflection) {
    if (solid.IsNull() || radius <= 0.0) {
        MeshPayload err; err.success = false; err.errorMessage = "Invalid solid or non-positive fillet radius.";
        return err;
    }

    try {
        BRepFilletAPI_MakeFillet mkFillet(solid);
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(solid, TopAbs_EDGE, edgeMap);

        if (edgeMap.IsEmpty()) {
            MeshPayload err; err.success = false; err.errorMessage = "Solid contains no edges.";
            return err;
        }

        if (targetEdgeId > 0 && targetEdgeId <= edgeMap.Extent()) {
            mkFillet.Add(radius, TopoDS::Edge(edgeMap(targetEdgeId)));
        } else {
            // Apply to all valid edges
            for (int i = 1; i <= edgeMap.Extent(); ++i) {
                mkFillet.Add(radius, TopoDS::Edge(edgeMap(i)));
            }
        }

        mkFillet.Build();
        if (!mkFillet.IsDone()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepFilletAPI_MakeFillet failed to build fillet.";
            return err;
        }

        outSolid = mkFillet.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Fillet Standard_Failure: ") + e.GetMessageString();
        return err;
    }
}

MeshPayload FeatureOps::MakeChamfer(const TopoDS_Shape& solid,
                                   double distance,
                                   int targetEdgeId,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection,
                                   double angularDeflection) {
    if (solid.IsNull() || distance <= 0.0) {
        MeshPayload err; err.success = false; err.errorMessage = "Invalid solid or non-positive chamfer distance.";
        return err;
    }

    try {
        BRepFilletAPI_MakeChamfer mkChamfer(solid);
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(solid, TopAbs_EDGE, edgeMap);

        if (edgeMap.IsEmpty()) {
            MeshPayload err; err.success = false; err.errorMessage = "Solid contains no edges.";
            return err;
        }

        int edgeIdx = (targetEdgeId > 0 && targetEdgeId <= edgeMap.Extent()) ? targetEdgeId : 1;
        const TopoDS_Edge& edge = TopoDS::Edge(edgeMap(edgeIdx));
        mkChamfer.Add(distance, edge);

        mkChamfer.Build();
        if (!mkChamfer.IsDone()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepFilletAPI_MakeChamfer failed to build chamfer.";
            return err;
        }

        outSolid = mkChamfer.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Chamfer Standard_Failure: ") + e.GetMessageString();
        return err;
    }
}

MeshPayload FeatureOps::MakeHole(const TopoDS_Shape& solid,
                                double px, double py, double pz,
                                double dx, double dy, double dz,
                                double radius, double depth,
                                TopoDS_Shape& outSolid,
                                double linearDeflection,
                                double angularDeflection) {
    if (solid.IsNull() || radius <= 0.0 || depth <= 0.0) {
        MeshPayload err; err.success = false; err.errorMessage = "Invalid parameters for hole creation.";
        return err;
    }

    try {
        gp_Dir dir(dx, dy, dz);
        gp_Pnt pnt(px - dir.X() * 0.1, py - dir.Y() * 0.1, pz - dir.Z() * 0.1);
        gp_Ax2 axis(pnt, dir);

        BRepPrimAPI_MakeCylinder cylinder(axis, radius, depth + 0.2);
        cylinder.Build();
        if (!cylinder.IsDone()) {
            MeshPayload err; err.success = false; err.errorMessage = "Failed to construct tool cylinder for hole.";
            return err;
        }

        BRepAlgoAPI_Cut cutter(solid, cylinder.Shape());
        cutter.SetFuzzyValue(1e-5);
        cutter.Build();

        if (!cutter.IsDone()) {
            MeshPayload err; err.success = false; err.errorMessage = "Boolean Cut failed for hole operation.";
            return err;
        }

        outSolid = cutter.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Hole Standard_Failure: ") + e.GetMessageString();
        return err;
    }
}

MeshPayload FeatureOps::MakeShell(const TopoDS_Shape& solid,
                                 double thickness,
                                 int removeFaceId,
                                 TopoDS_Shape& outSolid,
                                 double linearDeflection,
                                 double angularDeflection) {
    if (solid.IsNull() || thickness <= 0.0) {
        MeshPayload err; err.success = false; err.errorMessage = "Invalid solid or non-positive wall thickness.";
        return err;
    }

    try {
        TopTools_ListOfShape facesToRemove;
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(solid, TopAbs_FACE, faceMap);

        if (removeFaceId > 0 && removeFaceId <= faceMap.Extent()) {
            facesToRemove.Append(faceMap(removeFaceId));
        } else if (!faceMap.IsEmpty()) {
            facesToRemove.Append(faceMap(1));
        }

        BRepOffsetAPI_MakeThickSolid thickSolid;
        thickSolid.MakeThickSolidByJoin(solid, facesToRemove, -thickness, 1e-3);
        thickSolid.Build();

        if (!thickSolid.IsDone()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepOffsetAPI_MakeThickSolid failed to hollow solid.";
            return err;
        }

        outSolid = thickSolid.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Shell Standard_Failure: ") + e.GetMessageString();
        return err;
    }
}

MeshPayload FeatureOps::MakeBoolean(const TopoDS_Shape& solidA,
                                   const TopoDS_Shape& solidB,
                                   BooleanType type,
                                   double fuzzyValue,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection,
                                   double angularDeflection) {
    if (solidA.IsNull() || solidB.IsNull()) {
        MeshPayload err; err.success = false; err.errorMessage = "Input solids for Boolean operation cannot be null.";
        return err;
    }

    try {
        if (type == BooleanType::Join) {
            BRepAlgoAPI_Fuse bop(solidA, solidB);
            bop.SetFuzzyValue(fuzzyValue);
            bop.Build();
            if (!bop.IsDone()) {
                MeshPayload err; err.success = false; err.errorMessage = "BRepAlgoAPI_Fuse failed.";
                return err;
            }
            outSolid = bop.Shape();
        } else if (type == BooleanType::Cut) {
            BRepAlgoAPI_Cut bop(solidA, solidB);
            bop.SetFuzzyValue(fuzzyValue);
            bop.Build();
            if (!bop.IsDone()) {
                MeshPayload err; err.success = false; err.errorMessage = "BRepAlgoAPI_Cut failed.";
                return err;
            }
            outSolid = bop.Shape();
        } else {
            BRepAlgoAPI_Common bop(solidA, solidB);
            bop.SetFuzzyValue(fuzzyValue);
            bop.Build();
            if (!bop.IsDone()) {
                MeshPayload err; err.success = false; err.errorMessage = "BRepAlgoAPI_Common failed.";
                return err;
            }
            outSolid = bop.Shape();
        }

        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Boolean Standard_Failure: ") + e.GetMessageString();
        return err;
    }
}

} // namespace androcad
