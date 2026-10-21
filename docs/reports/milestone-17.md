# Milestone 17 Report: Threads, Fasteners & Associative Sketch Projection

**Status:** Completed  
**Release:** `v0.17.0-beta`  
**Date:** 2026-10-21  

---

## 1. Executive Summary

Milestone 17 delivers comprehensive mechanical engineering fastener, thread modeling, and associative sketching capabilities to AndroCAD:
1. **ISO Standard Hole Wizard (ISO 273 / ISO 262):** Automated generation of Counterbore, Countersink, Clearance, and Tapped holes with parametric standard metric parameters (M3 to M12).
2. **Modeled 3D Helical Threaded Fasteners:** Full physical helical thread tooth profile generation with exact pitch and outer diameter compliance for bolts and studs.
3. **Associative 3D-to-2D Sketch Edge Projection:** Projects 3D B-Rep boundary edges and circular arcs directly onto active sketch coordinate systems $(U, V)$ preserving geometric associativity.
4. **Cubic B-Spline Curve Generator:** Multi-pole cubic Catmull-Rom / B-Spline interpolation achieving $C^2$ second-derivative curvature continuity with tangent handle controls.
5. **Dual Device UX:** Landscape tablet CAD workstation layout and portrait mobile smartphone bottom-sheet parameter wizard.

---

## 2. Technical Implementation

### Core C++ Geometry Engine (`FastenerEngine.h` & `FastenerEngine.cpp`)
- **Fastener Standards Database:** Encapsulates ISO metric tables for nominal diameter, standard coarse pitch, clearance hole diameter, counterbore diameter/depth, and 90° countersink angles.
- **Hole Cut Boolean Operations:** Generates stepped cylindrical/conical cutters and carves solids via `BRepAlgoAPI_Cut`.
- **Modeled Helical Threads:** Sweeps pitch-interval tooth geometry along bolt shafts via `BRepAlgoAPI_Fuse`.
- **Sketch Edge Projection:** Extracts 3D curve parameters from `BRepAdaptor_Curve` and computes plane projection $U = (\vec{P} - \vec{O}) \cdot \hat{X}$, $V = (\vec{P} - \vec{O}) \cdot \hat{Y}$.
- **Cubic B-Spline Engine:** Evaluates $C^2$ continuous cubic spline polynomials through control points.

### Host CTest Verification (`test_fastener_engine.cpp`)
All 5 automated unit and regression tests pass:
1. `TestCounterboreHole`: Validates M6 counterbore hole cut (6.6mm clearance, 11mm cb dia, 6mm depth) on 50x50x20 block matching analytical volume deduction.
2. `TestCountersinkHole`: Validates M5 90° countersink hole cut.
3. `TestThreadedBolt`: Validates M8x30 bolt with 20mm thread length, exactly 16 helical teeth, pitch 1.25mm, and outer diameter > 8.0mm.
4. `TestEdgeProjection`: Validates projection of all 12 box edges onto 2D plane coordinate space.
5. `TestCubicBSpline`: Validates 5-pole cubic spline curve with 50 samples and arc-length calculation.

---

## 3. UI Screenshots

### Landscape Tablet Workstation UI
![AndroCAD Tablet Threads UI](../images/androcad_tablet_threads_ui.png)

### Portrait Mobile Phone UI
![AndroCAD Mobile Phone Threads UI](../images/androcad_phone_threads_ui.png)
