// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "SheetMetalEngine.h"
#include "FeatureOps.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <Standard_Failure.hxx>

#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace androcad {

double SheetMetalRule::CalculateBendAllowance(double angleRad) const {
    if (angleRad <= 0.0) return 0.0;
    return angleRad * (bendRadius + kFactor * thickness);
}

double SheetMetalRule::CalculateBendDeduction(double angleRad) const {
    if (angleRad <= 0.0) return 0.0;
    double ossb = std::tan(angleRad / 2.0) * (bendRadius + thickness);
    double ba = CalculateBendAllowance(angleRad);
    return 2.0 * ossb - ba;
}

double SheetMetalRule::CalculateFlatLength(double leg1, double leg2, double angleRad) const {
    double bd = CalculateBendDeduction(angleRad);
    return leg1 + leg2 - bd;
}

TopoDS_Shape SheetMetalEngine::CreateBasePlate(double width,
                                               double length,
                                               const SheetMetalRule& rule) {
    if (width <= 0.0 || length <= 0.0 || rule.thickness <= 0.0) {
        throw std::invalid_argument("Sheet metal dimensions must be strictly positive");
    }
    try {
        BRepPrimAPI_MakeBox box(length, width, rule.thickness);
        box.Build();
        if (!box.IsDone()) {
            throw std::runtime_error("Failed to build sheet metal base plate");
        }
        return box.Shape();
    } catch (Standard_Failure& e) {
        throw std::runtime_error(std::string("OCCT error in CreateBasePlate: ") + e.GetMessageString());
    }
}

