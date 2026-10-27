# AndroCAD

<p align="center">
  <img src="docs/images/androcad_app_icon.png" width="128" height="128" alt="AndroCAD Official App Icon" /><br>
  <b>Mobile-Native Professional 3D Parametric CAD / CAE / CAM / PLM Platform for Android</b><br>
  <sub>Powered by Open CASCADE Technology (OCCT 8.0.1) & PlaneGCS Geometric Constraint Solver</sub>
</p>

<p align="center">
  <a href="README_TR.md">🇹🇷 <b>Türkçe Dokümantasyon için buraya tıklayın</b></a>
</p>

<p align="center">
  <a href="https://github.com/dursuntokgoz/AndroCad/actions/workflows/ci.yml"><img src="https://github.com/dursuntokgoz/AndroCad/actions/workflows/ci.yml/badge.svg" alt="CI Status" /></a>
  <a href="https://github.com/dursuntokgoz/AndroCad/actions/workflows/ui-tests.yml"><img src="https://github.com/dursuntokgoz/AndroCad/actions/workflows/ui-tests.yml/badge.svg" alt="UI Tests Status" /></a>
  <a href="https://github.com/dursuntokgoz/AndroCad/actions/workflows/occt-build.yml"><img src="https://github.com/dursuntokgoz/AndroCad/actions/workflows/occt-build.yml/badge.svg" alt="OCCT 8.0.1 Build" /></a>
  <img src="https://img.shields.io/badge/Kernel-OCCT%208.0.1%20B--Rep-007ACC?logo=c%2B%2B" alt="Kernel OCCT 8.0.1" />
  <img src="https://img.shields.io/badge/Solver-PlaneGCS%20LM-4CAF50" alt="Solver PlaneGCS" />
  <img src="https://img.shields.io/badge/Android%2015-16%20KB%20Page--Aligned-blue" alt="16 KB Page Aligned" />
  <img src="https://img.shields.io/badge/Security-SAF%20Zero--Permission-success" alt="SAF Zero Permission" />
  <img src="https://img.shields.io/badge/Language-Bilingual%20TR%20%7C%20EN-purple" alt="Bilingual TR/EN" />
  <img src="https://img.shields.io/badge/Release-v28.0.0-orange" alt="Release v28.0.0" />
</p>

---

## 🌟 Visual Showcase (Actual Application Screenshots)

AndroCAD delivers an intuitive desktop-grade parametric engineering suite tailored specifically for touch-first Android tablets and smartphones. Below are verified, actual screenshots captured directly from the v28.0.0 application run:

### 1. Tablet Modern Workspace (Bilingual Turkish / English)

| 🇹🇷 Tablet Workspace (Turkish - Koyu Tema) | 🇬🇧 Tablet Workspace (English - Dark Theme) |
| :---: | :---: |
| ![Tablet Workspace Turkish](docs/images/androcad_v28_tablet_turkish_dark.png) | ![Tablet Workspace English](docs/images/androcad_v28_tablet_english_dark.png) |
| *Tam Türkçe yerelleştirme, dinamik `🇹🇷 TÜRKÇE` buton geçişi, BAKIŞ KÜPÜ ve ikonlu şerit araçları.* | *Full English localization, dynamic `🇬🇧 ENGLISH` toggle, VIEW CUBE, and modern iconography.* |

---

### 2. 2D Parametric Sketcher & Mobile Adaptive Layout

| 2D Parametric Sketcher (PlaneGCS Solver) | Smartphone Adaptive Workspace & Inspector |
| :---: | :---: |
| ![2D Sketcher Solver](docs/images/androcad_v28_sketcher_solver_ui.png) | ![Phone Adaptive UI](docs/images/androcad_v28_phone_bilingual_ui.png) |
| *PlaneGCS Levenberg-Marquardt solver with exact DoF tracking, dimensional/geometric constraints.* | *Single-hand ergonomic smartphone layout with pull-up parametric inspector and quick actions.* |

---

### 3. Integrated Engineering Modules (CAE, CAM, Sheet Metal, Onboarding)

| ⚡ Linear Static & Modal FEA Simulation | 🏭 CNC CAM Toolpaths & G-Code Post-Processor |
| :---: | :---: |
| ![FEA Simulation](docs/images/androcad_v28_simulation_fea_ui.png) | ![CAM G-Code](docs/images/androcad_v28_cam_gcode_ui.png) |
| *10-node quadratic tetrahedral (Tet10) FEA solver, von Mises stress heatmap, and modal vibration.* | *3D adaptive clearing, finishing passes, toolpaths, and GRBL/Haas/Fanuc G-code exporter.* |

