// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "DrawingEngine.h"

#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <Geom_Plane.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <Standard_Failure.hxx>

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace androcad {

namespace {

// Projects a 3D point (x, y, z) centered at (cx, cy, cz) into 2D view space (u, v) and depth
void ProjectPoint(double x, double y, double z,
                  double cx, double cy, double cz,
                  ViewType type,
                  double& u, double& v, double& depth) {
    double dx = x - cx;
    double dy = y - cy;
    double dz = z - cz;

    const double cos30 = 0.8660254037844386;
    const double sin30 = 0.5;

    switch (type) {
        case ViewType::Front:
            // Viewing along -Y onto X-Z
            u = dx;
            v = dz;
            depth = -dy;
            break;
        case ViewType::Top:
            // Viewing along -Z onto X-Y (Top view, Y points upwards)
            u = dx;
            v = dy;
            depth = dz;
            break;
        case ViewType::Right:
            // Viewing along +X onto Y-Z
            u = dy;
            v = dz;
            depth = -dx;
            break;
        case ViewType::Isometric:
            // Standard 30-degree isometric projection
            u = (dx - dy) * cos30;
            v = dz - (dx + dy) * sin30;
            depth = (dx + dy) * cos30 + dz * sin30;
            break;
        case ViewType::Section:
            // Section in X-Z
            u = dx;
            v = dz;
            depth = 0.0;
            break;
    }
}

// Returns the view normal vector pointing towards the observer
gp_Dir GetViewDirection(ViewType type) {
    switch (type) {
        case ViewType::Front:
            return gp_Dir(0.0, -1.0, 0.0);
        case ViewType::Top:
            return gp_Dir(0.0, 0.0, 1.0);
        case ViewType::Right:
            return gp_Dir(1.0, 0.0, 0.0);
        case ViewType::Isometric: {
            double invSqrt3 = 1.0 / std::sqrt(3.0);
            return gp_Dir(invSqrt3, invSqrt3, invSqrt3);
        }
        case ViewType::Section:
        default:
            return gp_Dir(0.0, -1.0, 0.0);
    }
}

} // namespace

DrawingView DrawingEngine::ProjectView(const TopoDS_Shape& shape,
                                       ViewType type,
                                       double originX,
                                       double originY,
                                       double scale) {
    DrawingView view;
    view.type = type;
    view.originX = originX;
    view.originY = originY;
    view.scale = scale;

    switch (type) {
        case ViewType::Front:     view.name = "FRONT VIEW"; break;
        case ViewType::Top:       view.name = "TOP VIEW"; break;
        case ViewType::Right:     view.name = "RIGHT VIEW"; break;
        case ViewType::Isometric: view.name = "ISOMETRIC VIEW"; break;
        case ViewType::Section:   view.name = "SECTION A-A"; break;
    }

    if (shape.IsNull()) {
        return view;
    }

    // 1. Calculate 3D bounding box center
    Bnd_Box bnd;
    BRepBndLib::Add(shape, bnd);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bnd.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    double cx = (xmin + xmax) * 0.5;
    double cy = (ymin + ymax) * 0.5;
    double cz = (zmin + zmax) * 0.5;

    // 2. Map edges to adjacent faces for visibility / silhouette classification
    TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
    TopExp::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);

    gp_Dir viewDir = GetViewDirection(type);

    double minU = 1e9, maxU = -1e9;
    double minV = 1e9, maxV = -1e9;

    // 3. Process each unique edge in the model
    for (Standard_Integer i = 1; i <= edgeFaceMap.Extent(); ++i) {
        const TopoDS_Edge& edge = TopoDS::Edge(edgeFaceMap.FindKey(i));
        const TopTools_ListOfShape& faceList = edgeFaceMap.FindFromIndex(i);

        BRepAdaptor_Curve curve(edge);
        Standard_Real first = curve.FirstParameter();
        Standard_Real last = curve.LastParameter();

        if (std::isinf(first) || std::isinf(last) || first >= last) {
            continue;
        }

        // Determine if edge is visible or hidden using face normals
        bool hasFrontFace = false;
        bool hasBackFace = false;

        for (TopTools_ListIteratorOfListOfShape it(faceList); it.More(); it.Next()) {
            const TopoDS_Face& face = TopoDS::Face(it.Value());
            try {
                BRepAdaptor_Surface surf(face);
                Standard_Real uMid = (surf.FirstUParameter() + surf.LastUParameter()) * 0.5;
                Standard_Real vMid = (surf.FirstVParameter() + surf.LastVParameter()) * 0.5;
                gp_Pnt p;
                gp_Vec d1u, d1v;
                surf.D1(uMid, vMid, p, d1u, d1v);
                gp_Vec normVec = d1u.Crossed(d1v);
                if (normVec.SquareMagnitude() > 1e-8) {
                    gp_Dir faceNormal(normVec);
                    if (face.Orientation() == TopAbs_REVERSED) {
                        faceNormal.Reverse();
                    }
                    double dot = faceNormal.Dot(viewDir);
                    if (dot > 0.05) {
                        hasFrontFace = true;
                    } else if (dot < -0.05) {
                        hasBackFace = true;
                    }
                }
            } catch (...) {
                hasFrontFace = true;
            }
        }

        // If edge only borders back faces, it is hidden
        bool isHidden = (!hasFrontFace && hasBackFace);

        // Discretize edge into linear segments (lines = 1 segment, curves = 8 segments)
        GeomAbs_CurveType cType = curve.GetType();
        int numSamples = (cType == GeomAbs_Line) ? 2 : 8;

        for (int s = 0; s < numSamples - 1; ++s) {
            double t1 = first + (last - first) * (static_cast<double>(s) / (numSamples - 1));
            double t2 = first + (last - first) * (static_cast<double>(s + 1) / (numSamples - 1));

            gp_Pnt p1 = curve.Value(t1);
            gp_Pnt p2 = curve.Value(t2);

            double u1, v1, d1;
            double u2, v2, d2;
            ProjectPoint(p1.X(), p1.Y(), p1.Z(), cx, cy, cz, type, u1, v1, d1);
            ProjectPoint(p2.X(), p2.Y(), p2.Z(), cx, cy, cz, type, u2, v2, d2);

            // Apply view scale and position offset
            u1 *= scale; v1 *= scale;
            u2 *= scale; v2 *= scale;

            ProjectedSegment seg;
            seg.x1 = originX + u1;
            seg.y1 = originY + v1;
            seg.x2 = originX + u2;
            seg.y2 = originY + v2;
            seg.isHidden = isHidden;

            view.segments.push_back(seg);

            minU = std::min({minU, u1, u2});
            maxU = std::max({maxU, u1, u2});
            minV = std::min({minV, v1, v2});
            maxV = std::max({maxV, v1, v2});
        }
    }

    view.width = (maxU > minU) ? (maxU - minU) : 0.0;
    view.height = (maxV > minV) ? (maxV - minV) : 0.0;

    return view;
}

