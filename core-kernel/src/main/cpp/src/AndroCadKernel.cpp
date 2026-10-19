// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "AndroCadKernel.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Standard_Failure.hxx>

#include <cmath>
#include <iostream>

namespace androcad {

MeshPayload AndroCadKernel::CreateBox(double dx, double dy, double dz,
                                      double linearDeflection, 
                                      double angularDeflection) {
    MeshPayload result;
    result.volume = 0.0;
    result.faceCount = 0;
    result.triangleCount = 0;
    result.success = false;

    if (dx <= 0.0 || dy <= 0.0 || dz <= 0.0) {
        result.errorMessage = "Box dimensions must be strictly positive.";
        return result;
    }

    try {
        // 1. Construct solid box via OCCT BRepPrimAPI
        BRepPrimAPI_MakeBox boxMaker(dx, dy, dz);
        boxMaker.Build();
        if (!boxMaker.IsDone()) {
            result.errorMessage = "BRepPrimAPI_MakeBox failed to construct solid.";
            return result;
        }
        TopoDS_Shape boxShape = boxMaker.Shape();

        // 2. Measure exact analytical volume
        GProp_GProps system;
        BRepGProp::VolumeProperties(boxShape, system);
        result.volume = system.Mass();

        // 3. Tessellate boundary representation using BRepMesh
        BRepMesh_IncrementalMesh mesher(boxShape, linearDeflection, Standard_False, angularDeflection, Standard_True);
        mesher.Perform();
        if (!mesher.IsDone()) {
            result.errorMessage = "BRepMesh_IncrementalMesh failed to tessellate shape.";
            return result;
        }

        // 4. Extract triangulation per face with normal vectors and distinct face IDs
        uint32_t currentFaceId = 1;
        uint32_t vertexOffset = 0;

        for (TopExp_Explorer faceExp(boxShape, TopAbs_FACE); faceExp.More(); faceExp.Next(), ++currentFaceId) {
            const TopoDS_Face& face = TopoDS::Face(faceExp.Current());
            TopLoc_Location location;
            Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, location);

            if (tri.IsNull() || tri->NbTriangles() == 0) {
                continue;
            }

            const gp_Trsf& trsf = location.Transformation();
            bool isReversed = (face.Orientation() == TopAbs_REVERSED);

            // Extract vertices for this face
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

            // Extract triangles and compute face normals
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

                // Compute face normal for flat shading / smooth accumulation
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

            // Normalize node normals
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

} // namespace androcad
