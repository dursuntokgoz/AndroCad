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
    Section = 4,   // Planar slice along cutting plane
    Detail = 5,    // Enlarged circular detail view
    Auxiliary = 6  // Oblique projection along arbitrary vector
};

enum class GDTCharacteristic {
    POSITION,
    FLATNESS,
    PERPENDICULARITY,
    PARALLELISM,
    CYLINDRICITY,
    TOTAL_RUNOUT,
    CONCENTRICITY,
    PROFILE_OF_SURFACE
};

enum class MaterialCondition {
    NONE, // Regardless of Feature Size (RFS)
    MMC,  // Maximum Material Condition (M)
    LMC   // Least Material Condition (L)
};

struct FeatureControlFrame {
    GDTCharacteristic characteristic{GDTCharacteristic::POSITION};
    double toleranceValue{0.05};
    bool hasDiameterSymbol{true};
    MaterialCondition materialCondition{MaterialCondition::MMC};
    std::string primaryDatum{"A"};
    MaterialCondition primaryDatumModifier{MaterialCondition::NONE};
    std::string secondaryDatum{"B"};
    MaterialCondition secondaryDatumModifier{MaterialCondition::MMC};
    std::string tertiaryDatum{"C"};
    MaterialCondition tertiaryDatumModifier{MaterialCondition::NONE};

    // Leader and placement coordinates (mm)
    double attachX{0.0}, attachY{0.0};
    double boxX{0.0}, boxY{0.0};
    double boxWidth{45.0}, boxHeight{8.0};

    std::string FormattedString() const;
    std::string ToSVG() const;
};

struct BOMItem {
    uint32_t itemNumber{1};
    std::string partNumber{"AC-1001"};
    std::string description{"Mounting Bracket Base"};
    uint32_t quantity{1};
    std::string materialName{"Structural Steel"};
    double unitMassGrams{450.0};
    double totalMassGrams{450.0};
};

struct BOMBalloon {
    uint32_t itemNumber{1};
    double attachX{0.0}, attachY{0.0};     // Point on component
    double balloonX{0.0}, balloonY{0.0};   // Center of balloon circle
    double radius{6.0};                   // Standard balloon radius (mm)
    std::string ToSVG() const;
};

struct BOMTable {
    double originX{260.0};  // Placement on drawing sheet (mm)
    double originY{20.0};
    double rowHeight{7.0};
    std::vector<BOMItem> items;
    std::string ToSVG() const;
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
    std::vector<FeatureControlFrame> gdtFrames;
    std::vector<BOMBalloon> balloons;
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
    uint32_t sheetIndex{1};
    std::string sheetName{"Sheet 1"};
    double sheetWidth{420.0};   // Standard A3 landscape width (mm)
    double sheetHeight{297.0};  // Standard A3 landscape height (mm)
    TitleBlockInfo titleBlock;
    std::vector<DrawingView> views;
    BOMTable bomTable;

    // Export formats
    std::string ToSVG() const;
    std::string ToDXF() const;
};

struct DrawingDocument {
    std::string title{"AndroCad Multi-Sheet Vector Drafting Document"};
    std::vector<DrawingSheet> sheets;

    std::string ToMultiSheetSVG() const;
    std::string ToMultiSheetDXF() const;
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

    // 3. Generates an enlarged circular Detail View from an existing view
    static DrawingView CreateDetailView(const DrawingView& parentView,
                                        double centerX, double centerY,
                                        double radiusMm,
                                        double scaleMultiplier,
                                        double placementX, double placementY,
                                        const std::string& detailLabel = "A");

    // 4. Generates an Auxiliary View projecting along an oblique 3D direction vector
    static DrawingView CreateAuxiliaryView(const TopoDS_Shape& shape,
                                           double dirX, double dirY, double dirZ,
                                           double originX, double originY,
                                           double scale = 1.0);

    // 5. Generates a standard complete 4-View Engineering Drawing Sheet (Top, Front, Right, Isometric + Title Block)
    static DrawingSheet GenerateStandardDrawingSheet(const TopoDS_Shape& shape,
                                                     const std::string& partName = "Part1",
                                                     double scale = 1.0);

    // 6. Generates a multi-sheet production drafting document (Sheet 1: Assembly & BOM, Sheet 2: Dimensions & GD&T, Sheet 3: Details & Sections)
    static DrawingDocument GenerateMultiSheetDocument(const TopoDS_Shape& shape,
                                                      const std::vector<BOMItem>& bomItems,
                                                      const std::string& partName = "Mechanical Assembly");

    // 7. Adds automatic bounding dimensions to views
    static void AddOverallDimensions(DrawingSheet& sheet);

    // 8. Helper to construct a standard GD&T Feature Control Frame
    static FeatureControlFrame CreateGDTFrame(GDTCharacteristic charac,
                                              double tol,
                                              bool diameterSymbol,
                                              MaterialCondition matCond,
                                              const std::string& primary,
                                              const std::string& secondary = "",
                                              const std::string& tertiary = "",
                                              double attachX = 0.0, double attachY = 0.0,
                                              double boxX = 0.0, double boxY = 0.0);
};

} // namespace androcad

#endif // ANDROCAD_DRAWING_ENGINE_H