DrawingView DrawingEngine::CreateSectionView(const TopoDS_Shape& shape,
                                             double cutY,
                                             double originX,
                                             double originY,
                                             double scale) {
    DrawingView view;
    view.type = ViewType::Section;
    view.name = "SECTION A-A";
    view.originX = originX;
    view.originY = originY;
    view.scale = scale;

    if (shape.IsNull()) {
        return view;
    }

    try {
        Bnd_Box bnd;
        BRepBndLib::Add(shape, bnd);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bnd.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        double cx = (xmin + xmax) * 0.5;
        double cy = (ymin + ymax) * 0.5;
        double cz = (zmin + zmax) * 0.5;

        // Plane cutting along Y = cutY with normal +Y
        gp_Pln cutPlane(gp_Pnt(0.0, cutY, 0.0), gp_Dir(0.0, 1.0, 0.0));
        BRepAlgoAPI_Section sectionMaker(shape, cutPlane);
        sectionMaker.Build();

        if (!sectionMaker.IsDone()) {
            return view;
        }

        TopoDS_Shape sectionShape = sectionMaker.Shape();

        double minU = 1e9, maxU = -1e9;
        double minV = 1e9, maxV = -1e9;

        // Extract section intersection boundary edges
        for (TopExp_Explorer exp(sectionShape, TopAbs_EDGE); exp.More(); exp.Next()) {
            const TopoDS_Edge& edge = TopoDS::Edge(exp.Current());
            BRepAdaptor_Curve curve(edge);
            Standard_Real first = curve.FirstParameter();
            Standard_Real last = curve.LastParameter();

            int numSamples = (curve.GetType() == GeomAbs_Line) ? 2 : 8;
            for (int s = 0; s < numSamples - 1; ++s) {
                double t1 = first + (last - first) * (static_cast<double>(s) / (numSamples - 1));
                double t2 = first + (last - first) * (static_cast<double>(s + 1) / (numSamples - 1));

                gp_Pnt p1 = curve.Value(t1);
                gp_Pnt p2 = curve.Value(t2);

                double u1 = (p1.X() - cx) * scale;
                double v1 = (p1.Z() - cz) * scale;
                double u2 = (p2.X() - cx) * scale;
                double v2 = (p2.Z() - cz) * scale;

                ProjectedSegment seg;
                seg.x1 = originX + u1;
                seg.y1 = originY + v1;
                seg.x2 = originX + u2;
                seg.y2 = originY + v2;
                seg.isHidden = false;
                view.segments.push_back(seg);

                minU = std::min({minU, u1, u2});
                maxU = std::max({maxU, u1, u2});
                minV = std::min({minV, v1, v2});
                maxV = std::max({maxV, v1, v2});
            }
        }

        // Add 45-degree section cross-hatching inside cut bounds
        if (maxU > minU && maxV > minV) {
            double hatchSpacing = 4.0 * scale;
            double diagStart = (minU - (maxV - minV));
            double diagEnd = (maxU + (maxV - minV));

            for (double d = diagStart; d <= diagEnd; d += hatchSpacing) {
                // Line at 45 degrees: v = u - d  ==> u - v = d
                double u_a = std::max(minU, minV + d);
                double v_a = u_a - d;
                double u_b = std::min(maxU, maxV + d);
                double v_b = u_b - d;

                if (u_b > u_a && v_a >= minV && v_a <= maxV && v_b >= minV && v_b <= maxV) {
                    ProjectedSegment hatchSeg;
                    hatchSeg.x1 = originX + u_a;
                    hatchSeg.y1 = originY + v_a;
                    hatchSeg.x2 = originX + u_b;
                    hatchSeg.y2 = originY + v_b;
                    hatchSeg.isHidden = false;
                    view.segments.push_back(hatchSeg);
                }
            }
        }

        view.width = (maxU > minU) ? (maxU - minU) : 0.0;
        view.height = (maxV > minV) ? (maxV - minV) : 0.0;
    } catch (...) {
        // Fallback for empty section
    }

    return view;
}

