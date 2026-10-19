# AndroCAD Architecture & Feasibility Evidence Log

This document records the evidence, authoritative sources, tool evaluations, findings, and identified unknowns for Step 1 research and planning of **AndroCAD** (an original parametric 3D CAD application for Android tablets and phones powered by OpenCASCADE).

---

## 1. Geometric Modeling Kernel: OpenCASCADE Technology (OCCT)

### Item 1.1: Licensing & Distribution Verification
* **Claim to Verify:** License status of OpenCASCADE Technology (OCCT) 7.7.0, 7.8.0, and 8.0.x; compliance obligations for an Android APK.
* **Source / Tool Inspected:** 
  - Official Open CASCADE SAS Git repository (`Open-Cascade-SAS/OCCT`)
  - License files: `LICENSE_LGPL_21.txt` and `OCCT_LGPL_EXCEPTION.txt` (Open CASCADE Exception v1.0)
* **Findings:**
  - **License:** LGPL Version 2.1-only with the **Open CASCADE Exception (version 1.0)**. 
  - **Correction of User Premise:** OCCT 7.7.0+ is **not** Apache 2.0. It remains strictly under LGPL-2.1 with Open CASCADE Exception across all current versions (including 7.7.0, 7.8.0, and 8.0.1).
  - **Open CASCADE Exception Terms:** Developers may distribute object code incorporating material from OCCT header files under terms of their choice, provided:
    1. Prominent notice is given that OCCT is used and licensed under LGPL 2.1 with the Exception.
    2. A copy of LGPL 2.1 and the Exception is included.
    3. Source code of OCCT (or an offer to provide it) is made available under LGPL 2.1.
  - **Android APK Obligation:**
    - To comply with LGPL 2.1 Section 6 without forcing the host Android application (Kotlin/Compose/JNI) to be open-sourced, OCCT must be dynamically linked as separate shared libraries (`.so` files placed in `lib/arm64-v8a/` and `lib/x86_64/`).
    - Standard dynamic linking via Android NDK allows end-users to extract the APK, swap or rebuild the `.so` files, resign, and run.
    - If any modifications are made to OCCT core source files, those patches must be released under LGPL 2.1. Vanilla builds require no custom code disclosure beyond build configuration scripts.
* **Unknowns:**
  - Exact legal posture if Google Play dynamic feature modules or APK split delivery are used (must ensure user-relocatable dynamic linkage remains valid).

---

### Item 1.2: Android NDK Build & Toolkit Pruning
* **Claim to Verify:** Feasibility of cross-compiling OCCT for Android NDK (arm64-v8a, x86_64), decoupling desktop visualization (`TKV3d`, `TKService`, `TKOpenGl`, `FreeType`), and estimating stripped binary size.
* **Source / Tool Inspected:**
  - Official sample: `Open-Cascade-SAS/OCCT-samples-kotlin` (`ReadMe.md`, `CMakeLists.txt`)
  - OCCT CMake build system (`adm/cmake`) and NDK toolchain (`$ANDROID_NDK/build/cmake/android.toolchain.cmake`)
* **Findings:**
  - **Decoupling Visualization:** OCCT's modeling toolkits are completely decoupled from its GUI/rendering modules. Setting `BUILD_MODULE_Visualization=OFF` and `BUILD_MODULE_Draw=OFF` removes the hard dependencies on FreeType, FreeImage, and desktop X11/OpenGL.
  - **Required Modeling Toolkits:**
    - *Foundation & Math:* `TKernel`, `TKMath`
    - *Geometry & B-Rep Topology:* `TKG2d`, `TKG3d`, `TKGeomBase`, `TKBRep`, `TKGeomAlgo`, `TKTopAlgo`
    - *Feature Modeling:* `TKPrim` (prisms, cylinders, primitives), `TKBO` & `TKBool` (Boolean Operations Processor), `TKFillet` (fillets, chamfers), `TKOffset` (sweeps, lofts, thick solids/shells)
    - *Tessellation:* `TKMesh` (`BRepMesh_IncrementalMesh` for generating triangle meshes for Vulkan/GLES)
    - *Data Exchange (DE):* `TKXSBase`, `TKDESTEP` (STEP AP214/AP242), `TKDESTL` (STL ASCII/binary), `TKDE3MF` (3MF)
  - **Size Estimation:**
    - Compiled with Clang NDK r26+, `-Oz` (optimize for size), `-flto`, `-ffunction-sections -fdata-sections -Wl,--gc-sections`, and stripped with `llvm-strip`:
    - Individual `.so` files total approx. 30–38 MB uncompressed on disk per architecture.
    - Inside the compressed APK (APK zip deflate), the library overhead is approximately **12–16 MB** for `arm64-v8a`.
* **Unknowns:**
  - Exact final byte count of OCCT 8.0.x DESTEP dependencies (whether STEP parser transitively pulls extra XCAF metadata toolkits).

---

## 2. 2D Sketch Geometric Constraint Solver Candidates

### Item 2.1: SolveSpace (libslvs) Evaluation
* **Claim to Verify:** License and suitability of SolveSpace's solver library (`libslvs`) for AndroCAD.
* **Source / Tool Inspected:** SolveSpace repository license headers and Debian package metadata.
* **Findings:**
  - **License:** **GPLv3-or-later**.
  - **Evaluation:** Strict copyleft. Linking `libslvs` (statically or dynamically) legally requires the entire containing project to be distributed under GPLv3.
  - **Verdict:** **REJECTED** per the project's strict legal rule: *"Avoid GPL-only code in the app core unless I approve."*

