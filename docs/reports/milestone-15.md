# Milestone 15 Report: Subtractive CAM & G-Code Post-Processing

**Date:** 2026-10-21  
**Branch:** `milestone-15`  
**PR:** #15  
**Beta Release:** `v0.15.0-beta`  
**Author:** dursuntokgoz  

---

## 1. Executive Summary

Milestone 15 completes the core Autodesk Fusion 360 workflow parity roadmap by introducing the dedicated **Manufacture (CAM)** workspace in AndroCAD. Powered by the native C++ `CAMEngine`, AndroCAD provides full 2.5D and 3-axis CNC subtractive machining toolpath generation, tool library management (flat endmills, ballnose mills, chamfer mills, twist drills), stock and Work Coordinate System (WCS G54) setup, multi-depth planar facing, 2D pocket clearing with concentric climb milling, 2D perimeter contour finishing with cutter radius compensation, canned deep-hole peck drilling cycles (G83), and post-processors generating standard ISO RS-274D / GRBL, Haas Automation, and Fanuc CNC machine G-Code.

---

## 2. Architecture & Computational Machining Engine

The CAM engine is implemented in `core-kernel/src/main/cpp/include/CAMEngine.h` and `src/CAMEngine.cpp`:

| CAM Component | Algorithm / Formulation | Functional Role |
| :--- | :--- | :--- |
| **Tool Library** | `ToolDefinition` (diameter, corner radius, flute count, RPM, cutting & plunge feedrates) | Manages standardized tooling geometry and cutting parameter regimes. |
| **Stock & WCS Setup** | `AutoStockFromShape` via OCCT `BRepBndLib` + padding offsets | Establishes stock envelope, clearance retract plane ($Z_{safe}$), and G54 origin. |
| **Facing Generator** | Bidirectional zigzag raster lines with tangential lead-in/lead-out arcs | Planes rough stock down to true datum surfaces without plunge marks. |
| **2D Pocket Clearing** | Inward boundary offsets with concentric climb milling loops | Roughs out interior cavities across multiple depth increments ($\Delta Z$). |
| **2D Contour Finishing** | Tool centerline perimeter offsets + tangent lead-in/lead-out arcs | Machine side walls to final dimensional tolerances without dwell blemishes. |
| **Drilling Cycles** | Automated detection of hole depths; triggers G83 peck drilling for $H > 2.5D$ | Safely clears chips during deep drilling with rapid retract and re-approach clearance. |
| **Multi-Dialect Post-Processor**| ISO GRBL, Haas Automation (`O1001`, `G43 H...`), Fanuc 0i / 31i (`G28 G91 Z0.`) | Exports ready-to-run CNC machine programs verified against travel envelopes. |
| **Simulation & Verification**| Travel envelope bounding checks ($X, Y, Z$), cycle time estimation, and MRR | Validates collision-free machining time and theoretical volume removal rate. |
| **Visual 3D Toolpaths** | Line segments tagged by motion: yellow (G00 rapid), cyan/green (G01 cut), red (plunge) | Direct interactive wireframe display inside Android GLES viewport. |

---

## 3. Host CTest Verification (`test_cam_engine.cpp`)

A comprehensive 5-test suite validates tool library setup, facing, pocketing, contouring, deep-hole peck drilling, and multi-dialect G-Code generation:

1. **Test 1: Tool Library & Auto Stock Setup**
   * Verified default tool library: 10mm flat endmill, 6mm flat endmill, 6mm ballnose, chamfer drill, 5mm twist drill.
   * Auto-stock from OCCT $100 \times 50 \times 20\text{ mm}$ block correctly added $5\text{ mm}$ lateral padding and $1\text{ mm}$ top stock ($Z = 21\text{ mm}$, clearance $Z = 31\text{ mm}$).
2. **Test 2: Facing Operation**
   * Verified raster path generation with lead-in and lead-out in air outside stock boundary ($X < 0\text{ mm}$ and $X > 100\text{ mm}$).
   * Verified safe rapid retract to clearance plane at operation end.
3. **Test 3: 2D Pocket & Contour Toolpaths**
   * Tested pocketing inside cavity ($[20, 80] \times [15, 45]\text{ mm}$, depth $10\text{ mm}$ with $2.5\text{ mm}$ stepdown). Confirmed all 4 depth passes reached target $Z = 10.0\text{ mm}$.
   * Tested perimeter contouring with 6mm tool: confirmed tool centerline correctly offset by $R = 3.0\text{ mm}$ ($X = -3.0\text{ mm}$ and $103.0\text{ mm}$).
4. **Test 4: Deep Hole Peck Drilling Cycles**
   * Tested hole depth $22\text{ mm}$ ($> 2.5 \times 5\text{ mm}$ tool diameter).
   * Verified chip-breaking retracts to retract plane ($Z = 22.0\text{ mm}$) and re-approach clearance ($+0.5\text{ mm}$) for multiple peck steps per hole.
5. **Test 5: Complete G-Code Post-Processor**
   * Generated complete bracket machining setup (Facing, Pocket, Contour, Spot Drill, Peck Drill).
   * Verified syntax for ISO/GRBL, Haas CNC (`O1001`, `G43 H1`), and Fanuc CNC dialects.
   * Verified $> 100$ G-code lines, positive cutting and rapid travel times, volume removed $> 0\text{ cm}^3$, and visual line vertex streams.

---

## 4. UI & JNI Integration

* **Manufacture (CAM) Workspace:** Added dedicated **Manufacture (CAM)** tab in `MainActivity.kt`:
  * `Generate G-Code (GRBL)`: Instant ISO/GRBL standard CNC program generation.
  * `Haas G-Code`: Industrial Haas VF-series CNC post-processor output.
  * `Fanuc G-Code`: Fanuc 0i/31i CNC post-processor output.
  * `Facing Pass`: Planar top surface cleanup operation.
  * `2D Pocketing`: Concentric cavity clearing.
  * `2D Contour`: Wall finishing with cutter compensation.
  * `Peck Drilling`: Multi-pass peck drill cycles for through-holes.
* **CAM Statistics HUD:** Displays operations count, line count, estimated cutting time, rapid time, path distance, and material volume removed.
* **Kotlin Data Contracts:** `CAMGCodeInfo`, `CAMPostDialect`, `NativeKernel.generateGCode()`, and `NativeKernel.getCAMVisualLines()`.

---

## 5. Original CAD SVG Icons

Original SVG drawings created in `docs/icons/`:
* `cam_facing.svg`: Surface milling cutter facing a rectangular stock block.
* `cam_pocket.svg`: Concentric pocket toolpath loops clearing an internal cavity.
* `cam_contour.svg`: Outer wall finishing toolpath with cutter radius offset.
* `cam_drill.svg`: Twist drill penetrating workpiece with peck depth increments.
* `cam_gcode.svg`: G-Code program file with coordinate motion paths.
