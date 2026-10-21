// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_CAM_ENGINE_H
#define ANDROCAD_CAM_ENGINE_H

#include "AndroCadKernel.h"
#include <TopoDS_Shape.hxx>

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace androcad {

enum class ToolType {
    FLAT_ENDMILL,
    BALL_ENDMILL,
    BULLNOSE_ENDMILL,
    CHAMFER_MILL,
    TWIST_DRILL,
    SPOT_DRILL
};

struct ToolDefinition {
    int toolNumber{1};
    ToolType type{ToolType::FLAT_ENDMILL};
    std::string name{"10mm 4-Flute Flat Endmill"};
    double diameterMm{10.0};
    double cornerRadiusMm{0.0};
    double fluteLengthMm{25.0};
    double overallLengthMm{75.0};
    int fluteCount{4};
    double spindleRpm{8000.0};
    double cuttingFeedrateMmMin{1200.0};
    double plungeFeedrateMmMin{300.0};
};

struct StockSetup {
    double minX{0.0}, minY{0.0}, minZ{0.0};
    double maxX{100.0}, maxY{50.0}, maxZ{20.0};
    double stockTopZ{20.0};
    double clearanceZ{25.0}; // Rapid safety plane
    double retractZ{22.0};   // Feed transition plane
    int wcsIndex{54};        // G54 Work Coordinate System
};

enum class MotionType {
    RAPID,          // G00
    LINEAR_CUT,     // G01
    CIRCULAR_CW,    // G02
    CIRCULAR_CCW,   // G03
    PECK_DRILL,     // G83
    DWELL           // G04
};

struct ToolpathPoint {
    double x{0.0}, y{0.0}, z{0.0};
    MotionType motion{MotionType::RAPID};
    double feedrate{0.0};
    double arcI{0.0}, arcJ{0.0}; // Relative arc center offsets in XY plane
    double arcRadius{0.0};
};

enum class OperationType {
    FACING,
    POCKET_2D,
    CONTOUR_2D,
    ADAPTIVE_CLEARING,
    DRILLING,
    ADAPTIVE_3D,
    SCALLOP_3D,
    PARALLEL_3D
};

struct CAMOperation {
    int id{1};
    std::string name{"Operation"};
    OperationType type{OperationType::FACING};
    ToolDefinition tool;
    double stepoverMm{7.0};
    double stepdownMm{2.0};
    double topZ{20.0};
    double bottomZ{18.0};
    std::vector<ToolpathPoint> pathPoints;
    double machiningTimeSec{0.0};
    double totalDistanceMm{0.0};
};

enum class PostProcessorDialect {
    ISO_GRBL,   // Standard RS-274D / GRBL
    HAAS_CNC,   // Haas Automation VF-series
    FANUC_CNC   // Fanuc 0i / 31i
};

struct CAMVisualPath {
    std::vector<float> lineVertices; // 6 floats per line segment: (X1, Y1, Z1, R, G, B) -> (X2, Y2, Z2, R, G, B)
    uint32_t rapidCount{0};
    uint32_t cutCount{0};
    uint32_t plungeCount{0};
};

struct GCodeResult {
    bool success{false};
    std::string gcode;
    int lineCount{0};
    double totalCuttingTimeSec{0.0};
    double totalRapidTimeSec{0.0};
    double totalLengthMm{0.0};
    double materialVolumeRemovedCm3{0.0};
    double minX{0.0}, maxX{0.0};
    double minY{0.0}, maxY{0.0};
    double minZ{0.0}, maxZ{0.0};
    CAMVisualPath visualPath;
    std::string errorMessage;
};

// ==========================================
// Milestone 18: Additive 3D Print Slicer
// ==========================================

enum class InfillPattern {
    RECTILINEAR,
    HONEYCOMB,
    GRID,
    CONCENTRIC
};

struct SlicerLayerSegment {
    double x1{0.0}, y1{0.0};
    double x2{0.0}, y2{0.0};
    bool isPerimeter{true};
    int perimeterIndex{0}; // 0 = outer wall, 1 = inner wall
};

struct SlicerLayer {
    int layerIndex{0};
    double zMm{0.0};
    std::vector<SlicerLayerSegment> segments;
    double layerExtrusionMm{0.0};
    double layerPrintTimeSec{0.0};
};

struct SlicerConfig {
    double layerHeightMm{0.2};
    double initialLayerHeightMm{0.3};
    double nozzleDiameterMm{0.4};
    double filamentDiameterMm{1.75};
    double printSpeedMmS{60.0};
    double travelSpeedMmS{150.0};
    double bedTempC{60.0};
    double nozzleTempC{205.0};
    double infillDensityPercent{20.0};
    InfillPattern infillPattern{InfillPattern::RECTILINEAR};
    int wallPerimeters{2};
    bool generateSupport{false};
    double supportOverhangDeg{45.0};
};

struct SlicerResult {
    bool success{false};
    int totalLayers{0};
    double totalFilamentMm{0.0};
    double totalFilamentGrams{0.0};
    double estimatedPrintTimeMin{0.0};
    std::string gcode;
    std::vector<SlicerLayer> layers;
    std::string errorMessage;
};

class CAMEngine {
public:
    CAMEngine();
    ~CAMEngine() = default;

    // Stock & WCS configuration
    void SetStock(const StockSetup& stock);
    const StockSetup& GetStock() const { return stock_; }

    // Automatic stock generation from OCCT shape bounding box
    void AutoStockFromShape(const TopoDS_Shape& shape, double stockPaddingMm = 5.0, double topStockPaddingMm = 1.0);

    // Tool library management
    void AddTool(const ToolDefinition& tool);
    const std::vector<ToolDefinition>& GetToolLibrary() const { return toolLibrary_; }
    ToolDefinition GetTool(int toolNumber) const;

    // 2.5D CAM Toolpath Generators
    CAMOperation GenerateFacingOperation(int opId, const std::string& name,
                                         const ToolDefinition& tool,
                                         double stepoverMm = 7.0,
                                         double topZ = 21.0,
                                         double finalZ = 20.0);

    CAMOperation GeneratePocket2DOperation(int opId, const std::string& name,
                                           const ToolDefinition& tool,
                                           double pocketMinX, double pocketMaxX,
                                           double pocketMinY, double pocketMaxY,
                                           double topZ, double bottomZ,
                                           double stepdownMm = 2.0,
                                           double stepoverMm = 6.0);

    CAMOperation GenerateContour2DOperation(int opId, const std::string& name,
                                            const ToolDefinition& tool,
                                            double partMinX, double partMaxX,
                                            double partMinY, double partMaxY,
                                            double topZ, double bottomZ,
                                            double stepdownMm = 2.0);

    CAMOperation GenerateDrillOperation(int opId, const std::string& name,
                                        const ToolDefinition& drillTool,
                                        const std::vector<std::pair<double, double>>& holeLocations,
                                        double topZ, double holeDepthMm,
                                        double peckIncrementMm = 3.0);

    // ==========================================
    // Milestone 18: 3D CAM Surfacing Operations
    // ==========================================

    CAMOperation GenerateAdaptiveClearing3D(int opId, const std::string& name,
                                            const ToolDefinition& tool,
                                            double minX, double maxX,
                                            double minY, double maxY,
                                            double topZ, double bottomZ,
                                            double stepdownMm = 2.5,
                                            double optimalLoadMm = 1.5);

    CAMOperation GenerateScallop3D(int opId, const std::string& name,
                                   const ToolDefinition& tool,
                                   double minX, double maxX,
                                   double minY, double maxY,
                                   double topZ, double bottomZ,
                                   double stepoverMm = 1.0,
                                   double cuspHeightMm = 0.05);

    CAMOperation GenerateParallel3D(int opId, const std::string& name,
                                    const ToolDefinition& tool,
                                    double minX, double maxX,
                                    double minY, double maxY,
                                    double topZ, double bottomZ,
                                    double stepoverMm = 1.5,
                                    double angleDeg = 0.0);

    // ==========================================
    // Milestone 18: Additive 3D Print Slicing
    // ==========================================

    static SlicerResult SliceSolid(const TopoDS_Shape& solid,
                                   const SlicerConfig& config = SlicerConfig{});

    static SlicerResult SliceBoundingBox(double minX, double maxX,
                                         double minY, double maxY,
                                         double minZ, double maxZ,
                                         const SlicerConfig& config = SlicerConfig{});

    // Add operation to the manufacturing setup
    void AddOperation(const CAMOperation& op);
    const std::vector<CAMOperation>& GetOperations() const { return operations_; }
    void ClearOperations() { operations_.clear(); }

    // Post-processing and G-Code generation
    GCodeResult PostProcess(PostProcessorDialect dialect = PostProcessorDialect::ISO_GRBL,
                            const std::string& programName = "ANDROCAD_PART",
                            int programNumber = 1001);

    // High-level complete manufacturing recipe on a pocketed mechanical bracket
    static GCodeResult GenerateCompleteBracketSetup(double bracketLength = 100.0,
                                                   double bracketWidth = 50.0,
                                                   double bracketHeight = 20.0,
                                                   double pocketDepth = 10.0,
                                                   PostProcessorDialect dialect = PostProcessorDialect::ISO_GRBL);

    // Build visual lines for GLES viewport
    CAMVisualPath BuildVisualToolpath(const std::vector<CAMOperation>& ops);

private:
    StockSetup stock_;
    std::vector<ToolDefinition> toolLibrary_;
    std::vector<CAMOperation> operations_;

    void SetupDefaultTools();
};

} // namespace androcad

#endif // ANDROCAD_CAM_ENGINE_H
