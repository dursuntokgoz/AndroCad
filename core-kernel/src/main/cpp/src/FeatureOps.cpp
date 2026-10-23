// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "FeatureOps.h"

#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
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
#include <exception>

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
    } catch (...) {
        result.errorMessage = "Unknown native error during triangulation.";
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
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Extrude std::exception: ") + e.what();
        return err;
    } catch (...) {
        MeshPayload err; err.success = false; err.errorMessage = "Extrude failed with unknown native error.";
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

        if (!revol.IsDone() || revol.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepPrimAPI_MakeRevol failed to revolve face.";
            return err;
        }

        outSolid = revol.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Revolve Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Revolve std::exception: ") + e.what();
        return err;
    } catch (...) {
        MeshPayload err; err.success = false; err.errorMessage = "Revolve failed with unknown native error.";
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
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(solid, TopAbs_EDGE, edgeMap);

        if (edgeMap.IsEmpty()) {
            MeshPayload err; err.success = false; err.errorMessage = "Solid contains no edges.";
            return err;
        }

        BRepFilletAPI_MakeFillet mkFillet(solid);

        if (targetEdgeId > 0 && targetEdgeId <= edgeMap.Extent()) {
            const TopoDS_Edge& edge = TopoDS::Edge(edgeMap(targetEdgeId));
            if (!edge.IsNull()) {
                mkFillet.Add(radius, edge);
            }
        } else {
            // Apply to all valid edges
            for (int i = 1; i <= edgeMap.Extent(); ++i) {
                const TopoDS_Edge& edge = TopoDS::Edge(edgeMap(i));
                if (!edge.IsNull()) {
                    mkFillet.Add(radius, edge);
                }
            }
        }

        mkFillet.Build();
        if (!mkFillet.IsDone() || mkFillet.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepFilletAPI_MakeFillet failed to build fillet.";
            return err;
        }

        outSolid = mkFillet.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Fillet Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Fillet std::exception: ") + e.what();
        return err;
    } catch (...) {
        MeshPayload err; err.success = false; err.errorMessage = "Fillet failed with unknown native error.";
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
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(solid, TopAbs_EDGE, edgeMap);

        if (edgeMap.IsEmpty()) {
            MeshPayload err; err.success = false; err.errorMessage = "Solid contains no edges.";
            return err;
        }

        int edgeIdx = (targetEdgeId > 0 && targetEdgeId <= edgeMap.Extent()) ? targetEdgeId : 1;
        const TopoDS_Edge& edge = TopoDS::Edge(edgeMap(edgeIdx));
        if (edge.IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "Selected edge is null.";
            return err;
        }

        BRepFilletAPI_MakeChamfer mkChamfer(solid);

        TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
        TopExp::MapShapesAndAncestors(solid, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);
        if (edgeFaceMap.Contains(edge) && !edgeFaceMap.FindFromKey(edge).IsEmpty()) {
            const TopoDS_Face& face = TopoDS::Face(edgeFaceMap.FindFromKey(edge).First());
            if (!face.IsNull()) {
                mkChamfer.Add(distance, distance, edge, face);
            } else {
                mkChamfer.Add(distance, edge);
            }
        } else {
            mkChamfer.Add(distance, edge);
        }

        mkChamfer.Build();
        if (!mkChamfer.IsDone() || mkChamfer.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepFilletAPI_MakeChamfer failed to build chamfer.";
            return err;
        }

        outSolid = mkChamfer.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Chamfer Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Chamfer std::exception: ") + e.what();
        return err;
    } catch (...) {
        MeshPayload err; err.success = false; err.errorMessage = "Chamfer failed with unknown native error.";
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
        if (!cylinder.IsDone() || cylinder.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "Failed to construct tool cylinder for hole.";
            return err;
        }

        BRepAlgoAPI_Cut cutter(solid, cylinder.Shape());
        cutter.SetFuzzyValue(1e-5);
        cutter.Build();

        if (!cutter.IsDone() || cutter.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "Boolean Cut failed for hole operation.";
            return err;
        }

        outSolid = cutter.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Hole Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Hole std::exception: ") + e.what();
        return err;
    } catch (...) {
        MeshPayload err; err.success = false; err.errorMessage = "Hole failed with unknown native error.";
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

        if (!thickSolid.IsDone() || thickSolid.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepOffsetAPI_MakeThickSolid failed to hollow solid.";
            return err;
        }

        outSolid = thickSolid.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Shell Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Shell std::exception: ") + e.what();
        return err;
    } catch (...) {
        MeshPayload err; err.success = false; err.errorMessage = "Shell failed with unknown native error.";
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
            if (!bop.IsDone() || bop.Shape().IsNull()) {
                MeshPayload err; err.success = false; err.errorMessage = "BRepAlgoAPI_Fuse failed.";
                return err;
            }
            outSolid = bop.Shape();
        } else if (type == BooleanType::Cut) {
            BRepAlgoAPI_Cut bop(solidA, solidB);
            bop.SetFuzzyValue(fuzzyValue);
            bop.Build();
            if (!bop.IsDone() || bop.Shape().IsNull()) {
                MeshPayload err; err.success = false; err.errorMessage = "BRepAlgoAPI_Cut failed.";
                return err;
            }
            outSolid = bop.Shape();
        } else {
            BRepAlgoAPI_Common bop(solidA, solidB);
            bop.SetFuzzyValue(fuzzyValue);
            bop.Build();
            if (!bop.IsDone() || bop.Shape().IsNull()) {
                MeshPayload err; err.success = false; err.errorMessage = "BRepAlgoAPI_Common failed.";
                return err;
            }
            outSolid = bop.Shape();
        }

        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Boolean Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Boolean std::exception: ") + e.what();
        return err;
    } catch (...) {
        MeshPayload err; err.success = false; err.errorMessage = "Boolean failed with unknown native error.";
        return err;
    }
}

