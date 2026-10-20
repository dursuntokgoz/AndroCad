# Milestone 11 Report: 2D Engineering Drawings, Orthographic Views & Dimension Drafting

**Date:** 2026-10-21  
**Branch:** `milestone-11`  
**PR:** #11  
**Beta Release:** `v0.11.0-beta`  
**Author:** dursuntokgoz  

---

## 1. Executive Summary

Milestone 11 introduces the **2D Engineering Drawings & Drafting Sheet** workspace in AndroCAD, providing functional parity with Fusion 360's **Drawing** environment. The new `DrawingEngine` delivers automated multi-view orthographic projection (Front, Top, Right, and Isometric views in standard 3rd Angle Projection), Hidden Line Removal (HLR) distinguishing solid visible contours from dashed hidden edges, section cut slicing with 45-degree cross-hatching, dimension callouts with leader extension lines, standard ANSI/ISO A3 sheet borders, an engineering Title Block, and high-precision export to vector **SVG** and **AutoCAD DXF** formats.

---

## 2. Architecture & Drafting Engine

The drafting pipeline is implemented in `core-kernel/src/main/cpp/include/DrawingEngine.h` and `src/DrawingEngine.cpp`:

| Drawing Concept | Technical Implementation | Functional Role |
| :--- | :--- | :--- |
| **Orthographic Projection** | Coordinate projection matrices ($u, v, \text{depth}$) | Projects 3D B-Rep boundary curves onto 2D view planes (Top, Front, Right). |
| **Isometric 3D Projection** | Axonometric $30^\circ$ affine mapping | Computes $u = (dx - dy)\cos(30^\circ), v = dz - (dx + dy)\sin(30^\circ)$. |
| **Hidden Line Removal (HLR)** | Silhouette & adjacent face-normal occlusion | Classifies edges as visible solid lines (`#1E2124`, 0.55mm) or occluded dashed lines (`#888888`, 0.3mm). |
| **Section Cut View** | `BRepAlgoAPI_Section` + $45^\circ$ diagonal hatching | Slices 3D models along a cutting plane and fills bounded cut areas with standard cross-hatching. |
| **Dimension Drafting** | Leader lines, dimension line, arrowheads, callout text | Automatically calculates overall bounding dimensions (Width, Depth, Height) in millimeters. |
| **Title Block & Frame** | ISO / ANSI A3 standard sheet layout ($420 \times 297\text{ mm}$) | Drawing borders (20mm binding, 10mm margins), title block cells (Title, Part, Author, Date, Scale, 3rd Angle). |
| **Multi-Format Export** | Vector SVG & AutoCAD DXF | W3C SVG vector graphic export and layered DXF (`BORDER`, `VISIBLE`, `HIDDEN`, `DIMENSIONS`). |

---

## 3. Host CTest Verification (`test_drawing_engine.cpp`)

The test suite validates projection coordinates, silhouette detection, section cutting, and file generation:

1. **Test 1: 2D Multi-View Orthographic & Isometric Projections**
   * $100 \times 50 \times 30\text{ mm}$ box model.
   * Front View dimensions: $100.0 \times 30.0\text{ mm}$ (exact).
   * Top View dimensions: $100.0 \times 50.0\text{ mm}$ (exact).
   * Right View dimensions: $50.0 \times 30.0\text{ mm}$ (exact).
   * Isometric View: non-empty projected segments spanning axonometric boundaries.
2. **Test 2: Hidden Line Removal & Silhouette Line Classification**
   * Classifies boundary edges into visible contours and occluded backface lines.
3. **Test 3: Section Cut Plane Slicing & 45° Cross-Hatching**
   * Slices model at $Y = 30.0\text{ mm}$.
   * Extracts section boundary and generates 45-degree cross-hatching lines inside the bounded profile.
4. **Test 4: Dimension Callouts & Leader Lines**
   * Verifies automatic placement of horizontal (Width: $120.0\text{ mm}$) and vertical (Height: $35.0\text{ mm}$) callouts with leader extension lines.
5. **Test 5: Vector SVG & AutoCAD DXF Sheet Export**
   * Validates generated SVG markup with `<svg`, `<rect`, `<line`, `<text`, and title block information.
   * Validates generated DXF with `BORDER`, `VISIBLE`, `HIDDEN`, and `DIMENSIONS` layers.

---

## 4. UI & JNI Integration

* **Drawing Ribbon Workspace:** Added dedicated **Drawing** workspace tab in `MainActivity.kt`:
  * `4-View Sheet`: Generates a standard A3 drawing sheet containing Top, Front, Right, and Isometric views with bounding dimensions.
  * `Section Cut`: Slices the active 3D body along a cutting plane and displays cross-hatched section details.
  * `Export SVG`: Exports the drawing sheet as an industry-standard vector SVG document.
  * `Export DXF`: Exports the drawing sheet as a multi-layer AutoCAD DXF file.
* **Kotlin Data Contracts:** `DrawingSheetInfo`, and `NativeKernel.exportDrawingSVG()`, `exportDrawingDXF()`.

---

## 5. Original CAD SVG Icons

Original SVG icons drawn to represent drawing and drafting operations in `docs/icons/`:
* `drawing_view.svg`: Engineering sheet with 4-view layout and title block.
* `section_view.svg`: Cross-hatched section cut with cutting plane line and direction arrows.
* `dimension_linear.svg`: Linear dimension line with extension leaders, dual arrowheads, and numerical text marker.
