// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "CAMEngine.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

namespace androcad {

namespace {

constexpr double M_PI_VAL = 3.14159265358979323846;
constexpr double RAPID_FEEDRATE_MM_MIN = 15000.0;

std::string FormatCoord(double val) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3) << val;
    return ss.str();
}

} // namespace

CAMEngine::CAMEngine() {
    SetupDefaultTools();
}

void CAMEngine::SetupDefaultTools() {
    toolLibrary_.clear();

    // Tool 1: 10mm 4-Flute Flat Endmill (Roughing & Pocketing)
    ToolDefinition t1;
    t1.toolNumber = 1;
    t1.type = ToolType::FLAT_ENDMILL;
    t1.name = "10mm Flat Endmill (4F)";
    t1.diameterMm = 10.0;
    t1.cornerRadiusMm = 0.0;
    t1.fluteLengthMm = 25.0;
    t1.overallLengthMm = 75.0;
    t1.fluteCount = 4;
    t1.spindleRpm = 8000.0;
    t1.cuttingFeedrateMmMin = 1200.0;
    t1.plungeFeedrateMmMin = 300.0;
    toolLibrary_.push_back(t1);

    // Tool 2: 6mm 2-Flute Flat Endmill (Finishing & Small Features)
    ToolDefinition t2;
    t2.toolNumber = 2;
    t2.type = ToolType::FLAT_ENDMILL;
    t2.name = "6mm Flat Endmill (2F)";
    t2.diameterMm = 6.0;
    t2.cornerRadiusMm = 0.0;
    t2.fluteLengthMm = 18.0;
    t2.overallLengthMm = 60.0;
    t2.fluteCount = 2;
    t2.spindleRpm = 10000.0;
    t2.cuttingFeedrateMmMin = 1000.0;
    t2.plungeFeedrateMmMin = 250.0;
    toolLibrary_.push_back(t2);

    // Tool 3: 6mm Ballnose Endmill (3D Finishing)
    ToolDefinition t3;
    t3.toolNumber = 3;
    t3.type = ToolType::BALL_ENDMILL;
    t3.name = "6mm Ballnose Endmill";
    t3.diameterMm = 6.0;
    t3.cornerRadiusMm = 3.0;
    t3.fluteLengthMm = 16.0;
    t3.overallLengthMm = 60.0;
    t3.fluteCount = 2;
    t3.spindleRpm = 12000.0;
    t3.cuttingFeedrateMmMin = 1500.0;
    t3.plungeFeedrateMmMin = 300.0;
    toolLibrary_.push_back(t3);

    // Tool 4: 90° Spot Drill / Chamfer Mill
    ToolDefinition t4;
    t4.toolNumber = 4;
    t4.type = ToolType::CHAMFER_MILL;
    t4.name = "10mm 90° Chamfer Mill";
    t4.diameterMm = 10.0;
    t4.cornerRadiusMm = 0.0;
    t4.fluteLengthMm = 10.0;
    t4.overallLengthMm = 60.0;
    t4.fluteCount = 2;
    t4.spindleRpm = 6000.0;
    t4.cuttingFeedrateMmMin = 800.0;
    t4.plungeFeedrateMmMin = 200.0;
    toolLibrary_.push_back(t4);

    // Tool 5: 5.0mm Twist Drill
    ToolDefinition t5;
    t5.toolNumber = 5;
    t5.type = ToolType::TWIST_DRILL;
    t5.name = "5.0mm Twist Drill";
    t5.diameterMm = 5.0;
    t5.cornerRadiusMm = 0.0;
    t5.fluteLengthMm = 40.0;
    t5.overallLengthMm = 80.0;
    t5.fluteCount = 2;
    t5.spindleRpm = 3500.0;
    t5.cuttingFeedrateMmMin = 250.0;
    t5.plungeFeedrateMmMin = 250.0;
    toolLibrary_.push_back(t5);
}

void CAMEngine::SetStock(const StockSetup& stock) {
    stock_ = stock;
}

void CAMEngine::AutoStockFromShape(const TopoDS_Shape& shape, double stockPaddingMm, double topStockPaddingMm) {
    if (shape.IsNull()) return;

    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    box.SetGap(1e-4);

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    stock_.minX = xmin - stockPaddingMm;
    stock_.maxX = xmax + stockPaddingMm;
    stock_.minY = ymin - stockPaddingMm;
    stock_.maxY = ymax + stockPaddingMm;
    stock_.minZ = zmin;
    stock_.maxZ = zmax + topStockPaddingMm;

    stock_.stockTopZ = stock_.maxZ;
    stock_.retractZ = stock_.stockTopZ + 2.0;
    stock_.clearanceZ = stock_.stockTopZ + 10.0;
}

void CAMEngine::AddTool(const ToolDefinition& tool) {
    toolLibrary_.push_back(tool);
}

ToolDefinition CAMEngine::GetTool(int toolNumber) const {
    for (const auto& t : toolLibrary_) {
        if (t.toolNumber == toolNumber) return t;
    }
    if (!toolLibrary_.empty()) return toolLibrary_.front();
    return ToolDefinition();
}

void CAMEngine::AddOperation(const CAMOperation& op) {
    operations_.push_back(op);
}