| 📐 Sheet Metal Unfold & Flat Pattern DXF | 🚀 Privacy-First Onboarding & Empty State |
| :---: | :---: |
| ![Sheet Metal Flat Pattern](docs/images/androcad_v28_sheet_metal_unfold_ui.png) | ![Empty State Screen](docs/images/androcad_v28_empty_state_bilingual_ui.png) |
| *Analytical K-factor bend deduction ($K=0.44$), formed 3D sheet, and 2D DXF flat pattern.* | *Bilingual welcome modal with interactive 6-step tutorial and pristine editable sample designs.* |

---

## 🚀 Key Architectural Pillars

```
┌────────────────────────────────────────────────────────────────────────┐
│                   ANDROCAD MODERN ARCHITECTURE                         │
├───────────────────────────────────┬────────────────────────────────────┤
│  JETPACK COMPOSE UI (KOTLIN)      │  CORE NATIVE KERNEL (C++20)        │
│  • Bilingual Dynamic Localization │  • Open CASCADE Technology 8.0.1   │
│  • Modern 6dp Rounded Ribbon      │  • PlaneGCS Levenberg-Marquardt    │
│  • Adaptive Tablet & Phone Panels │  • 16 KB ELF Page Alignment        │
│  • ViewCube & Touch 3D Navigation │  • 0.000 mm Exact B-Rep Precision  │
├───────────────────────────────────┴────────────────────────────────────┤
│  HARDWARE-ACCELERATED OPENGL ES 3.0+ RENDERING & RAY-PICKING           │
│  • Color-coded sub-pixel picking (Faces, Edges, Vertices)              │
│  • GL_DEPTH_TEST with zero simulated button behavior                   │
│  • Wireframe, Shaded, and Shaded+Edges display modes                   │
├────────────────────────────────────────────────────────────────────────┤
│  ANDROID 15 STORAGE ACCESS FRAMEWORK (SAF) COMPLIANCE                  │
│  • Strict zero broad storage permissions (MANAGE_EXTERNAL_STORAGE: NO) │
│  • System DocumentPicker for STEP, STL, DXF, SVG, and .androcad        │
└────────────────────────────────────────────────────────────────────────┘
```

1. **Open CASCADE Technology (OCCT 8.0.1):**  
   Powered by genuine boundary representation (B-Rep) solid modeling. Supports Boolean operations (Cut, Fuse, Common), fillets, chamfers, draft angles, shells, lofts, sweeps, and multi-body assemblies. Compiled with strict **16 KB ELF page alignment** for Android 15 compatibility and zero `TEXTREL` relocations.

2. **PlaneGCS Constraint Solver:**  
   Industrial-strength 2D geometric and dimensional constraint solver utilizing Levenberg-Marquardt optimization. Solves horizontal, vertical, perpendicular, parallel, concentric, tangent, and distance constraints with dynamic Degree of Freedom (DoF) feedback.

3. **Bilingual TR / EN Localization (`LocalizationManager`):**  
   Instant, runtime language switching between Turkish (`🇹🇷 TÜRKÇE`) and English (`🇬🇧 ENGLISH`) with reactive Compose state. Translates every tab, tool, viewcube facet, navigation control, dialog, and empty state without restarting the activity.

4. **100% Privacy & SAF Compliance:**  
   Zero broad storage permissions (`MANAGE_EXTERNAL_STORAGE` strictly absent). All file operations utilize Android's native Storage Access Framework (SAF) document pickers, ensuring full user privacy and Google Play compliance.

5. **Parametric Timeline & DAG Rebuild:**  
   Non-destructive Directed Acyclic Graph (DAG) feature tree with rollback markers, formula-driven dimensions (`width * 2 + 10`), downstream parameter recalculation, and multi-step Undo/Redo.

---

## 📊 Comprehensive Feature Status Matrix