MeshPayload FeatureOps::MakeSphere(double cx, double cy, double cz,
                                   double radius,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection,
                                   double angularDeflection) {
    if (radius <= 0.0) {
        MeshPayload err; err.success = false; err.errorMessage = "Sphere radius must be positive.";
        return err;
    }
    try {
        gp_Pnt center(cx, cy, cz);
        BRepPrimAPI_MakeSphere mkSphere(center, radius);
        mkSphere.Build();
        if (!mkSphere.IsDone() || mkSphere.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepPrimAPI_MakeSphere failed.";
            return err;
        }
        outSolid = mkSphere.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Sphere Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Sphere std::exception: ") + e.what();
        return err;
    }
}

MeshPayload FeatureOps::MakeTorus(double cx, double cy, double cz,
                                  double nx, double ny, double nz,
                                  double majorRadius, double minorRadius,
                                  TopoDS_Shape& outSolid,
                                  double linearDeflection,
                                  double angularDeflection) {
    if (majorRadius <= 0.0 || minorRadius <= 0.0 || minorRadius >= majorRadius) {
        MeshPayload err; err.success = false; err.errorMessage = "Torus requires 0 < minorRadius < majorRadius.";
        return err;
    }
    try {
        gp_Ax2 axis(gp_Pnt(cx, cy, cz), gp_Dir(nx, ny, nz));
        BRepPrimAPI_MakeTorus mkTorus(axis, majorRadius, minorRadius);
        mkTorus.Build();
        if (!mkTorus.IsDone() || mkTorus.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepPrimAPI_MakeTorus failed.";
            return err;
        }
        outSolid = mkTorus.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Torus Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Torus std::exception: ") + e.what();
        return err;
    }
}

MeshPayload FeatureOps::MakeCone(double cx, double cy, double cz,
                                 double nx, double ny, double nz,
                                 double bottomRadius, double topRadius, double height,
                                 TopoDS_Shape& outSolid,
                                 double linearDeflection,
                                 double angularDeflection) {
    if (bottomRadius < 0.0 || topRadius < 0.0 || (bottomRadius <= 1e-4 && topRadius <= 1e-4) || height <= 0.0) {
        MeshPayload err; err.success = false; err.errorMessage = "Cone requires positive height and at least one positive radius.";
        return err;
    }
    try {
        gp_Ax2 axis(gp_Pnt(cx, cy, cz), gp_Dir(nx, ny, nz));
        BRepPrimAPI_MakeCone mkCone(axis, bottomRadius, topRadius, height);
        mkCone.Build();
        if (!mkCone.IsDone() || mkCone.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "BRepPrimAPI_MakeCone failed.";
            return err;
        }
        outSolid = mkCone.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Cone Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Cone std::exception: ") + e.what();
        return err;
    }
}

