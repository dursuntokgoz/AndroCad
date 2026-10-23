// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "SurfaceOps.h"
#include "FeatureOps.h"

#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Geom_Plane.hxx>
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Standard_Failure.hxx>

#include <stdexcept>
#include <cmath>

namespace androcad {

TopoDS_Wire SurfaceOps::CreateCircleWire(double cx, double cy, double cz,
                                         double radius,
                                         double nx, double ny, double nz) {
    if (radius <= 0.0) {
        throw std::invalid_argument("Radius must be positive");
    }
    gp_Dir dir(nx, ny, nz);
    gp_Ax2 axes(gp_Pnt(cx, cy, cz), dir);
    gp_Circ circle(axes, radius);

    BRepBuilderAPI_MakeEdge edgeMaker(circle);
    if (!edgeMaker.IsDone()) {
        throw std::runtime_error("Failed to construct circular edge");
    }
    BRepBuilderAPI_MakeWire wireMaker(edgeMaker.Edge());
    if (!wireMaker.IsDone()) {
        throw std::runtime_error("Failed to construct circular wire");
    }
    return wireMaker.Wire();
}

TopoDS_Wire SurfaceOps::CreateRectangleWire(double minX, double minY, double z,
                                            double width, double height) {
    if (width <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("Width and height must be positive");
    }
    gp_Pnt p1(minX, minY, z);
    gp_Pnt p2(minX + width, minY, z);
    gp_Pnt p3(minX + width, minY + height, z);
    gp_Pnt p4(minX, minY + height, z);

    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2).Edge();
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3).Edge();
    TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4).Edge();
    TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(p4, p1).Edge();

    BRepBuilderAPI_MakeWire wireMaker;
    wireMaker.Add(e1);
    wireMaker.Add(e2);
    wireMaker.Add(e3);
    wireMaker.Add(e4);

    if (!wireMaker.IsDone()) {
        throw std::runtime_error("Failed to construct rectangular wire");
    }
    return wireMaker.Wire();
}

MeshPayload SurfaceOps::MakeLoft(const std::vector<TopoDS_Wire>& sections,
                                 bool isSolid,
                                 bool isRuled,
                                 TopoDS_Shape& outShape,
                                 double linearDeflection,
                                 double angularDeflection) {
    if (sections.size() < 2) {
        throw std::invalid_argument("Loft operation requires at least 2 cross-section wires");
    }

    try {
        BRepOffsetAPI_ThruSections loftMaker(isSolid ? Standard_True : Standard_False,
                                             isRuled ? Standard_True : Standard_False);

        for (const auto& w : sections) {
            loftMaker.AddWire(w);
        }

        loftMaker.Build();
        if (!loftMaker.IsDone()) {
            throw std::runtime_error("BRepOffsetAPI_ThruSections failed to compute geometry");
        }

        outShape = loftMaker.Shape();
        return FeatureOps::ShapeToMeshPayload(outShape, linearDeflection, angularDeflection);
    } catch (Standard_Failure& e) {
        throw std::runtime_error(std::string("OCCT Loft error: ") + e.GetMessageString());
    }
}

MeshPayload SurfaceOps::MakeSweep(const TopoDS_Wire& spinePath,
                                  const TopoDS_Wire& profileSection,
                                  bool isSolid,
                                  TopoDS_Shape& outShape,
                                  double linearDeflection,
                                  double angularDeflection) {
    try {
        TopoDS_Shape profileShape = profileSection;
        if (isSolid) {
            BRepBuilderAPI_MakeFace faceMaker(profileSection);
            if (faceMaker.IsDone()) {
                profileShape = faceMaker.Face();
            }
        }

        BRepOffsetAPI_MakePipe pipeMaker(spinePath, profileShape);
        pipeMaker.Build();

        if (!pipeMaker.IsDone()) {
            throw std::runtime_error("BRepOffsetAPI_MakePipe failed to compute sweep");
        }

        outShape = pipeMaker.Shape();
        return FeatureOps::ShapeToMeshPayload(outShape, linearDeflection, angularDeflection);
    } catch (Standard_Failure& e) {
        throw std::runtime_error(std::string("OCCT Sweep error: ") + e.GetMessageString());
    }
}

