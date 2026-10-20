// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_FEATURE_OPS_H
#define ANDROCAD_FEATURE_OPS_H

#include "AndroCadKernel.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <string>
#include <vector>

namespace androcad {

enum class BooleanType {
    Join = 0,     // Fuse / Union (A U B)
    Cut = 1,      // Difference (A \ B)
    Intersect = 2 // Common (A n B)
};

class FeatureOps {
public:
    // Converts any valid OCCT solid/compound shape into a GPU-ready MeshPayload
    static MeshPayload ShapeToMeshPayload(const TopoDS_Shape& shape,
                                          double linearDeflection = 0.1,
                                          double angularDeflection = 0.5);

    // 1. Extrude: extrudes a 2D profile face along a vector (dx, dy, dz)
    static MeshPayload MakeExtrude(const TopoDS_Face& profileFace,
                                   double dx, double dy, double dz,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection = 0.1,
                                   double angularDeflection = 0.5);

    // 2. Revolve: revolves a 2D profile face around an axis by angle in radians
    static MeshPayload MakeRevolve(const TopoDS_Face& profileFace,
                                   double ax, double ay, double az,
                                   double dx, double dy, double dz,
                                   double angleRad,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection = 0.1,
                                   double angularDeflection = 0.5);

    // 3. Fillet: rounds edge(s) of a solid by a constant radius
    static MeshPayload MakeFillet(const TopoDS_Shape& solid,
                                  double radius,
                                  int targetEdgeId, // 1-based edgeId, or -1 for all boundary edges
                                  TopoDS_Shape& outSolid,
                                  double linearDeflection = 0.1,
                                  double angularDeflection = 0.5);

    // 4. Chamfer: creates a bevel along edge(s) of a solid by a distance
    static MeshPayload MakeChamfer(const TopoDS_Shape& solid,
                                   double distance,
                                   int targetEdgeId, // 1-based edgeId, or -1 for all boundary edges
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection = 0.1,
                                   double angularDeflection = 0.5);

    // 5. Hole: drills a cylindrical hole through a solid along a direction
    static MeshPayload MakeHole(const TopoDS_Shape& solid,
                                double px, double py, double pz,
                                double dx, double dy, double dz,
                                double radius, double depth,
                                TopoDS_Shape& outSolid,
                                double linearDeflection = 0.1,
                                double angularDeflection = 0.5);

    // 6. Shell (Hollow): hollows a solid with a specified wall thickness
    static MeshPayload MakeShell(const TopoDS_Shape& solid,
                                 double thickness,
                                 int removeFaceId, // 1-based faceId to remove for opening, or -1
                                 TopoDS_Shape& outSolid,
                                 double linearDeflection = 0.1,
                                 double angularDeflection = 0.5);

    // 7. Boolean Combine: Join (Fuse), Cut, or Intersect between two solids
    static MeshPayload MakeBoolean(const TopoDS_Shape& solidA,
                                   const TopoDS_Shape& solidB,
                                   BooleanType type,
                                   double fuzzyValue,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection = 0.1,
                                   double angularDeflection = 0.5);
};

} // namespace androcad

#endif // ANDROCAD_FEATURE_OPS_H
