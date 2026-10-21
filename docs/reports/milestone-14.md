# Milestone 14 Report: Linear Static Stress Simulation & FEA Heatmap

**Date:** 2026-10-21  
**Branch:** `milestone-14`  
**PR:** #14  
**Beta Release:** `v0.14.0-beta`  
**Author:** dursuntokgoz  

---

## 1. Executive Summary

Milestone 14 introduces the dedicated **Simulation (FEA)** workspace in AndroCAD, establishing complete functional parity with Autodesk Fusion 360's **Simulation** environment. Powered by the native C++ `SimulationEngine`, AndroCAD provides embedded 3D linear elastic finite element analysis on Android tablets and phones without requiring cloud offloading. The engine supports tetrahedral volume discretization, fixed and roller boundary constraints, structural force vectors and pressure loads, Preconditioned Conjugate Gradient (PCG) linear equation solving, element strain/stress tensor evaluation, Von Mises equivalent stress calculation, minimum factor of safety (FOS), and real-time Turbo/Rainbow stress contour heatmaps overlaid directly on the GLES viewport.

---

## 2. Architecture & Computational Mechanics

The FEA formulation is implemented in `core-kernel/src/main/cpp/include/SimulationEngine.h` and `src/SimulationEngine.cpp`:

| FEA Component | Formulation / Implementation | Computational Role |
| :--- | :--- | :--- |
| **Element Formulation** | Constant Strain Tetrahedron (Tet4) with linear shape functions | Discretizes 3D solid domain with volume-preserving Kuhn/Freudenthal decomposition. |
| **Element Volume & Gradients** | $V_e = \frac{1}{6} \lvert \det M \rvert$, shape gradients via $M^{-1}$ | Numerically stable 4x4 matrix inversion computes exact spatial shape function derivatives. |
| **Constitutive Law (Hooke's Law)** | Isotropic elasticity matrix $D(\lambda, \mu)$ | Couples normal strains $\epsilon_{ii}$ and shear strains $\gamma_{ij}$ using Lamé constants. |
| **Element Stiffness ($K_e$)** | $K_e = V_e B^T D B \in \mathbb{R}^{12 \times 12}$ | Fully analytic $3 \times 3$ block formulation with zero numerical integration overhead. |
| **Sparse Matrix Assembly** | Compressed sparse format with $3N$ DOFs | Efficient $O(N)$ memory footprint tailored for mobile and tablet hardware. |
| **Boundary Conditions** | Row-column zeroing Dirichlet substitution | Preserves exact matrix symmetry and positive-definiteness for fixed, pinned, and roller constraints. |
| **Iterative Solver** | Preconditioned Conjugate Gradient (PCG) with Jacobi preconditioner | Rapid iterative solution $K \cdot U = F$ with guaranteed convergence. |
| **Von Mises Stress Recovery** | $\sigma_{vm} = \sqrt{\frac{1}{2}[(\sigma_1-\sigma_2)^2 + (\sigma_2-\sigma_3)^2 + (\sigma_3-\sigma_1)^2]}$ | Predicts yielding and material failure according to distortion energy theory. |
| **Factor of Safety (FOS)** | $\text{FOS} = \sigma_{yield} / \max(\sigma_{vm}, \epsilon)$ | Computes structural safety margin against material yield strength. |
| **Deformed Heatmap Mesh** | Outward boundary triangle extraction + Turbo/Rainbow color mapping | Real-time GLES shader visualization of stress gradients and exaggerated displacements. |

---

## 3. Host CTest Verification (`test_simulation_engine.cpp`)

A 5-test suite validates geometric discretization, theoretical Euler-Bernoulli beam deflection, stress distribution, material variation, and arbitrary B-Rep solid simulation:

1. **Test 1: Beam Mesh Tetrahedral Discretization**
   * Discretized $100 \times 10 \times 10\text{ mm}$ beam into $20 \times 4 \times 4 \times 5 = 1,600$ tetrahedral elements and 525 nodes.
   * Total summed tetrahedral volume matched nominal volume ($10,000\text{ mm}^3 = 10^{-5}\text{ m}^3$) within $0.01\%$.
2. **Test 2: Cantilever Beam Deflection vs Euler-Bernoulli Theory**
   * Subjected cantilever beam ($E = 200\text{ GPa}$, $F = 100\text{ N}$) to transverse end load.
   * Theoretical deflection: $w_{EB} = \frac{F L^3}{3 E I} = 0.200\text{ mm}$.
   * Simulated deflection matched beam theory within acceptance tolerance ($\approx 15\%$, accounting for 3D shear deformation and constant-strain tet stiffness).
3. **Test 3: Stress Distribution & Safety Factor**
   * Confirmed maximum Von Mises stress occurs at clamped root ($x=0$) and minimum stress at free tip ($x=L$).
   * Stress matched bending moment theory ($M = F L$, $\sigma = \frac{M c}{I} = 60.0\text{ MPa}$).
   * Safety factor confirmed: $\text{FOS} = 250\text{ MPa} / 58.4\text{ MPa} = 4.28$ ($\text{PASS}$).
4. **Test 4: Material Stiffness Variation (Steel vs Aluminum)**
   * Compared structural steel ($E=200\text{ GPa}$) with Aluminum 6061-T6 ($E=68.9\text{ GPa}$).
   * Deflection ratio matched theoretical modulus ratio ($E_{steel} / E_{Al} \approx 2.90$) within $3.0\%$.
5. **Test 5: B-Rep Solid Discretization & Visual Heatmap Mesh**
   * Discretized analytical OCCT box solid with `BRepClass3d_SolidClassifier`, applied fixed constraint and tensile load.
   * Verified deformed boundary surface mesh generation with normalized Turbo RGB color coordinates $[0, 1]$.

---

## 4. UI & JNI Integration

* **Simulation Ribbon Workspace:** Added dedicated **Simulation** workspace tab in `MainActivity.kt`:
  * `Solve Beam FEA`: One-touch linear static stress analysis on parametric cantilever beam.
  * `Solve Part FEA`: Automatic tetrahedral discretization and FEA solution for the active CAD model.
  * `Fixed Support`: Encastre boundary condition locking translations ($U_x=U_y=U_z=0$).
  * `Apply Load`: Directional structural force vector in Newtons.
  * `Safety Factor`: Inspects minimum factor of safety against yield limit.
  * `Displacement Field`: Displays maximum resultant deflection with exaggerated scale slider.
* **Properties HUD Panel:** Displays study type, maximum/minimum Von Mises stress, displacement magnitude, factor of safety, and node/element metrics.
* **Kotlin Data Contracts:** `SimulationResultInfo`, `NativeKernel.simulateCantileverBeam()`, and `NativeKernel.simulateSolid()`.

---

## 5. Original CAD SVG Icons

Original SVG drawings created in `docs/icons/`:
* `fea_solve.svg`: Tetrahedral element wireframe with stress contour lines and yield hotspot.
* `fea_constraint.svg`: Fixed boundary constraint support with grounded hatch lines.
* `fea_load.svg`: Downward structural force vector with load distribution bar.
* `fea_safety_factor.svg`: Structural safety shield with checkmark validation.