MeshPayload FeatureOps::TransformSolid(const TopoDS_Shape& solid,
                                       double tx, double ty, double tz,
                                       double rxDeg, double ryDeg, double rzDeg,
                                       double scaleFactor,
                                       TopoDS_Shape& outSolid,
                                       double linearDeflection,
                                       double angularDeflection) {
    if (solid.IsNull() || scaleFactor <= 1e-3) {
        MeshPayload err; err.success = false; err.errorMessage = "Invalid solid or non-positive scale factor.";
        return err;
    }
    try {
        Bnd_Box bbox;
        BRepBndLib::Add(solid, bbox);
        Standard_Real xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
        bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        gp_Pnt pivot((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);

        TopoDS_Shape current = solid;
        const double degToRad = M_PI / 180.0;

        if (std::abs(rxDeg) > 1e-4) {
            gp_Trsf rotX;
            rotX.SetRotation(gp_Ax1(pivot, gp_Dir(1.0, 0.0, 0.0)), rxDeg * degToRad);
            current = BRepBuilderAPI_Transform(current, rotX, Standard_True).Shape();
        }
        if (std::abs(ryDeg) > 1e-4) {
            gp_Trsf rotY;
            rotY.SetRotation(gp_Ax1(pivot, gp_Dir(0.0, 1.0, 0.0)), ryDeg * degToRad);
            current = BRepBuilderAPI_Transform(current, rotY, Standard_True).Shape();
        }
        if (std::abs(rzDeg) > 1e-4) {
            gp_Trsf rotZ;
            rotZ.SetRotation(gp_Ax1(pivot, gp_Dir(0.0, 0.0, 1.0)), rzDeg * degToRad);
            current = BRepBuilderAPI_Transform(current, rotZ, Standard_True).Shape();
        }
        if (std::abs(scaleFactor - 1.0) > 1e-4) {
            gp_Trsf scl;
            scl.SetScale(pivot, scaleFactor);
            current = BRepBuilderAPI_Transform(current, scl, Standard_True).Shape();
        }
        if (std::abs(tx) > 1e-4 || std::abs(ty) > 1e-4 || std::abs(tz) > 1e-4) {
            gp_Trsf tr;
            tr.SetTranslation(gp_Vec(tx, ty, tz));
            current = BRepBuilderAPI_Transform(current, tr, Standard_True).Shape();
        }

        outSolid = current;
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Transform Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Transform std::exception: ") + e.what();
        return err;
    }
}

MeshPayload FeatureOps::SplitSolidByPlane(const TopoDS_Shape& solid,
                                          double ox, double oy, double oz,
                                          double nx, double ny, double nz,
                                          TopoDS_Shape& outSolid,
                                          double linearDeflection,
                                          double angularDeflection) {
    if (solid.IsNull()) {
        MeshPayload err; err.success = false; err.errorMessage = "Solid is null.";
        return err;
    }
    try {
        gp_Dir normDir(nx, ny, nz);
        gp_Pnt origin(ox - normDir.X() * 500.0, oy - normDir.Y() * 500.0, oz - normDir.Z() * 500.0);
        // Create a coordinate system whose Z is along normDir, shifted in U and V by -500 so the 1000x1000x500 box covers the half-space
        gp_Ax2 cutAx(gp_Pnt(ox, oy, oz), normDir);
        gp_Dir uDir = cutAx.XDirection();
        gp_Dir vDir = cutAx.YDirection();
        gp_Pnt corner(
            ox - uDir.X() * 500.0 - vDir.X() * 500.0,
            oy - uDir.Y() * 500.0 - vDir.Y() * 500.0,
            oz - uDir.Z() * 500.0 - vDir.Z() * 500.0
        );
        gp_Ax2 boxAx(corner, normDir, uDir);
        BRepPrimAPI_MakeBox halfSpaceBox(boxAx, 1000.0, 1000.0, 1000.0);
        halfSpaceBox.Build();

        BRepAlgoAPI_Cut cutter(solid, halfSpaceBox.Shape());
        cutter.SetFuzzyValue(1e-5);
        cutter.Build();
        if (!cutter.IsDone() || cutter.Shape().IsNull()) {
            MeshPayload err; err.success = false; err.errorMessage = "SplitSolidByPlane Boolean Cut failed.";
            return err;
        }
        outSolid = cutter.Shape();
        return ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (const Standard_Failure& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Split Standard_Failure: ") + e.GetMessageString();
        return err;
    } catch (const std::exception& e) {
        MeshPayload err; err.success = false; err.errorMessage = std::string("Split std::exception: ") + e.what();
        return err;
    }
}

} // namespace androcad