TopoDS_Shape SheetMetalEngine::CreateUBracket(double baseLength,
                                              double flangeHeight,
                                              double width,
                                              const SheetMetalRule& rule) {
    if (baseLength <= 0.0 || flangeHeight <= 0.0 || width <= 0.0 || rule.thickness <= 0.0) {
        throw std::invalid_argument("All bracket dimensions must be strictly positive");
    }

    double T = rule.thickness;
    double R = rule.bendRadius;
    double Ro = R + T;

    if (baseLength < 2.0 * Ro || flangeHeight < Ro) {
        throw std::invalid_argument("Base length or flange height too small for specified bend radius");
    }

    try {
        // Construct closed 2D profile loop in X-Z plane
        BRepBuilderAPI_MakeWire wireMaker;

        // Outer corner points
        gp_Pnt p1(0.0, 0.0, flangeHeight);          // Left flange top outer
        gp_Pnt p2(0.0, 0.0, Ro);                    // Left flange outer bend start
        gp_Pnt p3(Ro, 0.0, 0.0);                    // Bottom left outer bend end
        gp_Pnt p4(baseLength - Ro, 0.0, 0.0);       // Bottom right outer bend start
        gp_Pnt p5(baseLength, 0.0, Ro);             // Right flange outer bend end
        gp_Pnt p6(baseLength, 0.0, flangeHeight);   // Right flange top outer
        gp_Pnt p7(baseLength - T, 0.0, flangeHeight); // Right flange top inner
        gp_Pnt p8(baseLength - T, 0.0, Ro);         // Right flange inner bend start
        gp_Pnt p9(baseLength - Ro, 0.0, T);         // Inner base right bend end
        gp_Pnt p10(Ro, 0.0, T);                     // Inner base left bend start
        gp_Pnt p11(T, 0.0, Ro);                     // Left flange inner bend end
        gp_Pnt p12(T, 0.0, flangeHeight);           // Left flange top inner

        // 1. Line p1 -> p2 (Left flange outer vertical edge)
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2).Edge());

        if (R > 0.001) {
            // 2. Arc p2 -> p3 (Outer bend 1)
            gp_Pnt c1(Ro, 0.0, Ro);
            gp_Pnt p2_mid(Ro * (1.0 - 0.70710678), 0.0, Ro * (1.0 - 0.70710678));
            GC_MakeArcOfCircle arc1(p2, p2_mid, p3);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(arc1.Value()).Edge());
        } else {
            gp_Pnt corner(0.0, 0.0, 0.0);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, corner).Edge());
            wireMaker.Add(BRepBuilderAPI_MakeEdge(corner, p3).Edge());
        }

        // 3. Line p3 -> p4 (Bottom outer flat edge)
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4).Edge());

        if (R > 0.001) {
            // 4. Arc p4 -> p5 (Outer bend 2)
            gp_Pnt c2(baseLength - Ro, 0.0, Ro);
            gp_Pnt p4_mid(baseLength - Ro * (1.0 - 0.70710678), 0.0, Ro * (1.0 - 0.70710678));
            GC_MakeArcOfCircle arc2(p4, p4_mid, p5);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(arc2.Value()).Edge());
        } else {
            gp_Pnt corner(baseLength, 0.0, 0.0);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, corner).Edge());
            wireMaker.Add(BRepBuilderAPI_MakeEdge(corner, p5).Edge());
        }

        // 5. Line p5 -> p6 (Right flange outer vertical edge)
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6).Edge());

        // 6. Line p6 -> p7 (Right flange top thickness edge)
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7).Edge());

        // 7. Line p7 -> p8 (Right flange inner vertical edge)
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8).Edge());

        if (R > 0.001) {
            // 8. Arc p8 -> p9 (Inner bend 2)
            gp_Pnt p8_mid(baseLength - Ro + 0.70710678 * R, 0.0, T + (1.0 - 0.70710678) * R);
            GC_MakeArcOfCircle arc3(p8, p8_mid, p9);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(arc3.Value()).Edge());
        } else {
            gp_Pnt corner(baseLength - T, 0.0, T);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, corner).Edge());
            wireMaker.Add(BRepBuilderAPI_MakeEdge(corner, p9).Edge());
        }

        // 9. Line p9 -> p10 (Inner base flat edge)
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p9, p10).Edge());

        if (R > 0.001) {
            // 10. Arc p10 -> p11 (Inner bend 1)
            gp_Pnt p10_mid(T + (1.0 - 0.70710678) * R, 0.0, T + (1.0 - 0.70710678) * R);
            GC_MakeArcOfCircle arc4(p10, p10_mid, p11);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(arc4.Value()).Edge());
        } else {
            gp_Pnt corner(T, 0.0, T);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(p10, corner).Edge());
            wireMaker.Add(BRepBuilderAPI_MakeEdge(corner, p11).Edge());
        }

        // 11. Line p11 -> p12 (Left flange inner vertical edge)
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12).Edge());

        // 12. Line p12 -> p1 (Left flange top thickness edge - closes wire)
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p1).Edge());

        if (!wireMaker.IsDone()) {
            throw std::runtime_error("Failed to construct closed wire for U-Bracket profile");
        }

        BRepBuilderAPI_MakeFace faceMaker(wireMaker.Wire());
        if (!faceMaker.IsDone()) {
            throw std::runtime_error("Failed to construct planar face for U-Bracket profile");
        }

        // Extrude planar face along Y axis by width
        gp_Vec extrudeVec(0.0, width, 0.0);
        BRepPrimAPI_MakePrism prism(faceMaker.Face(), extrudeVec);
        prism.Build();
        if (!prism.IsDone()) {
            throw std::runtime_error("Failed to extrude U-Bracket solid prism");
        }

        return prism.Shape();
    } catch (Standard_Failure& e) {
        throw std::runtime_error(std::string("OCCT error in CreateUBracket: ") + e.GetMessageString());
    }
}