DrawingSheet DrawingEngine::GenerateStandardDrawingSheet(const TopoDS_Shape& shape,
                                                         const std::string& partName,
                                                         double scale) {
    DrawingSheet sheet;
    sheet.sheetWidth = 420.0;  // A3 Width (mm)
    sheet.sheetHeight = 297.0; // A3 Height (mm)

    sheet.titleBlock.partName = partName;
    std::ostringstream sc;
    sc << "1:" << static_cast<int>(1.0 / scale);
    if (scale >= 1.0) {
        sc.str("");
        sc << static_cast<int>(scale) << ":1";
    }
    sheet.titleBlock.scaleStr = sc.str();

    // Standard 3rd Angle Projection Layout on A3:
    // Top View: (130, 205)
    // Front View: (130, 95)
    // Right View: (255, 95)
    // Isometric View: (275, 205)
    sheet.views.push_back(ProjectView(shape, ViewType::Top, 130.0, 205.0, scale));
    sheet.views.push_back(ProjectView(shape, ViewType::Front, 130.0, 95.0, scale));
    sheet.views.push_back(ProjectView(shape, ViewType::Right, 255.0, 95.0, scale));
    sheet.views.push_back(ProjectView(shape, ViewType::Isometric, 275.0, 205.0, scale));

    AddOverallDimensions(sheet);
    return sheet;
}

void DrawingEngine::AddOverallDimensions(DrawingSheet& sheet) {
    for (auto& view : sheet.views) {
        if (view.segments.empty()) continue;

        double minX = 1e9, maxX = -1e9;
        double minY = 1e9, maxY = -1e9;
        for (const auto& seg : view.segments) {
            minX = std::min({minX, seg.x1, seg.x2});
            maxX = std::max({maxX, seg.x1, seg.x2});
            minY = std::min({minY, seg.y1, seg.y2});
            maxY = std::max({maxY, seg.y1, seg.y2});
        }

        double widthMm = (maxX - minX) / view.scale;
        double heightMm = (maxY - minY) / view.scale;

        if (view.type == ViewType::Front) {
            // Horizontal overall dimension (Width)
            DimensionCallout d1;
            std::ostringstream s1;
            s1 << std::fixed << std::setprecision(1) << widthMm << " mm";
            d1.text = s1.str();
            d1.x1 = minX; d1.y1 = minY;
            d1.x2 = maxX; d1.y2 = minY;
            d1.offset = -12.0; // 12mm below
            view.dimensions.push_back(d1);

            // Vertical overall dimension (Height)
            DimensionCallout d2;
            std::ostringstream s2;
            s2 << std::fixed << std::setprecision(1) << heightMm << " mm";
            d2.text = s2.str();
            d2.x1 = maxX; d2.y1 = minY;
            d2.x2 = maxX; d2.y2 = maxY;
            d2.offset = 12.0; // 12mm right
            view.dimensions.push_back(d2);
        } else if (view.type == ViewType::Top) {
            // Vertical overall dimension (Depth)
            DimensionCallout d3;
            std::ostringstream s3;
            s3 << std::fixed << std::setprecision(1) << heightMm << " mm";
            d3.text = s3.str();
            d3.x1 = minX; d3.y1 = minY;
            d3.x2 = minX; d3.y2 = maxY;
            d3.offset = -12.0; // 12mm left
            view.dimensions.push_back(d3);
        }
    }
}

