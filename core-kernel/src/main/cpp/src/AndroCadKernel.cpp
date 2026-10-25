// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "AndroCadKernel.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
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
#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Standard_Failure.hxx>

#include <algorithm>
#include <cmath>
#include <iostream>

namespace androcad {

namespace {

// Möller–Trumbore ray-triangle intersection algorithm
bool RayIntersectsTriangle(const Ray& ray,
                           const Vertex& v0, const Vertex& v1, const Vertex& v2,
                           float& outDist, float outPoint[3]) {
    const float EPSILON = 1e-7f;
    float e1x = v1.x - v0.x, e1y = v1.y - v0.y, e1z = v1.z - v0.z;
    float e2x = v2.x - v0.x, e2y = v2.y - v0.y, e2z = v2.z - v0.z;

    float px = ray.dy * e2z - ray.dz * e2y;
    float py = ray.dz * e2x - ray.dx * e2z;
    float pz = ray.dx * e2y - ray.dy * e2x;

    float det = e1x * px + e1y * py + e1z * pz;
    if (std::fabs(det) < EPSILON) return false;

    float invDet = 1.0f / det;
    float tx = ray.ox - v0.x, ty = ray.oy - v0.y, tz = ray.oz - v0.z;

    float u = (tx * px + ty * py + tz * pz) * invDet;
    if (u < 0.0f || u > 1.0f) return false;

    float qx = ty * e1z - tz * e1y;
    float qy = tz * e1x - tx * e1z;
    float qz = tx * e1y - ty * e1x;

    float v = (ray.dx * qx + ray.dy * qy + ray.dz * qz) * invDet;
    if (v < 0.0f || (u + v) > 1.0f) return false;

    float t = (e2x * qx + e2y * qy + e2z * qz) * invDet;
    if (t > 1e-4f) {
        outDist = t;
        outPoint[0] = ray.ox + t * ray.dx;
        outPoint[1] = ray.oy + t * ray.dy;
        outPoint[2] = ray.oz + t * ray.dz;
        return true;
    }
    return false;
}

// Distance between Ray R(t) = P + t*D (t >= 0) and Segment S(s) = A + s*(B - A) (0 <= s <= 1)
bool RayIntersectsSegment(const Ray& ray,
                          const EdgeVertex& v0, const EdgeVertex& v1,
                          float tolerance, float& outDist, float outPoint[3]) {
    float vx = v1.x - v0.x, vy = v1.y - v0.y, vz = v1.z - v0.z;
    float w0x = ray.ox - v0.x, w0y = ray.oy - v0.y, w0z = ray.oz - v0.z;

    float b = ray.dx * vx + ray.dy * vy + ray.dz * vz;
    float c = vx * vx + vy * vy + vz * vz;
    float d = ray.dx * w0x + ray.dy * w0y + ray.dz * w0z;
    float e = vx * w0x + vy * w0y + vz * w0z;

    if (c < 1e-8f) return false; // Degenerate segment

    float denom = c - b * b;
    float s = 0.0f;
    float t = 0.0f;

    if (denom < 1e-7f * c) {
        // Parallel: pick midpoint
        s = 0.5f;
        t = std::max(0.0f, s * b - d);
    } else {
        s = (e - b * d) / denom;
        s = std::max(0.0f, std::min(1.0f, s));
        t = s * b - d;
        t = std::max(0.0f, t);
        // Re-evaluate segment parameter with clamped t
        s = (e + t * b) / c;
        s = std::max(0.0f, std::min(1.0f, s));
    }

    float rx = ray.ox + t * ray.dx;
    float ry = ray.oy + t * ray.dy;
    float rz = ray.oz + t * ray.dz;

    float sx = v0.x + s * vx;
    float sy = v0.y + s * vy;
    float sz = v0.z + s * vz;

    float distSq = (rx - sx) * (rx - sx) + (ry - sy) * (ry - sy) + (rz - sz) * (rz - sz);
    if (distSq <= tolerance * tolerance) {
        outDist = t;
        outPoint[0] = sx;
        outPoint[1] = sy;
        outPoint[2] = sz;
        return true;
    }
    return false;
}

} // namespace

MeshPayload AndroCadKernel::CreateBox(double dx, double dy, double dz,
                                      double linearDeflection, 
                                      double angularDeflection) {
    MeshPayload result;
    result.volume = 0.0;
    result.faceCount = 0;
    result.edgeCount = 0;
    result.triangleCount = 0;
    result.bounds = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
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

        // 3. Compute Bounding Box
        Bnd_Box aBox;
        BRepBndLib::Add(boxShape, aBox);
        Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
        aBox.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
        result.bounds.minX = static_cast<float>(aXmin);
        result.bounds.minY = static_cast<float>(aYmin);
        result.bounds.minZ = static_cast<float>(aZmin);
        result.bounds.maxX = static_cast<float>(aXmax);
        result.bounds.maxY = static_cast<float>(aYmax);
        result.bounds.maxZ = static_cast<float>(aZmax);

        // 4. Extract unique edges and discretize for wireframe & edge display
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(boxShape, TopAbs_EDGE, edgeMap);
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

        // 5. Tessellate boundary representation using BRepMesh
        BRepMesh_IncrementalMesh mesher(boxShape, linearDeflection, Standard_False, angularDeflection, Standard_True);
        mesher.Perform();
        if (!mesher.IsDone()) {
            result.errorMessage = "BRepMesh_IncrementalMesh failed to tessellate shape.";
            return result;
        }

        // 6. Extract triangulation per face with normal vectors and distinct face IDs
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

                // Compute normal for flat shading / smooth accumulation
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

PickResult AndroCadKernel::PickEntityByRay(const MeshPayload& payload, const Ray& ray, float edgeTolerance) {
    PickResult result;
    result.hit = false;
    result.entityId = 0;
    result.isEdge = false;
    result.distance = 1e9f;
    result.hitPoint[0] = 0.0f;
    result.hitPoint[1] = 0.0f;
    result.hitPoint[2] = 0.0f;
    result.normal[0] = 0.0f;
    result.normal[1] = 0.0f;
    result.normal[2] = 1.0f;

    if (!payload.success) return result;

    // 1. Test against edges (segments)
    float closestEdgeDist = 1e9f;
    uint32_t closestEdgeId = 0;
    float closestEdgePoint[3] = {0.0f, 0.0f, 0.0f};

    for (size_t i = 0; i + 1 < payload.edgeVertices.size(); i += 2) {
        const EdgeVertex& ev0 = payload.edgeVertices[i];
        const EdgeVertex& ev1 = payload.edgeVertices[i + 1];

        float dist = 0.0f;
        float pt[3] = {0.0f, 0.0f, 0.0f};
        if (RayIntersectsSegment(ray, ev0, ev1, edgeTolerance, dist, pt)) {
            if (dist < closestEdgeDist) {
                closestEdgeDist = dist;
                closestEdgeId = ev0.edgeId;
                closestEdgePoint[0] = pt[0];
                closestEdgePoint[1] = pt[1];
                closestEdgePoint[2] = pt[2];
            }
        }
    }

    // 2. Test against face triangles
    float closestFaceDist = 1e9f;
    uint32_t closestFaceId = 0;
    float closestFacePoint[3] = {0.0f, 0.0f, 0.0f};
    float closestFaceNormal[3] = {0.0f, 0.0f, 1.0f};

    for (size_t i = 0; i + 2 < payload.indices.size(); i += 3) {
        uint32_t idx0 = payload.indices[i];
        uint32_t idx1 = payload.indices[i + 1];
        uint32_t idx2 = payload.indices[i + 2];

        if (idx0 >= payload.vertices.size() || idx1 >= payload.vertices.size() || idx2 >= payload.vertices.size()) {
            continue;
        }

        const Vertex& v0 = payload.vertices[idx0];
        const Vertex& v1 = payload.vertices[idx1];
        const Vertex& v2 = payload.vertices[idx2];

        float dist = 0.0f;
        float pt[3] = {0.0f, 0.0f, 0.0f};
        if (RayIntersectsTriangle(ray, v0, v1, v2, dist, pt)) {
            if (dist < closestFaceDist) {
                closestFaceDist = dist;
                closestFaceId = v0.faceId;
                closestFacePoint[0] = pt[0];
                closestFacePoint[1] = pt[1];
                closestFacePoint[2] = pt[2];

                float ax = v1.x - v0.x, ay = v1.y - v0.y, az = v1.z - v0.z;
                float bx = v2.x - v0.x, by = v2.y - v0.y, bz = v2.z - v0.z;
                float nx = ay * bz - az * by;
                float ny = az * bx - ax * bz;
                float nz = ax * by - ay * bx;
                float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                if (len > 1e-6f) {
                    nx /= len;
                    ny /= len;
                    nz /= len;
                } else {
                    nx = v0.nx;
                    ny = v0.ny;
                    nz = v0.nz;
                }
                // Ensure outward surface normal opposes the incoming ray direction
                if (nx * ray.dx + ny * ray.dy + nz * ray.dz > 0.0f) {
                    nx = -nx;
                    ny = -ny;
                    nz = -nz;
                }
                closestFaceNormal[0] = nx;
                closestFaceNormal[1] = ny;
                closestFaceNormal[2] = nz;
            }
        }
    }

    // 3. Occlusion-aware priority: An edge can only be selected if it is NOT occluded behind a front face
    bool edgeInFrontOrAtFace = (closestEdgeDist <= closestFaceDist + 0.05f);
    if (closestEdgeId != 0 && edgeInFrontOrAtFace) {
        result.hit = true;
        result.entityId = closestEdgeId;
        result.isEdge = true;
        result.distance = closestEdgeDist;
        result.hitPoint[0] = closestEdgePoint[0];
        result.hitPoint[1] = closestEdgePoint[1];
        result.hitPoint[2] = closestEdgePoint[2];
        if (closestFaceId != 0) {
            result.normal[0] = closestFaceNormal[0];
            result.normal[1] = closestFaceNormal[1];
            result.normal[2] = closestFaceNormal[2];
        } else {
            result.normal[0] = 0.0f;
            result.normal[1] = 0.0f;
            result.normal[2] = 1.0f;
        }
        return result;
    }

    if (closestFaceId != 0) {
        result.hit = true;
        result.entityId = closestFaceId;
        result.isEdge = false;
        result.distance = closestFaceDist;
        result.hitPoint[0] = closestFacePoint[0];
        result.hitPoint[1] = closestFacePoint[1];
        result.hitPoint[2] = closestFacePoint[2];
        result.normal[0] = closestFaceNormal[0];
        result.normal[1] = closestFaceNormal[1];
        result.normal[2] = closestFaceNormal[2];
        return result;
    }

    return result;
}

} // namespace androcad