| Module / Domain | Status | Description & Engineering Invariants |
| :--- | :---: | :--- |
| **3D Solid Modeling** | ✅ Working | Box, Cylinder, Sphere, Torus, Cone, Extrude, Revolve, Fillet, Chamfer, Hole, Shell, Combine. |
| **2D Parametric Sketcher** | ✅ Working | PlaneGCS solver; line, circle, rectangle, polygon, slot, offset, B-spline with 0 DoF tracking. |
| **Bilingual Localization** | ✅ Working | Runtime TR/EN switcher with Android resource XMLs and reactive state. |
| **Model Browser Tree** | ✅ Working | Hierarchical DAG tree (Document, Planes, Bodies, Sketches, History) with visibility toggles. |
| **Viewport & ViewCube** | ✅ Working | Single-finger orbit, two-finger pan, pinch-zoom, and 3D ViewCube orientation presets. |
| **Sheet Metal & Flat Pattern**| ✅ Working | K-factor bend deduction ($K=0.44$), flange addition, relief cuts, and 2D DXF unrolled pattern export. |
| **2D Technical Drawings** | ✅ Working | Automated 4-view orthographic projection (ISO/ANSI), HLR hidden lines, title blocks, and SVG export. |
| **Linear Static & Modal FEA** | ✅ Working | Tet10 quadratic tetrahedrals, von Mises stress heatmap, safety factor, and eigenvalue vibration modes. |
| **Subtractive CAM & G-Code** | ✅ Working | 2.5D/3D adaptive clearing, contouring, peck drilling, and post-processors for GRBL, Haas, and Fanuc. |
| **Additive Slicing (3D Print)**| ✅ Working | On-device slicer with honeycomb & grid infill, layer height controls, and nozzle thermals. |
| **Kinematic Assemblies** | ✅ Working | Rigid, revolute, cylindrical, spherical, and slider joints with dynamic collision detection. |
| **Synchronous Modeling** | ✅ Working | Live face dragging, live pattern recognition, dynamic grouping, and non-history geometry edits. |
| **Class-A Surfacing** | ✅ Working | Jerk-continuous G3 surfacing, reflection lines, Gaussian curvature diagnostics, and multi-rail sweeps. |
| **Generative Design** | ✅ Working | BESO evolutionary structural optimization, level-set boundary solver, and overhang constraints. |
| **Industry 4.0 Digital Twin** | ✅ Working | Real-time OPC-UA / MQTT CNC telemetry sync, semantic GD&T PMI annotation, and Siemens JT / 3MF. |
| **Storage Access Framework** | ✅ Working | Strict zero broad storage permissions; system document picker for STEP, STL, and `.androcad`. |

---

## 📦 Editable Parametric Sample Designs

To allow immediate hands-on exploration without creating geometry from scratch, AndroCAD includes 4 original sample models pre-bundled in `assets/samples/`:

* **L-Bracket (`l_bracket.androcad`):** 90° structural mount bracket with dual ISO fastener holes and parametric wall thickness ($\approx 36,515\text{ mm}^3$).
* **Flanged Bushing (`flanged_bushing.androcad`):** Precision sleeve bushing with central shaft bore and mounting flange ($\approx 31,476\text{ mm}^3$).
* **Phone Stand (`phone_stand.androcad`):** Desktop smartphone rest block with inclined face and cable pass-through recess ($\approx 84,672\text{ mm}^3$).
* **Perforated Plate (`perforated_plate.androcad`):** Mounting plate with a 2x2 symmetrical hole pattern and single-variable thickness scaling ($\approx 62,391\text{ mm}^3$).

> [!TIP]
> **Copy-on-Open Invariant:** Opening a sample copies it directly to application private sandbox storage (`context.filesDir/samples/`). The pre-packaged APK asset is strictly immutable. You can alter parameters, add features, and export models without affecting the template.

---

## 🛠️ Build & CI/CD Pipeline (GitHub Actions)

AndroCAD uses a 100% automated cloud CI/CD infrastructure:

* **Zero Local Setup Required:** All builds, native compilations, and Robolectric test matrices execute on GitHub Actions.
* **Workflows:**
  - [`.github/workflows/ci.yml`](.github/workflows/ci.yml): Native OCCT host tests, Android NDK `arm64-v8a` compilation, manifest permission audits, and APK signing.
  - [`.github/workflows/ui-tests.yml`](.github/workflows/ui-tests.yml): Compose UI test matrix covering phone-portrait and tablet-landscape device profiles.
  - [`.github/workflows/occt-build.yml`](.github/workflows/occt-build.yml): Automated Open CASCADE Technology (OCCT 8.0.1) cross-compilation pipeline.

### Getting the Application
1. Visit the [GitHub Actions Runs](https://github.com/dursuntokgoz/AndroCad/actions).
2. Click on the latest green run on the `main` branch.
3. Download the signed APK artifact (`androcad-debug-apk`).

---

## ⚖️ Legal & Licensing Information

* **Application Code & Proprietary Assets:**  
  Copyright (c) 2026 dursuntokgoz. **All rights reserved.**  
  No permission is granted to use, copy, modify, merge, publish, distribute, sublicense, or sell copies of this proprietary software without the prior written permission of the copyright holder.

* **Third-Party Open-Source Components:**  
  Third-party components, including Open CASCADE Technology (OCCT 8.0.1), PlaneGCS, Eigen 3, and Boost, are licensed under their respective open-source licenses. OCCT and PlaneGCS are dynamically linked as standalone shared libraries (`.so`).  
  For complete licensing terms, notices, and dynamic relinking instructions satisfying LGPL 2.1 Section 6, please refer to [`THIRD_PARTY_NOTICES.md`](./THIRD_PARTY_NOTICES.md) and [`docs/licenses.md`](./docs/licenses.md).