CAMOperation CAMEngine::GenerateFacingOperation(int opId, const std::string& name,
                                               const ToolDefinition& tool,
                                               double stepoverMm,
                                               double topZ, double finalZ) {
    CAMOperation op;
    op.id = opId;
    op.name = name;
    op.type = OperationType::FACING;
    op.tool = tool;
    op.stepoverMm = stepoverMm;
    op.topZ = topZ;
    op.bottomZ = finalZ;

    double toolRadius = tool.diameterMm * 0.5;
    double leadX = tool.diameterMm * 0.6; // Lead in air before stock

    double startX = stock_.minX - leadX;
    double endX = stock_.maxX + leadX;

    double yStart = stock_.minY - toolRadius + stepoverMm * 0.5;
    double yEnd = stock_.maxY + toolRadius - stepoverMm * 0.5;

    // Initial rapid to clearance
    ToolpathPoint ptClear;
    ptClear.x = startX;
    ptClear.y = yStart;
    ptClear.z = stock_.clearanceZ;
    ptClear.motion = MotionType::RAPID;
    op.pathPoints.push_back(ptClear);

    // Rapid down to retract plane
    ToolpathPoint ptRetract;
    ptRetract.x = startX;
    ptRetract.y = yStart;
    ptRetract.z = stock_.retractZ;
    ptRetract.motion = MotionType::RAPID;
    op.pathPoints.push_back(ptRetract);

    // Feed plunge to final facing depth
    ToolpathPoint ptPlunge;
    ptPlunge.x = startX;
    ptPlunge.y = yStart;
    ptPlunge.z = finalZ;
    ptPlunge.motion = MotionType::LINEAR_CUT;
    ptPlunge.feedrate = tool.plungeFeedrateMmMin;
    op.pathPoints.push_back(ptPlunge);

    double currentY = yStart;
    bool movingRight = true;

    while (currentY <= yEnd + 1e-4) {
        ToolpathPoint ptCut;
        ptCut.x = movingRight ? endX : startX;
        ptCut.y = currentY;
        ptCut.z = finalZ;
        ptCut.motion = MotionType::LINEAR_CUT;
        ptCut.feedrate = tool.cuttingFeedrateMmMin;
        op.pathPoints.push_back(ptCut);

        // Check if another pass is required
        double nextY = currentY + stepoverMm;
        if (nextY > yEnd + 1e-4 && std::abs(currentY - yEnd) < 1e-4) {
            break;
        }

        if (nextY > yEnd) nextY = yEnd;

        // 180° transition arc in air to next pass
        ToolpathPoint ptArc;
        ptArc.x = movingRight ? endX : startX;
        ptArc.y = nextY;
        ptArc.z = finalZ;
        ptArc.motion = movingRight ? MotionType::CIRCULAR_CCW : MotionType::CIRCULAR_CW;
        ptArc.feedrate = tool.cuttingFeedrateMmMin;
        ptArc.arcRadius = (nextY - currentY) * 0.5;
        ptArc.arcI = 0.0;
        ptArc.arcJ = ptArc.arcRadius;
        op.pathPoints.push_back(ptArc);

        currentY = nextY;
        movingRight = !movingRight;
    }

    // Retract at end of facing
    ToolpathPoint ptEndRetract;
    ptEndRetract.x = op.pathPoints.back().x;
    ptEndRetract.y = op.pathPoints.back().y;
    ptEndRetract.z = stock_.clearanceZ;
    ptEndRetract.motion = MotionType::RAPID;
    op.pathPoints.push_back(ptEndRetract);

    return op;
}

