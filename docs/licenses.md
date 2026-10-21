# Third-Party Licenses & Compliance Ledger

> [!WARNING]
> **Legal Architecture Notice: Dynamic Linking & Relinking**  
> AndroCAD’s proprietary application code links against LGPL components (OCCT and PlaneGCS) strictly as separate, dynamically loaded shared libraries (`.so` files via JNI). Source code for any modifications to LGPL components, along with complete build scripts and relinking instructions allowing users to replace the `.so` binaries inside the application, are provided to satisfy Section 6 of LGPL 2.1.  
> **Status:** *Needs legal review before commercial release.*

---

## Dependency & License Matrix

| Component | Role | Exact SPDX Identifier | Upstream Origin | Obligations for Android Distribution |
| :--- | :--- | :--- | :--- | :--- |
| **Open CASCADE Technology (OCCT)** | 3D Geometric Kernel | `LGPL-2.1-only WITH OCCT-exception-1.0` | [Open CASCADE SAS](https://dev.opencascade.org/) | Must be built as dynamic `.so` libraries. Notice and copy of LGPL 2.1 + OCCT exception bundled. Relinking instructions provided. Patches to OCCT must be published. |
| **PlaneGCS** | 2D Geometric Constraint Solver | `LGPL-2.1-or-later` | [FreeCAD Project](https://github.com/FreeCAD/FreeCAD) / [CadQuery](https://github.com/CadQuery/PlaneGCS) | Built as dynamic `.so` or isolated JNI bridge. Notice and copy of LGPL 2.1 bundled. Source of modifications must be shared. |
| **Eigen 3** | Linear algebra backend for PlaneGCS | `MPL-2.0` | [Eigen Project](https://eigen.tuxfamily.org/) | Permissive file-level copyleft. Source of modified Eigen files must be made available. Headers incorporated into library. |
| **Boost C++ Libraries** | Geometry / Math utilities | `BSL-1.0` | [Boost Project](https://www.boost.org/) | Permissive license. Attribution in source/documentation. Binary distribution does not require attribution in executable. |
| **LLVM libc++ (Android NDK)** | C++ Standard Library runtime | `Apache-2.0 WITH LLVM-exception` | [LLVM Project](https://llvm.org/) / Android NDK | Standard Android NDK STL (`c++_shared`). Allows linking with proprietary applications without copyleft infection. |

---

## UI Iconography & Assets

| Asset Collection | Scope | Exact SPDX Identifier | Origin / Attribution |
| :--- | :--- | :--- | :--- |
| **Lucide Icons** | Application UI, tree, file operations | `ISC` | [Lucide](https://lucide.dev/) (Copyright (c) Lucide Contributors) |
| **Material Symbols** | Standard dialogs, system controls | `Apache-2.0` | [Google Material Symbols](https://fonts.google.com/icons) |
| **Custom CAD Icons** | CAD features (Extrude, Revolve, Fillet, Loft, Sweep, Assembly, Joints, Flange, Flat Pattern, Relief, Drawing Views, Section Cut, Dimensions, Materials, Mass Properties, Center of Gravity, Mesh Repair, Mesh to Solid, Mesh Smooth, FEA Solve, FEA Constraints, Structural Loads, Safety Factor, CAM Facing, CAM Pocketing, CAM Contour, CAM Drilling, G-Code Post, Rectangular Pattern, Circular Pattern, Mirror, Draft Angle, ViewCube, etc.) | `All Rights Reserved` (Proprietary) | 100% original custom SVG drawings created specifically for AndroCAD. No proprietary CAD vendor icons used. |

---

## Components Evaluated and Prohibited

| Component | License | Evaluation & Status |
| :--- | :--- | :--- |
| **SolveSpace (`libslvs`)** | `GPL-3.0-or-later` | **STRICTLY PROHIBITED from App Core.** Static or dynamic linkage would impose copyleft GPLv3 terms on the host application. |