TopoDS_Shape SheetMetalEngine::AddFlange(const TopoDS_Shape& basePlate,
                                        double flangeHeight,
                                        double angleRad,
                                        const SheetMetalRule& rule) {
    if (flangeHeight <= 0.0 || rule.thickness <= 0.0) {
        throw std::invalid_argument("Flange height and thickness must be positive");
    }

    double L = 100.0;
    double W = 50.0;
    if (!basePlate.IsNull()) {
        Bnd_Box bnd;
        BRepBndLib::Add(basePlate, bnd);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bnd.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        L = xmax - xmin;
        W = ymax - ymin;
        if (L <= 0.0) L = 100.0;
        if (W <= 0.0) W = 50.0;
    }

    double T = rule.thickness;
    double R = rule.bendRadius;
    double Ro = R + T;

    try {
        // Construct L-profile in X-Z plane
        BRepBuilderAPI_MakeWire wireMaker;

        gp_Pnt p1(0.0, 0.0, 0.0);
        gp_Pnt p2(L - Ro, 0.0, 0.0);
        gp_Pnt p3(L, 0.0, Ro);
        gp_Pnt p4(L, 0.0, flangeHeight);
        gp_Pnt p5(L - T, 0.0, flangeHeight);
        gp_Pnt p6(L - T, 0.0, Ro);
        gp_Pnt p7(L - Ro, 0.0, T);
        gp_Pnt p8(0.0, 0.0, T);

        wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2).Edge());

        if (R > 0.001) {
            gp_Pnt p2_mid(L - Ro * (1.0 - 0.70710678), 0.0, Ro * (1.0 - 0.70710678));
            GC_MakeArcOfCircle arc1(p2, p2_mid, p3);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(arc1.Value()).Edge());
        } else {
            gp_Pnt corner(L, 0.0, 0.0);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, corner).Edge());
            wireMaker.Add(BRepBuilderAPI_MakeEdge(corner, p3).Edge());
        }

        wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4).Edge());
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5).Edge());
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6).Edge());

        if (R > 0.001) {
            gp_Pnt p6_mid(L - Ro + 0.70710678 * R, 0.0, T + (1.0 - 0.70710678) * R);
            GC_MakeArcOfCircle arc2(p6, p6_mid, p7);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(arc2.Value()).Edge());
        } else {
            gp_Pnt corner(L - T, 0.0, T);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, corner).Edge());
            wireMaker.Add(BRepBuilderAPI_MakeEdge(corner, p7).Edge());
        }

        wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8).Edge());
        wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p1).Edge());

        if (!wireMaker.IsDone()) {
            throw std::runtime_error("Failed to construct closed wire for L-Flange profile");
        }

        BRepBuilderAPI_MakeFace faceMaker(wireMaker.Wire());
        if (!faceMaker.IsDone()) {
            throw std::runtime_error("Failed to construct face for L-Flange profile");
        }

        gp_Vec extrudeVec(0.0, W, 0.0);
        BRepPrimAPI_MakePrism prism(faceMaker.Face(), extrudeVec);
        prism.Build();
        if (!prism.IsDone()) {
            throw std::runtime_error("Failed to extrude L-Flange shape");
        }

        return prism.Shape();
    } catch (Standard_Failure& e) {
        throw std::runtime_error(std::string("OCCT error in AddFlange: ") + e.GetMessageString());
    }
}

