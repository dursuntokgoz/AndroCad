// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_DRAWING_ENGINE_H
#define ANDROCAD_DRAWING_ENGINE_H

#include "AndroCadKernel.h"
#include <TopoDS_Shape.hxx>

#include <string>
#include <vector>

namespace androcad {

enum class ViewType {
    Front = 0,     // Projection along -Y onto X-Z plane
    Top = 1,       // Projection along -Z onto X-Y plane
    Right = 2,     // Projection along +X onto Y-Z plane
    Isometric = 3, // Axonometric 30-degree isometric projection
    Section = 4    // Planar slice along cutting plane
};

struct ProjectedSegment {
    double x1, y1;
    double x2, y2;
    bool isHidden{false};   // false = solid visible line, true = dashed hidden line
    bool isCenterline{false};
};

struct DimensionCallout {
    std::string text;
    double x1, y1;
    double x2, y2;
    double offset;
    bool isDiameter{false};
};

struct DrawingView {
    ViewType type{ViewType::Front};
    std::string name{"FRONT VIEW"};
    double originX{0.0};    // Center placement on sheet (mm)
    double originY{0.0};    // Center placement on sheet (mm)
    double scale{1.0};
    double width{0.0};      // 2D bounding width (mm)
    double height{0.0};     // 2D bounding height (mm)
    std::vector<ProjectedSegment> segments;
    std::vector<DimensionCallout> dimensions;
};

struct TitleBlockInfo {
    std::string title{"ANDROCAD ENGINEERING DRAWING"};
    std::string partName{"Component1"};
    std::string author{"dursuntokgoz"};
    std::string date{"2026-10-21"};
    std::string scaleStr{"1:1"};
    std::string sheetNumber{"1 / 1"};
    bool isThirdAngleProjection{true};
};

struct DrawingSheet {
    double sheetWidth{420.0};   // Standard A3 landscape width (mm)
    double sheetHeight{297.0};  // Standard A3 landscape height (mm)
    TitleBlockInfo titleBlock;
    std::vector<DrawingView> views;

    // Export formats
    std::string ToSVG() const;
    std::string ToDXF() const;
};

class DrawingEngine {
public:
    // 1. Projects a 3D TopoDS_Shape into a specific 2D orthographic or isometric view
    static DrawingView ProjectView(const TopoDS_Shape& shape,
                                   ViewType type,
                                   double originX,
                                   double originY,
                                   double scale = 1.0);

    // 2. Generates a Section Cut view slicing the shape along a plane (Y = cutY)
    static DrawingView CreateSectionView(const TopoDS_Shape& shape,
                                         double cutY,
                                         double originX,
                                         double originY,
                                         double scale = 1.0);

    // 3. Generates a standard complete 4-View Engineering Drawing Sheet (Top, Front, Right, Isometric + Title Block)
    static DrawingSheet GenerateStandardDrawingSheet(const TopoDS_Shape& shape,
                                                     const std::string& partName = "Part1",
                                                     double scale = 1.0);

    // 4. Adds automatic bounding dimensions to views
    static void AddOverallDimensions(DrawingSheet& sheet);
};

} // namespace androcad

#endif // ANDROCAD_DRAWING_ENGINE_H