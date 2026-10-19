# AndroCAD

[![CI](https://github.com/dursuntokgoz/AndroCad/actions/workflows/ci.yml/badge.svg)](https://github.com/dursuntokgoz/AndroCad/actions/workflows/ci.yml)
[![OCCT Build](https://github.com/dursuntokgoz/AndroCad/actions/workflows/occt-build.yml/badge.svg)](https://github.com/dursuntokgoz/AndroCad/actions/workflows/occt-build.yml)

An original parametric 3D CAD application engineered for Android tablets and phones, powered by Open CASCADE Technology (OCCT) and PlaneGCS constraint solver.

---

## Overview

AndroCAD brings a desktop-grade parametric CAD experience to touch-first and stylus-enabled Android devices:
* **Geometry Kernel:** Open CASCADE Technology (OCCT), cross-compiled for Android ARM64 and x86_64 architectures.
* **Sketch Constraint Solver:** PlaneGCS (2D geometric constraint solver).
* **Parametric Engine:** Directed Acyclic Graph (DAG) feature timeline with topological naming resolution.
* **Rendering Architecture:** Native C++ graphics pipeline (OpenGL ES 3.x with Vulkan backend) and zero-copy GPU vertex streaming from B-Rep tessellation.
* **UI & Workflow:** Jetpack Compose layout following standard CAD conventions (Solid, Sketch, Surface, Mesh, Inspect; Browser tree, Timeline, ViewCube, Navigation Bar).

---

## Legal & Licensing Information

* **Application Code:**  
  Copyright (c) 2026 dursuntokgoz. **All rights reserved.**  
  No permission is granted to use, copy, modify, merge, publish, distribute, sublicense, or sell copies of this proprietary software without the prior written permission of the copyright holder.

* **Third-Party Open-Source Components:**  
  Third-party components, including Open CASCADE Technology (OCCT), PlaneGCS, Eigen, Boost, and UI iconography, are licensed under their respective open-source licenses. OCCT and PlaneGCS are dynamically linked as standalone shared libraries (`.so`).  
  For complete licensing terms, notices, and dynamic relinking instructions, please refer to [`THIRD_PARTY_NOTICES.md`](./THIRD_PARTY_NOTICES.md) and [`docs/licenses.md`](./docs/licenses.md).

---

## Architecture & Documentation

* [Roadmap & Decisions Log](docs/roadmap.md)
* [Evidence & Feasibility Log](docs/evidence.md)
* [Third-Party Licenses & Compliance](docs/licenses.md)