FlatPattern SheetMetalEngine::UnfoldUBracket(double baseLength,
                                             double flangeHeight,
                                             double width,
                                             const SheetMetalRule& rule) {
    FlatPattern result;
    if (baseLength <= 0.0 || flangeHeight <= 0.0 || width <= 0.0 || rule.thickness <= 0.0) {
        result.success = false;
        result.errorMessage = "Invalid sheet metal dimensions: all dimensions must be positive";
        return result;
    }

    const double angleRad = M_PI / 2.0; // 90 degree standard bends
    double bd = rule.CalculateBendDeduction(angleRad);

    // Total flat length = baseLength + 2 * flangeHeight - 2 * BD
    double flatLength = baseLength + 2.0 * flangeHeight - 2.0 * bd;
    if (flatLength <= 0.0) {
        result.success = false;
        result.errorMessage = "Calculated flat length is non-positive";
        return result;
    }

    result.success = true;
    result.flatLength = flatLength;
    result.flatWidth = width;
    result.flatArea = flatLength * width;

    // Bend Line 1 (left flange bend)
    double xBend1 = flangeHeight - bd / 2.0;
    BendLine b1;
    b1.id = 1;
    b1.startX = xBend1;
    b1.startY = 0.0;
    b1.endX = xBend1;
    b1.endY = width;
    b1.angleDeg = 90.0;
    b1.radius = rule.bendRadius;
    b1.isBendUp = true;
    result.bends.push_back(b1);

    // Bend Line 2 (right flange bend)
    double xBend2 = flatLength - (flangeHeight - bd / 2.0);
    BendLine b2;
    b2.id = 2;
    b2.startX = xBend2;
    b2.startY = 0.0;
    b2.endX = xBend2;
    b2.endY = width;
    b2.angleDeg = 90.0;
    b2.radius = rule.bendRadius;
    b2.isBendUp = true;
    result.bends.push_back(b2);

    try {
        result.flatSheetShape = BRepPrimAPI_MakeBox(flatLength, width, rule.thickness).Shape();
    } catch (...) {
        // Flat sheet shape generation is non-fatal for data
    }

    result.dxfContent = GenerateFlatPatternDXF(result);
    return result;
}

std::string SheetMetalEngine::GenerateFlatPatternDXF(const FlatPattern& pattern) {
    if (!pattern.success) {
        return "";
    }
    std::ostringstream dxf;
    dxf << std::fixed << std::setprecision(4);

    // Header & Layer Table
    dxf << "0\nSECTION\n2\nHEADER\n0\nENDSEC\n";
    dxf << "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nLAYER\n";
    // Layer CUT (Color 7: White)
    dxf << "0\nLAYER\n2\nCUT\n70\n0\n62\n7\n6\nCONTINUOUS\n0\n";
    // Layer BEND (Color 1: Red)
    dxf << "LAYER\n2\nBEND\n70\n0\n62\n1\n6\nCONTINUOUS\n0\n";
    dxf << "ENDTAB\n0\nENDSEC\n";

    // Entities
    dxf << "0\nSECTION\n2\nENTITIES\n";

    double L = pattern.flatLength;
    double W = pattern.flatWidth;

    auto addLine = [&](const std::string& layer, double x1, double y1, double x2, double y2) {
        dxf << "0\nLINE\n8\n" << layer << "\n";
        dxf << "10\n" << x1 << "\n20\n" << y1 << "\n30\n0.0\n";
        dxf << "11\n" << x2 << "\n21\n" << y2 << "\n31\n0.0\n";
    };

    // 4 Outer Cut boundary lines
    addLine("CUT", 0.0, 0.0, L, 0.0);
    addLine("CUT", L, 0.0, L, W);
    addLine("CUT", L, W, 0.0, W);
    addLine("CUT", 0.0, W, 0.0, 0.0);

    // Bend centerlines
    for (const auto& bend : pattern.bends) {
        addLine("BEND", bend.startX, bend.startY, bend.endX, bend.endY);

        double midX = (bend.startX + bend.endX) / 2.0;
        double midY = (bend.startY + bend.endY) / 2.0;
        dxf << "0\nTEXT\n8\nBEND\n";
        dxf << "10\n" << (midX + 2.0) << "\n20\n" << midY << "\n30\n0.0\n";
        dxf << "40\n3.0\n"; // 3mm text height
        dxf << "1\nBEND " << static_cast<int>(bend.angleDeg) << "%%d "
            << (bend.isBendUp ? "UP" : "DOWN") << " R" << bend.radius << "\n";
    }

    dxf << "0\nENDSEC\n0\nEOF\n";
    return dxf.str();
}

MeshPayload SheetMetalEngine::ToMeshPayload(const TopoDS_Shape& shape,
                                            double linearDeflection,
                                            double angularDeflection) {
    return FeatureOps::ShapeToMeshPayload(shape, linearDeflection, angularDeflection);
}

} // namespace androcad