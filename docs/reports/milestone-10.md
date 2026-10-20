# Milestone 10 Report: Sheet Metal Engine, K-Factor Bends & Flat Pattern Unfolding

**Date:** 2026-10-21  
**Branch:** `milestone-10`  
**PR:** #10  
**Beta Release:** `v0.10.0-beta`  
**Author:** dursuntokgoz  

---

## 1. Executive Summary

Milestone 10 introduces the dedicated **Sheet Metal** modeling workspace and unfolding pipeline in AndroCAD, providing functional parity with Fusion 360's **Sheet Metal** environment. The new `SheetMetalEngine` delivers analytical K-factor bend deduction calculations, automated 3D parametric sheet metal part generation (with realistic inner and outer cylindrical bend fillets), 2D flat pattern layout unfolding, and export to standard AutoCAD DXF format for CNC laser and waterjet manufacturing.

---

## 2. Architecture & Mathematical Foundation

The sheet metal mechanics are implemented in `core-kernel/src/main/cpp/include/SheetMetalEngine.h` and `src/SheetMetalEngine.cpp`:

| Sheet Metal Concept | Formula / OCCT Implementation | Functional Role |
| :--- | :--- | :--- |
| **Neutral Axis & K-Factor** | $R_{\text{neutral}} = R + K \cdot T$ ($K \approx 0.44$) | Determines the shift of the unstretched neutral fiber inside the bent material. |
| **Bend Allowance ($BA$)** | $BA = \theta \cdot (R + K \cdot T)$ | Arc length of the neutral sheet fiber along a bend of angle $\theta$ (radians). |
| **Outside Setback ($OSSB$)** | $OSSB = \tan(\frac{\theta}{2}) \cdot (R + T)$ | Distance from the vertex corner intersection to the tangent point of the bend. |
| **Bend Deduction ($BD$)** | $BD = 2 \cdot OSSB - BA$ | Analytical material compensation subtracted from the outside flange dimensions. |
| **Flat Length ($L_{\text{flat}}$)** | $L_{\text{flat}} = \sum L_{\text{legs}} - \sum BD$ | Total unfolded blank length required to achieve nominal formed dimensions. |
| **3D U-Bracket Solid** | Closed wire with circular fillets + `BRepPrimAPI_MakePrism` | Watertight 3D solid with constant wall thickness $T$ and tangential bend arcs $R$. |
| **Flat Pattern Layout** | Analytical layout with bend centerlines | Unfolded 2D geometry with bend line IDs, angles, directions (UP/DOWN), and radii. |
| **DXF Generation** | DXF R12 ASCII generator with layers | Distinct `CUT` (white perimeter) and `BEND` (red centerlines + text callouts) layers. |

---

## 3. Host CTest Verification (`test_sheet_metal.cpp`)

The unit test suite verifies analytical precision, OCCT shape topology, unfolding math, and DXF output:

1. **Test 1: K-Factor Bend Deduction & Flat Length Calculations**
   * Sheet thickness $T = 2.0\text{ mm}$, inner radius $R = 2.0\text{ mm}$, K-factor $K = 0.44$, bend angle $\theta = 90^\circ$ ($\pi/2$).
   * Calculated Bend Allowance: $BA = 4.5239\text{ mm}$ (exact).
   * Outside Setback: $OSSB = 4.0000\text{ mm}$.
   * Bend Deduction: $BD = 2 \times 4.0 - 4.5239 = 3.4761\text{ mm}$.
   * Flat length for $40\text{ mm} + 100\text{ mm}$ legs: $L_{\text{flat}} = 136.5239\text{ mm}$ ($|\Delta| < 10^{-4}\text{ mm}$).
2. **Test 2: Sheet Metal Base Plate Generation**
   * $100 \times 50 \times 2\text{ mm}$ base plate.
   * `BRepCheck_Analyzer`: Valid solid shape.
   * Measured mesh volume: $10,000.0\text{ mm}^3$ (100% exact).
3. **Test 3: 3D U-Bracket Solid Construction with Cylindrical Fillets**
   * Outer base length $100\text{ mm}$, flange height $40\text{ mm}$, width $50\text{ mm}$, thickness $2\text{ mm}$, inner radius $2\text{ mm}$.
   * Constructed from continuous $C^1$ closed profile extruded along $Y$.
   * `BRepCheck_Analyzer`: Valid watertight solid.
4. **Test 4: U-Bracket Unfolding to Flat Pattern & Bend Line Layout**
   * Analytical flat length: $100 + 2 \times 40 - 2 \times 3.4761 = 173.0478\text{ mm}$.
   * Measured flat length: $173.0478\text{ mm}$.
   * Measured flat width: $50.0000\text{ mm}$.
   * Exactly 2 bend lines identified with direction UP ($90^\circ$).
   * Distance between bend centerlines: $100 - BD = 96.5239\text{ mm}$.
5. **Test 5: Flat Pattern 2D DXF Export**
   * Generated DXF header, tables, and entities.
   * Verified presence of `CUT` layer (4 boundary lines) and `BEND` layer (centerlines + bend notes).

---

## 4. UI & JNI Integration

* **Sheet Metal Ribbon Workspace:** Added dedicated **Sheet Metal** workspace tab in `MainActivity.kt`:
  * `U-Bracket`: Generates and displays a 3D U-Bracket solid in the viewport with real-time volume calculation and adds a timeline feature.
  * `Unfold Pattern`: Unwraps the 3D sheet metal part into its 2D flat blank dimensions and bend lines.
  * `Export DXF`: Generates an industrial 2D DXF file with `CUT` and `BEND` layers for CNC sheet fabrication.
* **Kotlin Data Contracts:** `SheetMetalRule`, `SheetMetalBendLine`, `SheetMetalFlatPattern`, and `NativeKernel.createUBracket()`, `unfoldUBracket()`, `exportSheetMetalDXF()`.

---

## 5. Original CAD SVG Icons

Original SVG icons drawn to represent sheet metal operations in `docs/icons/`:
* `flange.svg`: Bent sheet metal flange with inner bend radius highlight.
* `flat_pattern.svg`: Unfolded flat sheet blank with dashed bend fold lines and cut boundary.
* `bend_relief.svg`: Corner notch relief with bend line indicator.