MeshPayload SurfaceOps::MakeThicken(const TopoDS_Shape& surfaceOrShell,
                                    double thickness,
                                    TopoDS_Shape& outSolid,
                                    double linearDeflection,
                                    double angularDeflection) {
    if (std::abs(thickness) < 1e-6) {
        throw std::invalid_argument("Thickness must be non-zero");
    }

    try {
        TopExp_Explorer faceExp(surfaceOrShell, TopAbs_FACE);
        if (!faceExp.More()) {
            throw std::runtime_error("No face found in surface to thicken");
        }

        int faceCount = 0;
        int topFaceId = 1;
        double bestZ = -1e18;
        for (TopExp_Explorer exp(surfaceOrShell, TopAbs_FACE); exp.More(); exp.Next()) {
            ++faceCount;
            TopoDS_Face f = TopoDS::Face(exp.Current());
            BRepAdaptor_Surface surf(f);
            if (surf.GetType() == GeomAbs_Plane) {
                gp_Dir n = surf.Plane().Axis().Direction();
                if (f.Orientation() == TopAbs_REVERSED) n.Reverse();
                if (n.Z() > bestZ) {
                    bestZ = n.Z();
                    topFaceId = faceCount;
                }
            }
        }

        if (faceCount > 1) {
            return PressPullFace(surfaceOrShell, topFaceId, thickness, outSolid, linearDeflection, angularDeflection);
        }

        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        BRepAdaptor_Surface adaptor(face);

        gp_Dir normal(0.0, 0.0, 1.0);
        if (adaptor.GetType() == GeomAbs_Plane) {
            normal = adaptor.Plane().Axis().Direction();
            if (face.Orientation() == TopAbs_REVERSED) {
                normal.Reverse();
            }
        }

        gp_Vec extrudeVec(normal.X() * thickness, normal.Y() * thickness, normal.Z() * thickness);
        BRepPrimAPI_MakePrism prismMaker(face, extrudeVec);
        prismMaker.Build();

        if (!prismMaker.IsDone()) {
            throw std::runtime_error("BRepPrimAPI_MakePrism failed in Thicken");
        }

        outSolid = prismMaker.Shape();
        return FeatureOps::ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (Standard_Failure& e) {
        throw std::runtime_error(std::string("OCCT Thicken error: ") + e.GetMessageString());
    }
}

MeshPayload SurfaceOps::SewSurfaces(const std::vector<TopoDS_Shape>& surfaceSheets,
                                    double tolerance,
                                    bool tryMakeSolid,
                                    TopoDS_Shape& outShape,
                                    double linearDeflection,
                                    double angularDeflection) {
    if (surfaceSheets.empty()) {
        throw std::invalid_argument("At least one surface sheet is required for sewing");
    }

    try {
        BRepBuilderAPI_Sewing sewer(tolerance);
        for (const auto& sheet : surfaceSheets) {
            sewer.Add(sheet);
        }
        sewer.Perform();

        TopoDS_Shape sewed = sewer.SewedShape();

        if (tryMakeSolid) {
            TopExp_Explorer shellExp(sewed, TopAbs_SHELL);
            if (shellExp.More()) {
                TopoDS_Shell shell = TopoDS::Shell(shellExp.Current());
                BRepBuilderAPI_MakeSolid solidMaker(shell);
                if (solidMaker.IsDone()) {
                    sewed = solidMaker.Solid();
                }
            }
        }

        outShape = sewed;
        return FeatureOps::ShapeToMeshPayload(outShape, linearDeflection, angularDeflection);
    } catch (Standard_Failure& e) {
        throw std::runtime_error(std::string("OCCT Sewing error: ") + e.GetMessageString());
    }
}

MeshPayload SurfaceOps::PressPullFace(const TopoDS_Shape& solid,
                                      int targetFaceId,
                                      double offsetDistance,
                                      TopoDS_Shape& outSolid,
                                      double linearDeflection,
                                      double angularDeflection) {
    if (std::abs(offsetDistance) < 1e-6) {
        outSolid = solid;
        return FeatureOps::ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    }

    try {
        // Find target face by 1-based index
        TopoDS_Face targetFace;
        int currentId = 1;
        for (TopExp_Explorer exp(solid, TopAbs_FACE); exp.More(); exp.Next(), ++currentId) {
            if (currentId == targetFaceId) {
                targetFace = TopoDS::Face(exp.Current());
                break;
            }
        }

        if (targetFace.IsNull()) {
            throw std::runtime_error("Target face ID not found for Press-Pull");
        }

        // Determine face plane and normal
        BRepAdaptor_Surface adaptor(targetFace);
        if (adaptor.GetType() != GeomAbs_Plane) {
            throw std::runtime_error("Press-Pull currently supports planar faces");
        }

        gp_Pln pln = adaptor.Plane();
        gp_Dir normal = pln.Axis().Direction();
        if (targetFace.Orientation() == TopAbs_REVERSED) {
            normal.Reverse();
        }

        // Extrude a tool prism along face normal (or inward if offsetDistance < 0)
        if (offsetDistance > 0.0) {
            gp_Vec extrudeVec(normal.X() * offsetDistance,
                              normal.Y() * offsetDistance,
                              normal.Z() * offsetDistance);
            BRepPrimAPI_MakePrism prismMaker(targetFace, extrudeVec);
            prismMaker.Build();
            if (!prismMaker.IsDone()) {
                throw std::runtime_error("Failed to construct prism for Press-Pull offset");
            }
            TopoDS_Shape toolPrism = prismMaker.Shape();

            // Join (Fuse) to extend material outwards
            BRepAlgoAPI_Fuse fuse(solid, toolPrism);
            fuse.SetFuzzyValue(1e-5);
            fuse.Build();
            if (!fuse.IsDone()) {
                throw std::runtime_error("Boolean Fuse failed in Press-Pull expansion");
            }
            outSolid = fuse.Shape();
        } else {
            double depth = std::abs(offsetDistance);
            gp_Vec extrudeVec(-normal.X() * depth,
                              -normal.Y() * depth,
                              -normal.Z() * depth);
            BRepPrimAPI_MakePrism prismMaker(targetFace, extrudeVec);
            prismMaker.Build();
            if (!prismMaker.IsDone()) {
                throw std::runtime_error("Failed to construct inward prism for Press-Pull cut");
            }
            TopoDS_Shape toolPrism = prismMaker.Shape();

            // Cut into body to remove material inwards
            BRepAlgoAPI_Cut cut(solid, toolPrism);
            cut.SetFuzzyValue(1e-5);
            cut.Build();
            if (!cut.IsDone()) {
                throw std::runtime_error("Boolean Cut failed in Press-Pull contraction");
            }
            outSolid = cut.Shape();
        }

        return FeatureOps::ShapeToMeshPayload(outSolid, linearDeflection, angularDeflection);
    } catch (Standard_Failure& e) {
        throw std::runtime_error(std::string("OCCT PressPull error: ") + e.GetMessageString());
    }
}

} // namespace androcad