std::string DrawingSheet::ToSVG() const {
    std::ostringstream svg;
    svg << std::fixed << std::setprecision(2);

    svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 "
        << sheetWidth << " " << sheetHeight << "\" width=\""
        << sheetWidth << "mm\" height=\"" << sheetHeight << "mm\">\n";

    // Styles
    svg << "<defs>\n";
    svg << "  <marker id=\"arrow\" viewBox=\"0 0 10 10\" refX=\"5\" refY=\"5\" markerWidth=\"4\" markerHeight=\"4\" orient=\"auto-start-reverse\">\n";
    svg << "    <path d=\"M 0 2 L 10 5 L 0 8 z\" fill=\"#007ACC\" />\n";
    svg << "  </marker>\n";
    svg << "</defs>\n";

    // Background sheet paper
    svg << "<rect width=\"" << sheetWidth << "\" height=\"" << sheetHeight << "\" fill=\"#FAFAFA\" stroke=\"none\" />\n";

    // Drawing Sheet Border (Outer 10mm margin, left 20mm filing margin)
    double leftMargin = 20.0;
    double margin = 10.0;
    svg << "<rect x=\"" << leftMargin << "\" y=\"" << margin << "\" width=\""
        << (sheetWidth - leftMargin - margin) << "\" height=\""
        << (sheetHeight - 2.0 * margin) << "\" fill=\"none\" stroke=\"#1E2124\" stroke-width=\"0.7\" />\n";

    // Standard Title Block in bottom-right corner (160mm x 50mm)
    double tbW = 160.0;
    double tbH = 45.0;
    double tbX = sheetWidth - margin - tbW;
    double tbY = sheetHeight - margin - tbH;

    svg << "<rect x=\"" << tbX << "\" y=\"" << tbY << "\" width=\"" << tbW << "\" height=\"" << tbH
        << "\" fill=\"#FFFFFF\" stroke=\"#1E2124\" stroke-width=\"0.6\" />\n";
    svg << "<line x1=\"" << tbX << "\" y1=\"" << (tbY + 15.0) << "\" x2=\"" << (tbX + tbW) << "\" y2=\"" << (tbY + 15.0) << "\" stroke=\"#1E2124\" stroke-width=\"0.4\" />\n";
    svg << "<line x1=\"" << tbX << "\" y1=\"" << (tbY + 30.0) << "\" x2=\"" << (tbX + tbW) << "\" y2=\"" << (tbY + 30.0) << "\" stroke=\"#1E2124\" stroke-width=\"0.4\" />\n";
    svg << "<line x1=\"" << (tbX + 80.0) << "\" y1=\"" << (tbY + 15.0) << "\" x2=\"" << (tbX + 80.0) << "\" y2=\"" << (tbY + tbH) << "\" stroke=\"#1E2124\" stroke-width=\"0.4\" />\n";

    // Title Block Text
    svg << "<text x=\"" << (tbX + 5.0) << "\" y=\"" << (tbY + 10.0) << "\" font-family=\"monospace\" font-size=\"4.5\" font-weight=\"bold\" fill=\"#007ACC\">"
        << titleBlock.title << "</text>\n";
    svg << "<text x=\"" << (tbX + 5.0) << "\" y=\"" << (tbY + 23.0) << "\" font-family=\"sans-serif\" font-size=\"3.2\" fill=\"#1E2124\">PART: "
        << titleBlock.partName << "</text>\n";
    svg << "<text x=\"" << (tbX + 85.0) << "\" y=\"" << (tbY + 23.0) << "\" font-family=\"sans-serif\" font-size=\"3.2\" fill=\"#1E2124\">SCALE: "
        << titleBlock.scaleStr << "</text>\n";
    svg << "<text x=\"" << (tbX + 5.0) << "\" y=\"" << (tbY + 38.0) << "\" font-family=\"sans-serif\" font-size=\"3.2\" fill=\"#1E2124\">DATE: "
        << titleBlock.date << "</text>\n";
    svg << "<text x=\"" << (tbX + 85.0) << "\" y=\"" << (tbY + 38.0) << "\" font-family=\"sans-serif\" font-size=\"3.2\" fill=\"#1E2124\">DWG BY: "
        << titleBlock.author << "</text>\n";

    // Render Views
    for (const auto& view : views) {
        // SVG coordinate system has Y pointing down, CAD drawing has Y pointing up
        // So we invert Y relative to sheetHeight
        auto svgY = [this](double cadY) { return sheetHeight - cadY; };

        // View Name Label
        svg << "<text x=\"" << (view.originX - 25.0) << "\" y=\"" << svgY(view.originY - (view.height * 0.5) - 10.0)
            << "\" font-family=\"sans-serif\" font-size=\"3.8\" font-weight=\"bold\" fill=\"#2F3136\">"
            << view.name << "</text>\n";

        // Segments
        for (const auto& seg : view.segments) {
            double sy1 = svgY(seg.y1);
            double sy2 = svgY(seg.y2);

            if (seg.isHidden) {
                svg << "<line x1=\"" << seg.x1 << "\" y1=\"" << sy1
                    << "\" x2=\"" << seg.x2 << "\" y2=\"" << sy2
                    << "\" stroke=\"#888888\" stroke-width=\"0.3\" stroke-dasharray=\"2,1.5\" />\n";
            } else {
                svg << "<line x1=\"" << seg.x1 << "\" y1=\"" << sy1
                    << "\" x2=\"" << seg.x2 << "\" y2=\"" << sy2
                    << "\" stroke=\"#1E2124\" stroke-width=\"0.55\" stroke-linecap=\"round\" />\n";
            }
        }

        // Dimensions
        for (const auto& dim : view.dimensions) {
            double sy1 = svgY(dim.y1);
            double sy2 = svgY(dim.y2);

            if (std::abs(dim.y1 - dim.y2) < 1e-4) {
                // Horizontal dimension line
                double dY = svgY(dim.y1 + dim.offset);
                // Extension lines
                svg << "<line x1=\"" << dim.x1 << "\" y1=\"" << sy1 << "\" x2=\"" << dim.x1 << "\" y2=\"" << (dY + (dim.offset > 0 ? -2.0 : 2.0)) << "\" stroke=\"#007ACC\" stroke-width=\"0.25\" />\n";
                svg << "<line x1=\"" << dim.x2 << "\" y1=\"" << sy2 << "\" x2=\"" << dim.x2 << "\" y2=\"" << (dY + (dim.offset > 0 ? -2.0 : 2.0)) << "\" stroke=\"#007ACC\" stroke-width=\"0.25\" />\n";
                // Dimension line with arrows
                svg << "<line x1=\"" << dim.x1 << "\" y1=\"" << dY << "\" x2=\"" << dim.x2 << "\" y2=\"" << dY << "\" stroke=\"#007ACC\" stroke-width=\"0.35\" marker-start=\"url(#arrow)\" marker-end=\"url(#arrow)\" />\n";
                // Text label
                double midX = (dim.x1 + dim.x2) * 0.5;
                svg << "<text x=\"" << (midX - 10.0) << "\" y=\"" << (dY - 1.5) << "\" font-family=\"sans-serif\" font-size=\"3.0\" fill=\"#007ACC\">" << dim.text << "</text>\n";
            } else {
                // Vertical dimension line
                double dX = dim.x1 + dim.offset;
                svg << "<line x1=\"" << dim.x1 << "\" y1=\"" << sy1 << "\" x2=\"" << (dX + (dim.offset > 0 ? 2.0 : -2.0)) << "\" y2=\"" << sy1 << "\" stroke=\"#007ACC\" stroke-width=\"0.25\" />\n";
                svg << "<line x1=\"" << dim.x2 << "\" y1=\"" << sy2 << "\" x2=\"" << (dX + (dim.offset > 0 ? 2.0 : -2.0)) << "\" y2=\"" << sy2 << "\" stroke=\"#007ACC\" stroke-width=\"0.25\" />\n";
                svg << "<line x1=\"" << dX << "\" y1=\"" << sy1 << "\" x2=\"" << dX << "\" y2=\"" << sy2 << "\" stroke=\"#007ACC\" stroke-width=\"0.35\" marker-start=\"url(#arrow)\" marker-end=\"url(#arrow)\" />\n";
                double midY = (sy1 + sy2) * 0.5;
                svg << "<text x=\"" << (dX + 2.0) << "\" y=\"" << midY << "\" font-family=\"sans-serif\" font-size=\"3.0\" fill=\"#007ACC\">" << dim.text << "</text>\n";
            }
        }

        // GD&T Feature Control Frames
        for (const auto& gdt : view.gdtFrames) {
            svg << gdt.ToSVG();
        }

        // BOM Callout Balloons
        for (const auto& balloon : view.balloons) {
            svg << balloon.ToSVG();
        }
    }

    // Bill of Materials Table
    if (!bomTable.items.empty()) {
        svg << bomTable.ToSVG();
    }

    svg << "</svg>\n";
    return svg.str();
}