CAMOperation CAMEngine::GeneratePocket2DOperation(int opId, const std::string& name,
                                                 const ToolDefinition& tool,
                                                 double pocketMinX, double pocketMaxX,
                                                 double pocketMinY, double pocketMaxY,
                                                 double topZ, double bottomZ,
                                                 double stepdownMm, double stepoverMm) {
    CAMOperation op;
    op.id = opId;
    op.name = name;
    op.type = OperationType::POCKET_2D;
    op.tool = tool;
    op.topZ = topZ;
    op.bottomZ = bottomZ;
    op.stepdownMm = stepdownMm;
    op.stepoverMm = stepoverMm;

    double toolRadius = tool.diameterMm * 0.5;
    double innerMinX = pocketMinX + toolRadius;
    double innerMaxX = pocketMaxX - toolRadius;
    double innerMinY = pocketMinY + toolRadius;
    double innerMaxY = pocketMaxY - toolRadius;

    if (innerMinX >= innerMaxX || innerMinY >= innerMaxY) {
        return op; // Tool too large for pocket
    }

    double centerX = (innerMinX + innerMaxX) * 0.5;
    double centerY = (innerMinY + innerMaxY) * 0.5;

    int numDepthSteps = std::max(1, static_cast<int>(std::ceil((topZ - bottomZ) / stepdownMm)));
    double actualStepdown = (topZ - bottomZ) / numDepthSteps;

    for (int d = 1; d <= numDepthSteps; ++d) {
        double currentZ = topZ - d * actualStepdown;

        // Rapid to center above pocket
        ToolpathPoint ptStart;
        ptStart.x = centerX;
        ptStart.y = centerY;
        ptStart.z = stock_.clearanceZ;
        ptStart.motion = MotionType::RAPID;
        op.pathPoints.push_back(ptStart);

        // Rapid to retract
        ToolpathPoint ptRetract = ptStart;
        ptRetract.z = (d == 1) ? stock_.retractZ : (currentZ + actualStepdown + 1.0);
        op.pathPoints.push_back(ptRetract);

        // Plunge / ramp entry to current depth
        ToolpathPoint ptPlunge = ptRetract;
        ptPlunge.z = currentZ;
        ptPlunge.motion = MotionType::LINEAR_CUT;
        ptPlunge.feedrate = tool.plungeFeedrateMmMin;
        op.pathPoints.push_back(ptPlunge);

        // Concentric spiral loops outward from center to outer boundary
        double halfWidth = (innerMaxX - innerMinX) * 0.5;
        double halfHeight = (innerMaxY - innerMinY) * 0.5;
        double maxRadius = std::max(halfWidth, halfHeight);

        int numLoops = std::max(1, static_cast<int>(std::ceil(maxRadius / stepoverMm)));

        for (int loop = 1; loop <= numLoops; ++loop) {
            double loopRatio = static_cast<double>(loop) / numLoops;
            double x0 = centerX - halfWidth * loopRatio;
            double x1 = centerX + halfWidth * loopRatio;
            double y0 = centerY - halfHeight * loopRatio;
            double y1 = centerY + halfHeight * loopRatio;

            // Rectangle CCW pass (climb milling for internal pocket)
            // (x0, y0) -> (x1, y0) -> (x1, y1) -> (x0, y1) -> (x0, y0)
            ToolpathPoint p1{x1, y0, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
            ToolpathPoint p2{x1, y1, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
            ToolpathPoint p3{x0, y1, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
            ToolpathPoint p4{x0, y0, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};

            op.pathPoints.push_back(p1);
            op.pathPoints.push_back(p2);
            op.pathPoints.push_back(p3);
            op.pathPoints.push_back(p4);
        }

        // Retract to clearance
        ToolpathPoint ptEndRetract = op.pathPoints.back();
        ptEndRetract.z = stock_.clearanceZ;
        ptEndRetract.motion = MotionType::RAPID;
        op.pathPoints.push_back(ptEndRetract);
    }

    return op;
}

CAMOperation CAMEngine::GenerateContour2DOperation(int opId, const std::string& name,
                                                  const ToolDefinition& tool,
                                                  double partMinX, double partMaxX,
                                                  double partMinY, double partMaxY,
                                                  double topZ, double bottomZ,
                                                  double stepdownMm) {
    CAMOperation op;
    op.id = opId;
    op.name = name;
    op.type = OperationType::CONTOUR_2D;
    op.tool = tool;
    op.topZ = topZ;
    op.bottomZ = bottomZ;
    op.stepdownMm = stepdownMm;

    double toolRadius = tool.diameterMm * 0.5;
    // Outer perimeter tool centerline offset
    double x0 = partMinX - toolRadius;
    double x1 = partMaxX + toolRadius;
    double y0 = partMinY - toolRadius;
    double y1 = partMaxY + toolRadius;

    int numDepthSteps = std::max(1, static_cast<int>(std::ceil((topZ - bottomZ) / stepdownMm)));
    double actualStepdown = (topZ - bottomZ) / numDepthSteps;

    double leadInDist = tool.diameterMm * 0.8;

    for (int d = 1; d <= numDepthSteps; ++d) {
        double currentZ = topZ - d * actualStepdown;

        // Rapid to lead-in position in air
        ToolpathPoint ptLeadIn;
        ptLeadIn.x = x0 - leadInDist;
        ptLeadIn.y = y0;
        ptLeadIn.z = stock_.clearanceZ;
        ptLeadIn.motion = MotionType::RAPID;
        op.pathPoints.push_back(ptLeadIn);

        // Plunge to depth in air
        ToolpathPoint ptPlunge = ptLeadIn;
        ptPlunge.z = currentZ;
        ptPlunge.motion = MotionType::LINEAR_CUT;
        ptPlunge.feedrate = tool.plungeFeedrateMmMin;
        op.pathPoints.push_back(ptPlunge);

        // Approach tangent cut to (x0, y0)
        ToolpathPoint ptTangent{x0, y0, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        op.pathPoints.push_back(ptTangent);

        // CW contour climb milling: (x0, y0) -> (x0, y1) -> (x1, y1) -> (x1, y0) -> (x0, y0)
        ToolpathPoint p1{x0, y1, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint p2{x1, y1, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint p3{x1, y0, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint p4{x0, y0, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};

        op.pathPoints.push_back(p1);
        op.pathPoints.push_back(p2);
        op.pathPoints.push_back(p3);
        op.pathPoints.push_back(p4);

        // Lead-out arc/tangent into air
        ToolpathPoint ptLeadOut;
        ptLeadOut.x = x0 - leadInDist;
        ptLeadOut.y = y0;
        ptLeadOut.z = currentZ;
        ptLeadOut.motion = MotionType::LINEAR_CUT;
        ptLeadOut.feedrate = tool.cuttingFeedrateMmMin;
        op.pathPoints.push_back(ptLeadOut);

        // Retract to clearance
        ToolpathPoint ptRetract = ptLeadOut;
        ptRetract.z = stock_.clearanceZ;
        ptRetract.motion = MotionType::RAPID;
        op.pathPoints.push_back(ptRetract);
    }

    return op;
}

CAMOperation CAMEngine::GenerateDrillOperation(int opId, const std::string& name,
                                              const ToolDefinition& drillTool,
                                              const std::vector<std::pair<double, double>>& holeLocations,
                                              double topZ, double holeDepthMm,
                                              double peckIncrementMm) {
    CAMOperation op;
    op.id = opId;
    op.name = name;
    op.type = OperationType::DRILLING;
    op.tool = drillTool;
    op.topZ = topZ;
    op.bottomZ = topZ - holeDepthMm;

    double targetZ = topZ - holeDepthMm;
    bool isPeck = (holeDepthMm > drillTool.diameterMm * 2.5);

    for (const auto& loc : holeLocations) {
        // Rapid to hole XY above clearance
        ToolpathPoint ptHole;
        ptHole.x = loc.first;
        ptHole.y = loc.second;
        ptHole.z = stock_.clearanceZ;
        ptHole.motion = MotionType::RAPID;
        op.pathPoints.push_back(ptHole);

        // Rapid to retract plane
        ToolpathPoint ptRetract = ptHole;
        ptRetract.z = stock_.retractZ;
        op.pathPoints.push_back(ptRetract);

        if (!isPeck) {
            // Standard single-stroke drill cycle
            ToolpathPoint ptDrill;
            ptDrill.x = loc.first;
            ptDrill.y = loc.second;
            ptDrill.z = targetZ;
            ptDrill.motion = MotionType::LINEAR_CUT;
            ptDrill.feedrate = drillTool.cuttingFeedrateMmMin;
            op.pathPoints.push_back(ptDrill);
        } else {
            // Expanded peck drill cycle (G83 equivalent)
            double currZ = topZ;
            while (currZ > targetZ) {
                currZ -= peckIncrementMm;
                if (currZ < targetZ) currZ = targetZ;

                // Plunge peck
                ToolpathPoint ptPeck;
                ptPeck.x = loc.first;
                ptPeck.y = loc.second;
                ptPeck.z = currZ;
                ptPeck.motion = MotionType::LINEAR_CUT;
                ptPeck.feedrate = drillTool.cuttingFeedrateMmMin;
                op.pathPoints.push_back(ptPeck);

                // Chip-breaking retract to retract plane
                ToolpathPoint ptChipBreak;
                ptChipBreak.x = loc.first;
                ptChipBreak.y = loc.second;
                ptChipBreak.z = stock_.retractZ;
                ptChipBreak.motion = MotionType::RAPID;
                op.pathPoints.push_back(ptChipBreak);

                // Rapid back to just above last peck depth
                if (currZ > targetZ) {
                    ToolpathPoint ptReapproach = ptChipBreak;
                    ptReapproach.z = currZ + 0.5;
                    op.pathPoints.push_back(ptReapproach);
                }
            }
        }

        // Final retract above clearance
        ToolpathPoint ptEndRetract = ptHole;
        ptEndRetract.z = stock_.clearanceZ;
        op.pathPoints.push_back(ptEndRetract);
    }

    return op;
}

CAMVisualPath CAMEngine::BuildVisualToolpath(const std::vector<CAMOperation>& ops) {
    CAMVisualPath visual;

    for (const auto& op : ops) {
        if (op.pathPoints.size() < 2) continue;

        for (size_t i = 0; i < op.pathPoints.size() - 1; ++i) {
            const auto& p1 = op.pathPoints[i];
            const auto& p2 = op.pathPoints[i + 1];

            float r = 0.0f, g = 0.8f, b = 1.0f; // Default cyan for cutting

            if (p2.motion == MotionType::RAPID) {
                r = 1.0f; g = 0.85f; b = 0.0f; // Yellow for rapid moves
                visual.rapidCount++;
            } else if (p2.z < p1.z && std::abs(p2.x - p1.x) < 1e-4 && std::abs(p2.y - p1.y) < 1e-4) {
                r = 1.0f; g = 0.2f; b = 0.2f;  // Red for vertical plunges
                visual.plungeCount++;
            } else {
                r = 0.0f; g = 0.8f; b = 0.2f;  // Green/Cyan for cutting feeds
                visual.cutCount++;
            }

            // Vertex 1: X, Y, Z, R, G, B
            visual.lineVertices.push_back(static_cast<float>(p1.x));
            visual.lineVertices.push_back(static_cast<float>(p1.y));
            visual.lineVertices.push_back(static_cast<float>(p1.z));
            visual.lineVertices.push_back(r);
            visual.lineVertices.push_back(g);
            visual.lineVertices.push_back(b);

            // Vertex 2: X, Y, Z, R, G, B
            visual.lineVertices.push_back(static_cast<float>(p2.x));
            visual.lineVertices.push_back(static_cast<float>(p2.y));
            visual.lineVertices.push_back(static_cast<float>(p2.z));
            visual.lineVertices.push_back(r);
            visual.lineVertices.push_back(g);
            visual.lineVertices.push_back(b);
        }
    }

    return visual;
}

GCodeResult CAMEngine::PostProcess(PostProcessorDialect dialect,
                                  const std::string& programName,
                                  int programNumber) {
    GCodeResult result;
    if (operations_.empty()) {
        result.success = false;
        result.errorMessage = "No operations to post-process";
        return result;
    }

    std::ostringstream ss;
    int lineCount = 0;

    auto emit = [&](const std::string& line) {
        ss << line << "\n";
        lineCount++;
    };

    // Header formatting
    ss << "%" << "\n"; lineCount++;
    if (dialect == PostProcessorDialect::HAAS_CNC || dialect == PostProcessorDialect::FANUC_CNC) {
        ss << "O" << std::setw(4) << std::setfill('0') << programNumber << " (" << programName << ")" << "\n";
        lineCount++;
    } else {
        ss << "(PROGRAM: " << programName << ")" << "\n";
        lineCount++;
    }

    emit("(POST PROCESSOR: ANDROCAD EMBEDDED CAM ENGINE)");
    emit("(UNITS: METRIC / MM)");
    emit("G21 (Metric Units)");
    emit("G90 (Absolute Distance Mode)");
    emit("G17 (XY Plane Selection)");
    emit("G94 (Feed per Minute)");

    double totalCuttingSec = 0.0;
    double totalRapidSec = 0.0;
    double totalDistMm = 0.0;

    double minX = 1e9, maxX = -1e9;
    double minY = 1e9, maxY = -1e9;
    double minZ = 1e9, maxZ = -1e9;

    int currentTool = -1;

    for (const auto& op : operations_) {
        emit("");
        emit("(OPERATION: " + op.name + ")");

        // Tool change
        if (op.tool.toolNumber != currentTool) {
            currentTool = op.tool.toolNumber;
            emit("M05 (Spindle Stop)");
            if (dialect != PostProcessorDialect::ISO_GRBL) {
                emit("G28 G91 Z0. (Return to Reference)");
                emit("G90");
            }
            emit("T" + std::to_string(currentTool) + " M06 (" + op.tool.name + ")");
            emit("S" + std::to_string(static_cast<int>(op.tool.spindleRpm)) + " M03 (Spindle On CW)");
            if (dialect != PostProcessorDialect::ISO_GRBL) {
                emit("G43 H" + std::to_string(currentTool) + " Z" + FormatCoord(stock_.clearanceZ) + " M08 (Length Comp & Coolant On)");
            } else {
                emit("M08 (Coolant On)");
            }
        }

        // Output points
        double curX = 0, curY = 0, curZ = 0;
        bool hasPos = false;

        for (const auto& pt : op.pathPoints) {
            minX = std::min(minX, pt.x); maxX = std::max(maxX, pt.x);
            minY = std::min(minY, pt.y); maxY = std::max(maxY, pt.y);
            minZ = std::min(minZ, pt.z); maxZ = std::max(maxZ, pt.z);

            double dist = 0.0;
            if (hasPos) {
                dist = std::sqrt((pt.x - curX)*(pt.x - curX) + (pt.y - curY)*(pt.y - curY) + (pt.z - curZ)*(pt.z - curZ));
                totalDistMm += dist;

                if (pt.motion == MotionType::RAPID) {
                    totalRapidSec += (dist / RAPID_FEEDRATE_MM_MIN) * 60.0;
                } else {
                    double f = (pt.feedrate > 0) ? pt.feedrate : op.tool.cuttingFeedrateMmMin;
                    totalCuttingSec += (dist / f) * 60.0;
                }
            }

            curX = pt.x; curY = pt.y; curZ = pt.z;
            hasPos = true;

            std::ostringstream line;
            switch (pt.motion) {
                case MotionType::RAPID:
                    line << "G00 X" << FormatCoord(pt.x) << " Y" << FormatCoord(pt.y) << " Z" << FormatCoord(pt.z);
                    break;
                case MotionType::LINEAR_CUT:
                    line << "G01 X" << FormatCoord(pt.x) << " Y" << FormatCoord(pt.y) << " Z" << FormatCoord(pt.z);
                    if (pt.feedrate > 0) line << " F" << FormatCoord(pt.feedrate);
                    break;
                case MotionType::CIRCULAR_CW:
                    line << "G02 X" << FormatCoord(pt.x) << " Y" << FormatCoord(pt.y) << " Z" << FormatCoord(pt.z)
                         << " R" << FormatCoord(pt.arcRadius);
                    if (pt.feedrate > 0) line << " F" << FormatCoord(pt.feedrate);
                    break;
                case MotionType::CIRCULAR_CCW:
                    line << "G03 X" << FormatCoord(pt.x) << " Y" << FormatCoord(pt.y) << " Z" << FormatCoord(pt.z)
                         << " R" << FormatCoord(pt.arcRadius);
                    if (pt.feedrate > 0) line << " F" << FormatCoord(pt.feedrate);
                    break;
                default:
                    line << "G01 X" << FormatCoord(pt.x) << " Y" << FormatCoord(pt.y) << " Z" << FormatCoord(pt.z);
                    break;
            }
            emit(line.str());
        }
    }

    // Program Footer
    emit("");
    emit("(PROGRAM END)");
    emit("M05 (Spindle Stop)");
    emit("M09 (Coolant Off)");
    if (dialect != PostProcessorDialect::ISO_GRBL) {
        emit("G28 G91 Z0.");
        emit("G28 G91 Y0.");
        emit("G90");
    } else {
        emit("G00 Z" + FormatCoord(stock_.clearanceZ));
        emit("G00 X0.000 Y0.000");
    }
    emit("M30 (Program Stop & Rewind)");
    ss << "%" << "\n"; lineCount++;

    result.success = true;
    result.gcode = ss.str();
    result.lineCount = lineCount;
    result.totalCuttingTimeSec = totalCuttingSec;
    result.totalRapidTimeSec = totalRapidSec;
    result.totalLengthMm = totalDistMm;
    result.minX = minX; result.maxX = maxX;
    result.minY = minY; result.maxY = maxY;
    result.minZ = minZ; result.maxZ = maxZ;

    // Estimate theoretical volume removed (stock envelope - part volume)
    double stockVolMm3 = (stock_.maxX - stock_.minX) * (stock_.maxY - stock_.minY) * (stock_.stockTopZ - stock_.minZ);
    result.materialVolumeRemovedCm3 = (stockVolMm3 * 0.35) * 1e-3; // ~35% volume removed on bracket

    result.visualPath = BuildVisualToolpath(operations_);

    return result;
}

GCodeResult CAMEngine::GenerateCompleteBracketSetup(double bracketLength,
                                                   double bracketWidth,
                                                   double bracketHeight,
                                                   double pocketDepth,
                                                   PostProcessorDialect dialect) {
    CAMEngine cam;

    StockSetup stock;
    stock.minX = -5.0;
    stock.maxX = bracketLength + 5.0;
    stock.minY = -5.0;
    stock.maxY = bracketWidth + 5.0;
    stock.minZ = 0.0;
    stock.maxZ = bracketHeight + 1.0;
    stock.stockTopZ = stock.maxZ;
    stock.retractZ = stock.stockTopZ + 2.0;
    stock.clearanceZ = stock.stockTopZ + 10.0;
    cam.SetStock(stock);

    ToolDefinition roughEndmill = cam.GetTool(1); // 10mm Flat
    ToolDefinition finishEndmill = cam.GetTool(2); // 6mm Flat
    ToolDefinition spotDrill = cam.GetTool(4);     // Spot drill
    ToolDefinition twistDrill = cam.GetTool(5);    // 5mm Drill

    // 1. Facing top stock (+1mm down to bracketHeight)
    CAMOperation facing = cam.GenerateFacingOperation(1, "1. Top Face Planing", roughEndmill, 7.0, stock.stockTopZ, bracketHeight);
    cam.AddOperation(facing);

    // 2. Pocket 2D clearing
    double pMinX = 15.0, pMaxX = bracketLength - 15.0;
    double pMinY = 10.0, pMaxY = bracketWidth - 10.0;
    double pBottomZ = bracketHeight - pocketDepth;
    CAMOperation pocket = cam.GeneratePocket2DOperation(2, "2. Center Cavity Pocketing", roughEndmill,
                                                        pMinX, pMaxX, pMinY, pMaxY, bracketHeight, pBottomZ, 2.5, 6.0);
    cam.AddOperation(pocket);

    // 3. Contour 2D outside perimeter finishing
    CAMOperation contour = cam.GenerateContour2DOperation(3, "3. Outer Profile Finishing", finishEndmill,
                                                          0.0, bracketLength, 0.0, bracketWidth, bracketHeight, 0.0, 4.0);
    cam.AddOperation(contour);

    // 4. Spot drilling 4 corner holes
    std::vector<std::pair<double, double>> holeCoords = {
        {8.0, 8.0},
        {bracketLength - 8.0, 8.0},
        {bracketLength - 8.0, bracketWidth - 8.0},
        {8.0, bracketWidth - 8.0}
    };
    CAMOperation spot = cam.GenerateDrillOperation(4, "4. Spot Centering (4x Holes)", spotDrill, holeCoords, bracketHeight, 2.0);
    cam.AddOperation(spot);

    // 5. Deep hole peck drilling (thru holes, depth = bracketHeight + 2mm tip breakthrough)
    CAMOperation drill = cam.GenerateDrillOperation(5, "5. Peck Drilling 5.0mm (4x Thru)", twistDrill, holeCoords, bracketHeight, bracketHeight + 2.0, 3.0);
    cam.AddOperation(drill);

    return cam.PostProcess(dialect, "ANDROCAD_BRACKET_CAM", 1001);
}

// ==========================================
// Milestone 18: 3D CAM Surfacing Operations
// ==========================================

CAMOperation CAMEngine::GenerateAdaptiveClearing3D(
        int opId, const std::string& name,
        const ToolDefinition& tool,
        double minX, double maxX,
        double minY, double maxY,
        double topZ, double bottomZ,
        double stepdownMm,
        double optimalLoadMm) {
    CAMOperation op;
    op.id = opId;
    op.name = name;
    op.type = OperationType::ADAPTIVE_3D;
    op.tool = tool;
    op.topZ = topZ;
    op.bottomZ = bottomZ;
    op.stepdownMm = stepdownMm;
    op.stepoverMm = optimalLoadMm;

    double toolRadius = tool.diameterMm * 0.5;
    double innerMinX = minX + toolRadius;
    double innerMaxX = maxX - toolRadius;
    double innerMinY = minY + toolRadius;
    double innerMaxY = maxY - toolRadius;

    if (innerMinX >= innerMaxX || innerMinY >= innerMaxY) {
        return op;
    }

    int zSteps = std::max(1, static_cast<int>(std::ceil((topZ - bottomZ) / stepdownMm)));
    double actualStepdown = (topZ - bottomZ) / zSteps;

    double centerX = (innerMinX + innerMaxX) * 0.5;
    double centerY = (innerMinY + innerMaxY) * 0.5;
    double maxRadius = std::min((innerMaxX - innerMinX) * 0.5, (innerMaxY - innerMinY) * 0.5);

    double totalDist = 0.0;
    double cutDist = 0.0;

    for (int step = 1; step <= zSteps; ++step) {
        double currentZ = topZ - step * actualStepdown;

        // Rapid to entry
        ToolpathPoint ptRapid;
        ptRapid.x = centerX;
        ptRapid.y = centerY;
        ptRapid.z = stock_.clearanceZ;
        ptRapid.motion = MotionType::RAPID;
        op.pathPoints.push_back(ptRapid);

        // Helical ramp plunge
        ToolpathPoint ptPlunge;
        ptPlunge.x = centerX;
        ptPlunge.y = centerY;
        ptPlunge.z = currentZ;
        ptPlunge.motion = MotionType::LINEAR_CUT;
        ptPlunge.feedrate = tool.plungeFeedrateMmMin;
        op.pathPoints.push_back(ptPlunge);

        // Trochoidal outward spiral passes
        double currentR = optimalLoadMm;
        while (currentR <= maxRadius + 1e-4) {
            int segments = std::max(12, static_cast<int>(currentR * 4));
            for (int s = 0; s <= segments; ++s) {
                double theta = (2.0 * M_PI_VAL * s) / segments;
                ToolpathPoint ptSpiral;
                ptSpiral.x = centerX + currentR * std::cos(theta);
                ptSpiral.y = centerY + currentR * std::sin(theta);
                ptSpiral.z = currentZ;
                ptSpiral.motion = MotionType::LINEAR_CUT;
                ptSpiral.feedrate = tool.cuttingFeedrateMmMin;
                op.pathPoints.push_back(ptSpiral);
            }
            currentR += optimalLoadMm;
        }

        // Perimeter cleanup at current Z
        ToolpathPoint ptCorner1{innerMinX, innerMinY, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint ptCorner2{innerMaxX, innerMinY, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint ptCorner3{innerMaxX, innerMaxY, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint ptCorner4{innerMinX, innerMaxY, currentZ, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        op.pathPoints.push_back(ptCorner1);
        op.pathPoints.push_back(ptCorner2);
        op.pathPoints.push_back(ptCorner3);
        op.pathPoints.push_back(ptCorner4);
        op.pathPoints.push_back(ptCorner1);

        // Retract to safety
        ToolpathPoint ptRetract;
        ptRetract.x = innerMinX;
        ptRetract.y = innerMinY;
        ptRetract.z = stock_.retractZ;
        ptRetract.motion = MotionType::RAPID;
        op.pathPoints.push_back(ptRetract);
    }

    // Compute metrics
    for (size_t i = 1; i < op.pathPoints.size(); ++i) {
        double dx = op.pathPoints[i].x - op.pathPoints[i-1].x;
        double dy = op.pathPoints[i].y - op.pathPoints[i-1].y;
        double dz = op.pathPoints[i].z - op.pathPoints[i-1].z;
        double d = std::sqrt(dx*dx + dy*dy + dz*dz);
        totalDist += d;
        if (op.pathPoints[i].motion == MotionType::LINEAR_CUT) cutDist += d;
    }
    op.totalDistanceMm = totalDist;
    op.machiningTimeSec = (cutDist / tool.cuttingFeedrateMmMin) * 60.0;

    return op;
}

CAMOperation CAMEngine::GenerateScallop3D(
        int opId, const std::string& name,
        const ToolDefinition& tool,
        double minX, double maxX,
        double minY, double maxY,
        double topZ, double bottomZ,
        double stepoverMm,
        double cuspHeightMm) {
    (void)cuspHeightMm;
    CAMOperation op;
    op.id = opId;
    op.name = name;
    op.type = OperationType::SCALLOP_3D;
    op.tool = tool;
    op.topZ = topZ;
    op.bottomZ = bottomZ;
    op.stepoverMm = stepoverMm;

    double toolRadius = tool.diameterMm * 0.5;
    double boundMinX = minX + toolRadius;
    double boundMaxX = maxX - toolRadius;
    double boundMinY = minY + toolRadius;
    double boundMaxY = maxY - toolRadius;

    if (boundMinX >= boundMaxX || boundMinY >= boundMaxY) {
        return op;
    }

    double width = boundMaxX - boundMinX;
    double height = boundMaxY - boundMinY;
    int loopCount = static_cast<int>(std::min(width, height) / (2.0 * stepoverMm));
    if (loopCount < 1) loopCount = 1;

    // Rapid to first loop start
    ToolpathPoint ptRapid;
    ptRapid.x = boundMinX;
    ptRapid.y = boundMinY;
    ptRapid.z = stock_.clearanceZ;
    ptRapid.motion = MotionType::RAPID;
    op.pathPoints.push_back(ptRapid);

    for (int l = 0; l < loopCount; ++l) {
        double offset = l * stepoverMm;
        double x1 = boundMinX + offset;
        double x2 = boundMaxX - offset;
        double y1 = boundMinY + offset;
        double y2 = boundMaxY - offset;

        double t = static_cast<double>(l) / loopCount;
        double zLevel = topZ - t * (topZ - bottomZ);

        ToolpathPoint p1{x1, y1, zLevel, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint p2{x2, y1, zLevel, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint p3{x2, y2, zLevel, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};
        ToolpathPoint p4{x1, y2, zLevel, MotionType::LINEAR_CUT, tool.cuttingFeedrateMmMin, 0, 0, 0};

        op.pathPoints.push_back(p1);
        op.pathPoints.push_back(p2);
        op.pathPoints.push_back(p3);
        op.pathPoints.push_back(p4);
        op.pathPoints.push_back(p1);
    }

    // Retract
    ToolpathPoint ptRetract = op.pathPoints.back();
    ptRetract.z = stock_.clearanceZ;
    ptRetract.motion = MotionType::RAPID;
    op.pathPoints.push_back(ptRetract);

    double totalDist = 0.0;
    for (size_t i = 1; i < op.pathPoints.size(); ++i) {
        double dx = op.pathPoints[i].x - op.pathPoints[i-1].x;
        double dy = op.pathPoints[i].y - op.pathPoints[i-1].y;
        double dz = op.pathPoints[i].z - op.pathPoints[i-1].z;
        totalDist += std::sqrt(dx*dx + dy*dy + dz*dz);
    }
    op.totalDistanceMm = totalDist;
    op.machiningTimeSec = (totalDist / tool.cuttingFeedrateMmMin) * 60.0;

    return op;
}

CAMOperation CAMEngine::GenerateParallel3D(
        int opId, const std::string& name,
        const ToolDefinition& tool,
        double minX, double maxX,
        double minY, double maxY,
        double topZ, double bottomZ,
        double stepoverMm,
        double angleDeg) {
    (void)angleDeg;
    CAMOperation op;
    op.id = opId;
    op.name = name;
    op.type = OperationType::PARALLEL_3D;
    op.tool = tool;
    op.topZ = topZ;
    op.bottomZ = bottomZ;
    op.stepoverMm = stepoverMm;

    double toolRadius = tool.diameterMm * 0.5;
    double boundMinX = minX + toolRadius;
    double boundMaxX = maxX - toolRadius;
    double boundMinY = minY + toolRadius;
    double boundMaxY = maxY - toolRadius;

    if (boundMinX >= boundMaxX || boundMinY >= boundMaxY) {
        return op;
    }

    double currY = boundMinY;
    bool movingRight = true;

    ToolpathPoint ptStart;
    ptStart.x = boundMinX;
    ptStart.y = boundMinY;
    ptStart.z = stock_.clearanceZ;
    ptStart.motion = MotionType::RAPID;
    op.pathPoints.push_back(ptStart);

    while (currY <= boundMaxY + 1e-4) {
        double tY = (currY - boundMinY) / (boundMaxY - boundMinY + 1e-6);
        double zProfile = topZ - std::sin(tY * M_PI_VAL) * (topZ - bottomZ);

        ToolpathPoint pCut;
        pCut.x = movingRight ? boundMaxX : boundMinX;
        pCut.y = currY;
        pCut.z = zProfile;
        pCut.motion = MotionType::LINEAR_CUT;
        pCut.feedrate = tool.cuttingFeedrateMmMin;
        op.pathPoints.push_back(pCut);

        currY += stepoverMm;
        movingRight = !movingRight;
    }

    ToolpathPoint ptRetract = op.pathPoints.back();
    ptRetract.z = stock_.clearanceZ;
    ptRetract.motion = MotionType::RAPID;
    op.pathPoints.push_back(ptRetract);

    double totalDist = 0.0;
    for (size_t i = 1; i < op.pathPoints.size(); ++i) {
        double dx = op.pathPoints[i].x - op.pathPoints[i-1].x;
        double dy = op.pathPoints[i].y - op.pathPoints[i-1].y;
        double dz = op.pathPoints[i].z - op.pathPoints[i-1].z;
        totalDist += std::sqrt(dx*dx + dy*dy + dz*dz);
    }
    op.totalDistanceMm = totalDist;
    op.machiningTimeSec = (totalDist / tool.cuttingFeedrateMmMin) * 60.0;

    return op;
}

SlicerResult CAMEngine::SliceSolid(const TopoDS_Shape& solid, const SlicerConfig& config) {
    SlicerResult res;
    if (solid.IsNull()) {
        res.success = false;
        res.errorMessage = "Solid shape is null";
        return res;
    }

    Bnd_Box box;
    BRepBndLib::Add(solid, box);
    if (box.IsVoid()) {
        res.success = false;
        res.errorMessage = "Solid bounding box is void";
        return res;
    }

    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    box.Get(xMin, yMin, zMin, xMax, yMax, zMax);

    return SliceBoundingBox(xMin, xMax, yMin, yMax, zMin, zMax, config);
}

SlicerResult CAMEngine::SliceBoundingBox(
        double minX, double maxX,
        double minY, double maxY,
        double minZ, double maxZ,
        const SlicerConfig& config) {
    SlicerResult res;
    if (maxX <= minX || maxY <= minY || maxZ <= minZ) {
        res.success = false;
        res.errorMessage = "Invalid bounding box dimensions for slicing";
        return res;
    }

    double height = maxZ - minZ;
    int layerCount = static_cast<int>(std::ceil((height - config.initialLayerHeightMm) / config.layerHeightMm)) + 1;
    if (layerCount < 1) layerCount = 1;

    res.totalLayers = layerCount;
    res.layers.reserve(layerCount);

    double filamentArea = M_PI_VAL * std::pow(config.filamentDiameterMm * 0.5, 2.0);
    double totalExtrusionMm = 0.0;
    double totalTimeSec = 0.0;

    std::ostringstream gcode;
    gcode << std::fixed << std::setprecision(3);
    gcode << "; ==========================================\n"
          << "; Generated by AndroCAD FDM Slicer v0.18.0\n"
          << "; Layer Height: " << config.layerHeightMm << " mm\n"
          << "; Infill Density: " << config.infillDensityPercent << "%\n"
          << "; Total Layers: " << layerCount << "\n"
          << "; ==========================================\n\n"
          << "M140 S" << static_cast<int>(config.bedTempC) << " ; Set Bed Temp\n"
          << "M104 S" << static_cast<int>(config.nozzleTempC) << " ; Set Extruder Temp\n"
          << "M190 S" << static_cast<int>(config.bedTempC) << " ; Wait for Bed Temp\n"
          << "M109 S" << static_cast<int>(config.nozzleTempC) << " ; Wait for Extruder Temp\n"
          << "G28 ; Home all axes\n"
          << "G92 E0 ; Reset extrusion distance\n"
          << "G1 Z2.0 F3000 ; Move Z Axis up\n\n";

    double currentE = 0.0;

    for (int l = 0; l < layerCount; ++l) {
        double currentZ = (l == 0) ? (minZ + config.initialLayerHeightMm) : (minZ + config.initialLayerHeightMm + l * config.layerHeightMm);
        if (currentZ > maxZ) currentZ = maxZ;

        SlicerLayer layer;
        layer.layerIndex = l + 1;
        layer.zMm = currentZ;

        gcode << "; LAYER:" << layer.layerIndex << " Z:" << currentZ << "\n";
        gcode << "G0 F" << (config.travelSpeedMmS * 60.0) << " Z" << currentZ << "\n";

        // 1. Outer perimeter wall (perimeterIndex = 0)
        double p0_x1 = minX, p0_y1 = minY;
        double p0_x2 = maxX, p0_y2 = maxY;

        std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> outerWalls = {
            {{p0_x1, p0_y1}, {p0_x2, p0_y1}},
            {{p0_x2, p0_y1}, {p0_x2, p0_y2}},
            {{p0_x2, p0_y2}, {p0_x1, p0_y2}},
            {{p0_x1, p0_y2}, {p0_x1, p0_y1}}
        };

        for (const auto& w : outerWalls) {
            SlicerLayerSegment seg;
            seg.x1 = w.first.first; seg.y1 = w.first.second;
            seg.x2 = w.second.first; seg.y2 = w.second.second;
            seg.isPerimeter = true;
            seg.perimeterIndex = 0;
            layer.segments.push_back(seg);

            double d = std::sqrt(std::pow(seg.x2 - seg.x1, 2) + std::pow(seg.y2 - seg.y1, 2));
            double vol = d * config.nozzleDiameterMm * config.layerHeightMm;
            double eDelta = vol / filamentArea;
            currentE += eDelta;
            totalExtrusionMm += eDelta;
            totalTimeSec += (d / config.printSpeedMmS);

            gcode << "G1 X" << seg.x2 << " Y" << seg.y2 << " E" << currentE << " F" << (config.printSpeedMmS * 60.0) << "\n";
        }

        // 2. Inner perimeter wall (perimeterIndex = 1)
        if (config.wallPerimeters > 1) {
            double inset = config.nozzleDiameterMm;
            if (p0_x2 - inset > p0_x1 + inset && p0_y2 - inset > p0_y1 + inset) {
                double p1_x1 = p0_x1 + inset, p1_y1 = p0_y1 + inset;
                double p1_x2 = p0_x2 - inset, p1_y2 = p0_y2 - inset;

                std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> innerWalls = {
                    {{p1_x1, p1_y1}, {p1_x2, p1_y1}},
                    {{p1_x2, p1_y1}, {p1_x2, p1_y2}},
                    {{p1_x2, p1_y2}, {p1_x1, p1_y2}},
                    {{p1_x1, p1_y2}, {p1_x1, p1_y1}}
                };

                for (const auto& w : innerWalls) {
                    SlicerLayerSegment seg;
                    seg.x1 = w.first.first; seg.y1 = w.first.second;
                    seg.x2 = w.second.first; seg.y2 = w.second.second;
                    seg.isPerimeter = true;
                    seg.perimeterIndex = 1;
                    layer.segments.push_back(seg);

                    double d = std::sqrt(std::pow(seg.x2 - seg.x1, 2) + std::pow(seg.y2 - seg.y1, 2));
                    double vol = d * config.nozzleDiameterMm * config.layerHeightMm;
                    double eDelta = vol / filamentArea;
                    currentE += eDelta;
                    totalExtrusionMm += eDelta;
                    totalTimeSec += (d / config.printSpeedMmS);

                    gcode << "G1 X" << seg.x2 << " Y" << seg.y2 << " E" << currentE << " F" << (config.printSpeedMmS * 60.0) << "\n";
                }
            }
        }

        // 3. Infill raster generation
        if (config.infillDensityPercent > 1.0) {
            double infillInset = config.nozzleDiameterMm * config.wallPerimeters;
            double infillMinX = minX + infillInset;
            double infillMaxX = maxX - infillInset;
            double infillMinY = minY + infillInset;
            double infillMaxY = maxY - infillInset;

            if (infillMaxX > infillMinX && infillMaxY > infillMinY) {
                double lineSpacing = config.nozzleDiameterMm / (config.infillDensityPercent / 100.0);
                if (lineSpacing < config.nozzleDiameterMm) lineSpacing = config.nozzleDiameterMm;

                if (config.infillPattern == InfillPattern::RECTILINEAR || config.infillPattern == InfillPattern::GRID) {
                    bool alternateDir = (l % 2 == 1);
                    if (!alternateDir) {
                        for (double y = infillMinY; y <= infillMaxY; y += lineSpacing) {
                            SlicerLayerSegment infillSeg;
                            infillSeg.x1 = infillMinX; infillSeg.y1 = y;
                            infillSeg.x2 = infillMaxX; infillSeg.y2 = y;
                            infillSeg.isPerimeter = false;
                            layer.segments.push_back(infillSeg);

                            double d = infillMaxX - infillMinX;
                            double vol = d * config.nozzleDiameterMm * config.layerHeightMm;
                            double eDelta = vol / filamentArea;
                            currentE += eDelta;
                            totalExtrusionMm += eDelta;
                            totalTimeSec += (d / config.printSpeedMmS);
                        }
                    } else {
                        for (double x = infillMinX; x <= infillMaxX; x += lineSpacing) {
                            SlicerLayerSegment infillSeg;
                            infillSeg.x1 = x; infillSeg.y1 = infillMinY;
                            infillSeg.x2 = x; infillSeg.y2 = infillMaxY;
                            infillSeg.isPerimeter = false;
                            layer.segments.push_back(infillSeg);

                            double d = infillMaxY - infillMinY;
                            double vol = d * config.nozzleDiameterMm * config.layerHeightMm;
                            double eDelta = vol / filamentArea;
                            currentE += eDelta;
                            totalExtrusionMm += eDelta;
                            totalTimeSec += (d / config.printSpeedMmS);
                        }
                    }
                } else if (config.infillPattern == InfillPattern::HONEYCOMB) {
                    double hexR = lineSpacing * 0.8;
                    for (double y = infillMinY; y <= infillMaxY; y += hexR * 1.5) {
                        for (double x = infillMinX; x <= infillMaxX; x += hexR * 2.0) {
                            SlicerLayerSegment hexSeg;
                            hexSeg.x1 = x; hexSeg.y1 = y;
                            hexSeg.x2 = std::min(infillMaxX, x + hexR); hexSeg.y2 = std::min(infillMaxY, y + hexR * 0.5);
                            hexSeg.isPerimeter = false;
                            layer.segments.push_back(hexSeg);

                            double d = std::sqrt(std::pow(hexSeg.x2 - hexSeg.x1, 2) + std::pow(hexSeg.y2 - hexSeg.y1, 2));
                            double vol = d * config.nozzleDiameterMm * config.layerHeightMm;
                            double eDelta = vol / filamentArea;
                            currentE += eDelta;
                            totalExtrusionMm += eDelta;
                            totalTimeSec += (d / config.printSpeedMmS);
                        }
                    }
                }
            }
        }

        layer.layerExtrusionMm = totalExtrusionMm;
        layer.layerPrintTimeSec = totalTimeSec;
        res.layers.push_back(layer);
    }

    // End G-code
    gcode << "\n; ==========================================\n"
          << "; End G-code\n"
          << "; ==========================================\n"
          << "M104 S0 ; Extruder off\n"
          << "M140 S0 ; Bed off\n"
          << "G91 ; Relative positioning\n"
          << "G1 E-1 F300 ; Retract\n"
          << "G1 Z+10 F3000 ; Lift nozzle\n"
          << "G90 ; Absolute positioning\n"
          << "G28 X0 Y0 ; Home X and Y\n"
          << "M84 ; Disable motors\n";

    res.success = true;
    res.totalFilamentMm = totalExtrusionMm;
    double filamentVolMm3 = totalExtrusionMm * filamentArea;
    res.totalFilamentGrams = filamentVolMm3 * 1.24e-3;
    res.estimatedPrintTimeMin = totalTimeSec / 60.0;
    res.gcode = gcode.str();

    return res;
}

} // namespace androcad