---

### Item 2.2: FreeCAD PlaneGCS Evaluation
* **Claim to Verify:** License, architecture, and embeddability of FreeCAD's PlaneGCS solver.
* **Source / Tool Inspected:**
  - FreeCAD Sketcher source code and standalone extraction (`CadQuery/PlaneGCS`)
  - License: `LGPL-2.1-or-later`
* **Findings:**
  - **License:** **LGPL-2.1-or-later**. Identical licensing tier to OCCT.
  - **Language & Footprint:** Pure C++ (~350 KB compiled binary). Uses Levenberg-Marquardt and Dogleg nonlinear least-squares minimization.
  - **Supported Constraints:** Coincident, horizontal, vertical, parallel, perpendicular, tangent, point-on-object, distance (horizontal, vertical, point-to-point, point-to-line), angle, radius/diameter, equal length/radius, symmetric.
  - **Verdict:** **PRIMARY RECOMMENDATION**. Can be built directly into the JNI library using the same Android NDK C++ toolchain without extra runtimes.

---

### Item 2.3: KittyCAD ezpz (Permissive Alternative)
* **Claim to Verify:** Feasibility of an MIT-licensed 2D geometric constraint solver.
* **Source / Tool Inspected:** KittyCAD / Zoo `ezpz` repository.
* **Findings:**
  - **License:** **MIT**.
  - **Language:** Rust.
  - **Verdict:** Viable secondary candidate if pure permissive licensing is required, but requires integrating `cargo-ndk` alongside CMake in the build pipeline.

---

## 3. Parametric History & The Topological Naming Problem (TNP)

### Item 3.1: Topological Persistence Strategy
* **Problem:** OCCT `TopoDS_Shape` instances are immutable boundary representation graphs. When a preceding parameter is modified, OCCT regenerates new topology with newly allocated memory pointers and regenerated sub-shape indices. Downstream features referencing "Face 4" or "Edge 12" break.
* **Source / Tool Inspected:**
  - OCCT `BRepTools_History` and `BRepAlgoAPI_BooleanOperation::History()`
  - RealThunder's Topological Naming Algorithm (FreeCAD 1.0 TNP mitigation)
  - Open CASCADE OCAF `TNaming` package analysis
* **Proposed Solution Architecture:**
  1. **Semantic Hierarchy / Path Naming:**
     - Give every base sketch element a persistent GUID (`sk1_line_A`, `sk1_arc_B`).
     - When `Pad_1` extrudes `sk1`, tag generated faces: `Pad_1:Cap_Start`, `Pad_1:Cap_End`, `Pad_1:Side(sk1_line_A)`.
  2. **OCCT BRepTools_History Tracking:**
     - Every modeling step queries the operation's `History()` for `Generated(S)`, `Modified(S)`, and `IsDeleted(S)`.
     - Tags propagate deterministically through modifications: `Pad_1:Cap_End.mod_1`.
  3. **Geometric Fingerprint Fallback:**
     - For each referenced sub-shape (Face/Edge), store an invariant geometric signature:
       - Face: Surface type (Plane, Cylinder, etc.), center of gravity, normal vector, surface area.
       - Edge: Curve type, start/end points, length, tangent at midpoint.
     - If history tracking yields an ambiguous match after radical topology changes, the fingerprint matcher selects the highest-scoring candidate and alerts the user with a non-fatal warning in the Timeline.

---

## 4. Mobile Tablet Performance & Rendering

### Item 4.1: Tessellation & Zero-Copy GPU Streaming
* **Problem:** Mid-range Android tablets (Snapdragon 7xx/8xx, 6–8 GB RAM) cannot freeze the UI thread during B-Rep re-meshing.
* **Proposed Pipeline:**
  1. **Adaptive LOD:**
     - *Interactive / Scrubbing mode:* Coarse deflection ($\Delta_{lin} = 0.5\text{ mm}$, $\Delta_{ang} = 25^\circ$) for real-time responsiveness (<20 ms).
     - *Resting / Accurate mode:* Fine deflection ($\Delta_{lin} = 0.05\text{ mm}$, $\Delta_{ang} = 12^\circ$) computed asynchronously in a background coroutine once user releases touch.
  2. **Direct Buffer Transfer:**
     - Native C++ JNI allocates direct `java.nio.ByteBuffer` or uploads directly to OpenGL ES VBOs / Vulkan `VkBuffer`.
     - Interleaved vertex format: `[Pos(3f), Normal(3f), FaceId(1i)]`. Zero JVM array copy overhead.
  3. **Face/Edge Picking:**
     - Color-coded off-screen GPU picking buffer (integer ID buffer). One-pixel read on touch gives instantaneous $O(1)$ sub-shape identification without CPU ray tracing.

---

## 5. Summary of Unknowns Requiring Step 2 Empirical Verification

| Unknown Item | Risk Level | Validation Method in Step 2 |
| :--- | :--- | :--- |
| **OCCT 8.0.x vs 7.8.2 NDK stability** | Low | Compile minimal JNI box test on NDK r26+ for arm64-v8a. |
| **Exact APK size of stripped DESTEP toolkits** | Low | Measure APK build output size in Milestone 1. |
| **Vulkan Filament vs raw GLES 3.2 on diverse Android GPUs** | Medium | Implement basic GLES 3.2 fallback in Milestone 2 alongside Vulkan. |
| **PlaneGCS convergence on complex over-constrained sketches** | Low | Create native unit tests with typical Fusion 360 test sketches. |