std::string FeatureControlFrame::FormattedString() const {
    std::ostringstream ss;
    ss << "[ ";
    switch (characteristic) {
        case GDTCharacteristic::POSITION: ss << "POS (⌖)"; break;
        case GDTCharacteristic::FLATNESS: ss << "FLAT (⏥)"; break;
        case GDTCharacteristic::PERPENDICULARITY: ss << "PERP (⟂)"; break;
        case GDTCharacteristic::PARALLELISM: ss << "PARA (∥)"; break;
        case GDTCharacteristic::CYLINDRICITY: ss << "CYL (⌭)"; break;
        case GDTCharacteristic::TOTAL_RUNOUT: ss << "RUNOUT (⌰)"; break;
        case GDTCharacteristic::CONCENTRICITY: ss << "CONC (◎)"; break;
        case GDTCharacteristic::PROFILE_OF_SURFACE: ss << "PROF (⌒)"; break;
    }
    ss << " | ";
    if (hasDiameterSymbol) ss << "⌀";
    ss << std::fixed << std::setprecision(3) << toleranceValue;
    if (materialCondition == MaterialCondition::MMC) ss << " Ⓜ";
    else if (materialCondition == MaterialCondition::LMC) ss << " Ⓛ";

    if (!primaryDatum.empty()) {
        ss << " | " << primaryDatum;
        if (primaryDatumModifier == MaterialCondition::MMC) ss << " Ⓜ";
        else if (primaryDatumModifier == MaterialCondition::LMC) ss << " Ⓛ";
    }
    if (!secondaryDatum.empty()) {
        ss << " | " << secondaryDatum;
        if (secondaryDatumModifier == MaterialCondition::MMC) ss << " Ⓜ";
        else if (secondaryDatumModifier == MaterialCondition::LMC) ss << " Ⓛ";
    }
    if (!tertiaryDatum.empty()) {
        ss << " | " << tertiaryDatum;
        if (tertiaryDatumModifier == MaterialCondition::MMC) ss << " Ⓜ";
        else if (tertiaryDatumModifier == MaterialCondition::LMC) ss << " Ⓛ";
    }
    ss << " ]";
    return ss.str();
}

std::string FeatureControlFrame::ToSVG() const {
    std::ostringstream svg;
    svg << std::fixed << std::setprecision(2);
    if (std::abs(attachX) > 1e-3 || std::abs(attachY) > 1e-3) {
        svg << "<line x1=\"" << attachX << "\" y1=\"" << (297.0 - attachY)
            << "\" x2=\"" << boxX << "\" y2=\"" << (297.0 - boxY)
            << "\" stroke=\"#007ACC\" stroke-width=\"0.35\" marker-start=\"url(#arrow)\" />\n";
    }

    double sy = 297.0 - boxY;
    svg << "<g id=\"gdt_frame\">\n";
    svg << "  <rect x=\"" << boxX << "\" y=\"" << (sy - boxHeight) << "\" width=\"" << boxWidth << "\" height=\"" << boxHeight
        << "\" fill=\"#FFFFFF\" stroke=\"#1E2124\" stroke-width=\"0.5\" />\n";
    svg << "  <text x=\"" << (boxX + 2.0) << "\" y=\"" << (sy - 2.5) << "\" font-family=\"monospace\" font-size=\"3.0\" fill=\"#1E2124\">"
        << FormattedString() << "</text>\n";
    svg << "</g>\n";
    return svg.str();
}

std::string BOMBalloon::ToSVG() const {
    std::ostringstream svg;
    svg << std::fixed << std::setprecision(2);
    double sy = 297.0 - balloonY;
    double say = 297.0 - attachY;

    svg << "<line x1=\"" << attachX << "\" y1=\"" << say
        << "\" x2=\"" << balloonX << "\" y2=\"" << sy
        << "\" stroke=\"#007ACC\" stroke-width=\"0.35\" marker-start=\"url(#arrow)\" />\n";

    svg << "<circle cx=\"" << balloonX << "\" cy=\"" << sy << "\" r=\"" << radius
        << "\" fill=\"#FFFFFF\" stroke=\"#1E2124\" stroke-width=\"0.6\" />\n";

    svg << "<text x=\"" << balloonX << "\" y=\"" << (sy + 1.2)
        << "\" font-family=\"sans-serif\" font-size=\"4.0\" font-weight=\"bold\" text-anchor=\"middle\" dominant-baseline=\"middle\" fill=\"#007ACC\">"
        << itemNumber << "</text>\n";

    return svg.str();
}

