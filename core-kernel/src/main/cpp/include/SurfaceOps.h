// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_SURFACE_OPS_H
#define ANDROCAD_SURFACE_OPS_H

#include "AndroCadKernel.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>

#include <vector>
#include <string>

namespace androcad {

class SurfaceOps {
public:
    // Helper to generate a planar circular wire at specified 3D origin and normal
    static TopoDS_Wire CreateCircleWire(double cx, double cy, double cz,
                                        double radius,
                                        double nx = 0.0, double ny = 0.0, double nz = 1.0);

    // Helper to generate a planar rectangular wire at specified 3D plane
    static TopoDS_Wire CreateRectangleWire(double minX, double minY, double z,
                                           double width, double height);

    // 1. Loft: generates a smooth transition surface or solid across multi-section profile wires
    static MeshPayload MakeLoft(const std::vector<TopoDS_Wire>& sections,
                                bool isSolid,
                                bool isRuled,
                                TopoDS_Shape& outShape,
                                double linearDeflection = 0.1,
                                double angularDeflection = 0.5);

    // 2. Sweep / Pipe: sweeps a 2D profile wire along a 3D spine wire trajectory
    static MeshPayload MakeSweep(const TopoDS_Wire& spinePath,
                                 const TopoDS_Wire& profileSection,
                                 bool isSolid,
                                 TopoDS_Shape& outShape,
                                 double linearDeflection = 0.1,
                                 double angularDeflection = 0.5);

    // 3. Thicken: offsets an open surface / shell to generate a solid with specified thickness
    static MeshPayload MakeThicken(const TopoDS_Shape& surfaceOrShell,
                                   double thickness,
                                   TopoDS_Shape& outSolid,
                                   double linearDeflection = 0.1,
                                   double angularDeflection = 0.5);

    // 4. Sewing: knits adjoining open surface sheets within tolerance into a shell / watertight solid
    static MeshPayload SewSurfaces(const std::vector<TopoDS_Shape>& surfaceSheets,
                                   double tolerance,
                                   bool tryMakeSolid,
                                   TopoDS_Shape& outShape,
                                   double linearDeflection = 0.1,
                                   double angularDeflection = 0.5);

    // 5. Press-Pull / Face Offset: direct modeling offset on selected planar face
    static MeshPayload PressPullFace(const TopoDS_Shape& solid,
                                     int targetFaceId,
                                     double offsetDistance,
                                     TopoDS_Shape& outSolid,
                                     double linearDeflection = 0.1,
                                     double angularDeflection = 0.5);
};

} // namespace androcad

#endif // ANDROCAD_SURFACE_OPS_H