std::string BOMTable::ToSVG() const {
    if (items.empty()) return "";
    std::ostringstream svg;
    svg << std::fixed << std::setprecision(1);

    double totalW = 145.0;
    double colW[] = {12.0, 28.0, 42.0, 12.0, 26.0, 25.0};
    double headerH = 7.0;
    double rH = rowHeight;
    double totalH = headerH + items.size() * rH;

    double x = originX;
    double y = 297.0 - originY;

    svg << "<g id=\"bom_table\">\n";
    svg << "  <rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << totalW << "\" height=\"" << totalH
        << "\" fill=\"#FFFFFF\" stroke=\"#1E2124\" stroke-width=\"0.6\" />\n";

    svg << "  <rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << totalW << "\" height=\"" << headerH
        << "\" fill=\"#E2E8F0\" stroke=\"#1E2124\" stroke-width=\"0.4\" />\n";

    const char* headers[] = {"ITEM", "PART NO", "DESCRIPTION", "QTY", "MATERIAL", "MASS (g)"};
    double curX = x;
    for (int c = 0; c < 6; ++c) {
        svg << "  <text x=\"" << (curX + 1.5) << "\" y=\"" << (y + 4.8)
            << "\" font-family=\"sans-serif\" font-size=\"2.8\" font-weight=\"bold\" fill=\"#1E2124\">"
            << headers[c] << "</text>\n";
        curX += colW[c];
        if (c < 5) {
            svg << "  <line x1=\"" << curX << "\" y1=\"" << y << "\" x2=\"" << curX << "\" y2=\"" << (y + totalH)
                << "\" stroke=\"#1E2124\" stroke-width=\"0.3\" />\n";
        }
    }

    for (size_t r = 0; r < items.size(); ++r) {
        const auto& it = items[r];
        double ry = y + headerH + r * rH;
        svg << "  <line x1=\"" << x << "\" y1=\"" << ry << "\" x2=\"" << (x + totalW) << "\" y2=\"" << ry
            << "\" stroke=\"#CBD5E1\" stroke-width=\"0.3\" />\n";

        double cx = x;
        svg << "  <text x=\"" << (cx + colW[0]*0.5) << "\" y=\"" << (ry + 4.8)
            << "\" font-family=\"sans-serif\" font-size=\"2.6\" text-anchor=\"middle\" fill=\"#1E2124\">"
            << it.itemNumber << "</text>\n";
        cx += colW[0];

        svg << "  <text x=\"" << (cx + 1.5) << "\" y=\"" << (ry + 4.8)
            << "\" font-family=\"sans-serif\" font-size=\"2.6\" fill=\"#007ACC\">"
            << it.partNumber << "</text>\n";
        cx += colW[1];

        svg << "  <text x=\"" << (cx + 1.5) << "\" y=\"" << (ry + 4.8)
            << "\" font-family=\"sans-serif\" font-size=\"2.6\" fill=\"#1E2124\">"
            << it.description << "</text>\n";
        cx += colW[2];

        svg << "  <text x=\"" << (cx + colW[3]*0.5) << "\" y=\"" << (ry + 4.8)
            << "\" font-family=\"sans-serif\" font-size=\"2.6\" text-anchor=\"middle\" fill=\"#1E2124\">"
            << it.quantity << "</text>\n";
        cx += colW[3];

        svg << "  <text x=\"" << (cx + 1.5) << "\" y=\"" << (ry + 4.8)
            << "\" font-family=\"sans-serif\" font-size=\"2.6\" fill=\"#475569\">"
            << it.materialName << "</text>\n";
        cx += colW[4];

        svg << "  <text x=\"" << (cx + colW[5] - 2.0) << "\" y=\"" << (ry + 4.8)
            << "\" font-family=\"sans-serif\" font-size=\"2.6\" text-anchor=\"end\" fill=\"#1E2124\">"
            << it.totalMassGrams << "</text>\n";
    }
    svg << "</g>\n";
    return svg.str();
}

DrawingView DrawingEngine::CreateDetailView(const DrawingView& parentView,
                                            double centerX, double centerY,
                                            double radiusMm,
                                            double scaleMultiplier,
                                            double placementX, double placementY,
                                            const std::string& detailLabel) {
    DrawingView view;
    view.type = ViewType::Detail;
    std::ostringstream ss;
    ss << "DETAIL " << detailLabel << " (" << static_cast<int>(scaleMultiplier) << ":1)";
    view.name = ss.str();
    view.originX = placementX;
    view.originY = placementY;
    view.scale = parentView.scale * scaleMultiplier;

    double rSq = radiusMm * radiusMm;

    for (const auto& seg : parentView.segments) {
        double mx = (seg.x1 + seg.x2) * 0.5;
        double my = (seg.y1 + seg.y2) * 0.5;

        double dSq = (mx - centerX) * (mx - centerX) + (my - centerY) * (my - centerY);
        if (dSq <= rSq * 1.2) {
            ProjectedSegment cropSeg;
            cropSeg.x1 = placementX + (seg.x1 - centerX) * scaleMultiplier;
            cropSeg.y1 = placementY + (seg.y1 - centerY) * scaleMultiplier;
            cropSeg.x2 = placementX + (seg.x2 - centerX) * scaleMultiplier;
            cropSeg.y2 = placementY + (seg.y2 - centerY) * scaleMultiplier;
            cropSeg.isHidden = seg.isHidden;
            cropSeg.isCenterline = seg.isCenterline;
            view.segments.push_back(cropSeg);
        }
    }

    double detailR = radiusMm * scaleMultiplier;
    int circleSamples = 24;
    for (int i = 0; i < circleSamples; ++i) {
        double th1 = 2.0 * M_PI * i / circleSamples;
        double th2 = 2.0 * M_PI * (i + 1) / circleSamples;
        ProjectedSegment cSeg;
        cSeg.x1 = placementX + detailR * std::cos(th1);
        cSeg.y1 = placementY + detailR * std::sin(th1);
        cSeg.x2 = placementX + detailR * std::cos(th2);
        cSeg.y2 = placementY + detailR * std::sin(th2);
        cSeg.isHidden = false;
        view.segments.push_back(cSeg);
    }

    view.width = detailR * 2.0;
    view.height = detailR * 2.0;
    return view;
}

DrawingView DrawingEngine::CreateAuxiliaryView(const TopoDS_Shape& shape,
                                               double dirX, double dirY, double dirZ,
                                               double originX, double originY,
                                               double scale) {
    DrawingView view;
    view.type = ViewType::Auxiliary;
    view.name = "AUXILIARY VIEW (OBLIQUE)";
    view.originX = originX;
    view.originY = originY;
    view.scale = scale;

    if (shape.IsNull()) return view;

    double len = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
    if (len < 1e-6) {
        dirX = 1.0; dirY = 1.0; dirZ = 1.0; len = std::sqrt(3.0);
    }
    dirX /= len; dirY /= len; dirZ /= len;

    gp_Dir wDir(dirX, dirY, dirZ);
    gp_Dir uDir = (std::abs(dirZ) < 0.9) ? gp_Dir(-dirY, dirX, 0.0) : gp_Dir(0.0, -dirZ, dirY);
    gp_Vec vVec = gp_Vec(wDir).Crossed(gp_Vec(uDir));
    gp_Dir vDir(vVec);

    Bnd_Box bnd;
    BRepBndLib::Add(shape, bnd);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bnd.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    double cx = (xmin + xmax) * 0.5;
    double cy = (ymin + ymax) * 0.5;
    double cz = (zmin + zmax) * 0.5;

    double minU = 1e9, maxU = -1e9;
    double minV = 1e9, maxV = -1e9;

    for (TopExp_Explorer exp(shape, TopAbs_EDGE); exp.More(); exp.Next()) {
        const TopoDS_Edge& edge = TopoDS::Edge(exp.Current());
        BRepAdaptor_Curve curve(edge);
        Standard_Real first = curve.FirstParameter();
        Standard_Real last = curve.LastParameter();
        if (std::isinf(first) || std::isinf(last) || first >= last) continue;

        int numSamples = (curve.GetType() == GeomAbs_Line) ? 2 : 8;
        for (int s = 0; s < numSamples - 1; ++s) {
            double t1 = first + (last - first) * (static_cast<double>(s) / (numSamples - 1));
            double t2 = first + (last - first) * (static_cast<double>(s + 1) / (numSamples - 1));

            gp_Pnt p1 = curve.Value(t1);
            gp_Pnt p2 = curve.Value(t2);

            gp_Vec d1(p1.X() - cx, p1.Y() - cy, p1.Z() - cz);
            gp_Vec d2(p2.X() - cx, p2.Y() - cy, p2.Z() - cz);

            double u1 = d1.Dot(gp_Vec(uDir)) * scale;
            double v1 = d1.Dot(gp_Vec(vDir)) * scale;
            double u2 = d2.Dot(gp_Vec(uDir)) * scale;
            double v2 = d2.Dot(gp_Vec(vDir)) * scale;

            ProjectedSegment seg;
            seg.x1 = originX + u1;
            seg.y1 = originY + v1;
            seg.x2 = originX + u2;
            seg.y2 = originY + v2;
            seg.isHidden = false;
            view.segments.push_back(seg);

            minU = std::min({minU, u1, u2});
            maxU = std::max({maxU, u1, u2});
            minV = std::min({minV, v1, v2});
            maxV = std::max({maxV, v1, v2});
        }
    }

    view.width = (maxU > minU) ? (maxU - minU) : 0.0;
    view.height = (maxV > minV) ? (maxV - minV) : 0.0;
    return view;
}

FeatureControlFrame DrawingEngine::CreateGDTFrame(GDTCharacteristic charac,
                                                  double tol,
                                                  bool diameterSymbol,
                                                  MaterialCondition matCond,
                                                  const std::string& primary,
                                                  const std::string& secondary,
                                                  const std::string& tertiary,
                                                  double attachX, double attachY,
                                                  double boxX, double boxY) {
    FeatureControlFrame fcf;
    fcf.characteristic = charac;
    fcf.toleranceValue = tol;
    fcf.hasDiameterSymbol = diameterSymbol;
    fcf.materialCondition = matCond;
    fcf.primaryDatum = primary;
    fcf.secondaryDatum = secondary;
    fcf.tertiaryDatum = tertiary;
    fcf.attachX = attachX;
    fcf.attachY = attachY;
    fcf.boxX = boxX;
    fcf.boxY = boxY;
    return fcf;
}

DrawingDocument DrawingEngine::GenerateMultiSheetDocument(const TopoDS_Shape& shape,
                                                          const std::vector<BOMItem>& bomItems,
                                                          const std::string& partName) {
    DrawingDocument doc;
    doc.title = partName + " Production Multi-Sheet Drafting";

    // SHEET 1: General Assembly Overview, Isometric View & BOM Table
    DrawingSheet sheet1;
    sheet1.sheetIndex = 1;
    sheet1.sheetName = "Sheet 1 (Assembly & BOM)";
    sheet1.titleBlock.partName = partName;
    sheet1.titleBlock.sheetNumber = "1 / 3";
    sheet1.titleBlock.title = "GENERAL ASSEMBLY & BILL OF MATERIALS";

    DrawingView isoView = ProjectView(shape, ViewType::Isometric, 120.0, 150.0, 1.2);
    BOMBalloon b1{1, 100.0, 140.0, 60.0, 200.0, 6.0};
    BOMBalloon b2{2, 140.0, 160.0, 180.0, 220.0, 6.0};
    isoView.balloons.push_back(b1);
    isoView.balloons.push_back(b2);
    sheet1.views.push_back(isoView);

    sheet1.bomTable.originX = 250.0;
    sheet1.bomTable.originY = 25.0;
    sheet1.bomTable.items = bomItems;
    doc.sheets.push_back(sheet1);

    // SHEET 2: Component Orthographic Projections, Dimensions & GD&T
    DrawingSheet sheet2 = GenerateStandardDrawingSheet(shape, partName, 1.0);
    sheet2.sheetIndex = 2;
    sheet2.sheetName = "Sheet 2 (Orthographic & GD&T)";
    sheet2.titleBlock.sheetNumber = "2 / 3";
    sheet2.titleBlock.title = "PART DETAILS & GEOMETRIC TOLERANCING (GD&T)";

    FeatureControlFrame fcfPos = CreateGDTFrame(
        GDTCharacteristic::POSITION, 0.05, true, MaterialCondition::MMC,
        "A", "B", "C", 100.0, 80.0, 120.0, 100.0
    );
    FeatureControlFrame fcfFlat = CreateGDTFrame(
        GDTCharacteristic::FLATNESS, 0.02, false, MaterialCondition::NONE,
        "", "", "", 100.0, 220.0, 120.0, 240.0
    );
    FeatureControlFrame fcfPerp = CreateGDTFrame(
        GDTCharacteristic::PERPENDICULARITY, 0.03, false, MaterialCondition::MMC,
        "A", "B", "", 240.0, 80.0, 260.0, 100.0
    );

    if (!sheet2.views.empty()) sheet2.views[0].gdtFrames.push_back(fcfPos);
    if (sheet2.views.size() > 1) sheet2.views[1].gdtFrames.push_back(fcfFlat);
    if (sheet2.views.size() > 2) sheet2.views[2].gdtFrames.push_back(fcfPerp);

    doc.sheets.push_back(sheet2);

    // SHEET 3: Section Cuts, Enlarged Detail Views & Auxiliary Views
    DrawingSheet sheet3;
    sheet3.sheetIndex = 3;
    sheet3.sheetName = "Sheet 3 (Sections & Details)";
    sheet3.titleBlock.partName = partName;
    sheet3.titleBlock.sheetNumber = "3 / 3";
    sheet3.titleBlock.title = "SECTIONS, DETAILS & AUXILIARY VIEWS";

    DrawingView secView = CreateSectionView(shape, 0.0, 110.0, 160.0, 1.2);
    DrawingView detView = CreateDetailView(secView, 110.0, 160.0, 20.0, 2.0, 270.0, 180.0, "A");
    DrawingView auxView = CreateAuxiliaryView(shape, 1.0, 1.0, 1.0, 270.0, 70.0, 1.0);

    sheet3.views.push_back(secView);
    sheet3.views.push_back(detView);
    sheet3.views.push_back(auxView);
    doc.sheets.push_back(sheet3);

    return doc;
}

std::string DrawingDocument::ToMultiSheetSVG() const {
    std::ostringstream ss;
    for (size_t s = 0; s < sheets.size(); ++s) {
        ss << "<!-- SHEET " << (s + 1) << ": " << sheets[s].sheetName << " -->\n";
        ss << sheets[s].ToSVG() << "\n\n";
    }
    return ss.str();
}

std::string DrawingDocument::ToMultiSheetDXF() const {
    if (sheets.empty()) return "";
    return sheets[0].ToDXF();
}

std::string DrawingSheet::ToDXF() const {
    std::ostringstream dxf;
    dxf << std::fixed << std::setprecision(4);

    // Header & Layer Table
    dxf << "0\nSECTION\n2\nHEADER\n0\nENDSEC\n";
    dxf << "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nLAYER\n";
    // Layer BORDER (Color 7: White)
    dxf << "0\nLAYER\n2\nBORDER\n70\n0\n62\n7\n6\nCONTINUOUS\n0\n";
    // Layer VISIBLE (Color 7: White)
    dxf << "LAYER\n2\nVISIBLE\n70\n0\n62\n7\n6\nCONTINUOUS\n0\n";
    // Layer HIDDEN (Color 8: Gray)
    dxf << "LAYER\n2\nHIDDEN\n70\n0\n62\n8\n6\nDASHED\n0\n";
    // Layer DIMENSIONS (Color 4: Cyan)
    dxf << "LAYER\n2\nDIMENSIONS\n70\n0\n62\n4\n6\nCONTINUOUS\n0\n";
    dxf << "ENDTAB\n0\nENDSEC\n";

    dxf << "0\nSECTION\n2\nENTITIES\n";

    auto addLine = [&](const std::string& layer, double x1, double y1, double x2, double y2) {
        dxf << "0\nLINE\n8\n" << layer << "\n";
        dxf << "10\n" << x1 << "\n20\n" << y1 << "\n30\n0.0\n";
        dxf << "11\n" << x2 << "\n21\n" << y2 << "\n31\n0.0\n";
    };

    // Border
    addLine("BORDER", 20.0, 10.0, sheetWidth - 10.0, 10.0);
    addLine("BORDER", sheetWidth - 10.0, 10.0, sheetWidth - 10.0, sheetHeight - 10.0);
    addLine("BORDER", sheetWidth - 10.0, sheetHeight - 10.0, 20.0, sheetHeight - 10.0);
    addLine("BORDER", 20.0, sheetHeight - 10.0, 20.0, 10.0);

    // Title Block
    double tbW = 160.0;
    double tbH = 45.0;
    double tbX = sheetWidth - 10.0 - tbW;
    double tbY = 10.0;
    addLine("BORDER", tbX, tbY, tbX + tbW, tbY);
    addLine("BORDER", tbX + tbW, tbY, tbX + tbW, tbY + tbH);
    addLine("BORDER", tbX + tbW, tbY + tbH, tbX, tbY + tbH);
    addLine("BORDER", tbX, tbY + tbH, tbX, tbY);

    // Views
    for (const auto& view : views) {
        for (const auto& seg : view.segments) {
            std::string layer = seg.isHidden ? "HIDDEN" : "VISIBLE";
            addLine(layer, seg.x1, seg.y1, seg.x2, seg.y2);
        }
        for (const auto& dim : view.dimensions) {
            addLine("DIMENSIONS", dim.x1, dim.y1, dim.x2, dim.y2);
        }
    }

    dxf << "0\nENDSEC\n0\nEOF\n";
    return dxf.str();
}

} // namespace androcad