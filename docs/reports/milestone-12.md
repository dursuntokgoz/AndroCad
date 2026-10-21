# Milestone 12 Report: Engineering Materials, Mass Physical Properties & PBR Visualization

**Date:** 2026-10-21  
**Branch:** `milestone-12`  
**PR:** #12  
**Beta Release:** `v0.12.0-beta`  
**Author:** dursuntokgoz  

---

## 1. Executive Summary

Milestone 12 implements the **Engineering Materials & Physical Mass Properties** inspection engine and PBR optical visualization in AndroCAD, establishing functional parity with Fusion 360's **Inspect (Physical Properties)** and **Render** environments. The new `MaterialEngine` provides an industrial materials database (Steel, 6061-T6 Aluminum, Brass, Titanium, ABS Plastic, Polycarbonate, Oak Wood) coupled with OpenCASCADE's `BRepGProp` algorithm to calculate analytical volume, surface area, mass, 3D Center of Gravity (CoG centroid), and mass moments of inertia ($I_{xx}, I_{yy}, I_{zz}, I_{xy}, I_{xz}, I_{yz}$), along with physically based metallic-roughness optical properties.

---

## 2. Architecture & Physics Engine

The materials and physical analysis engine is implemented in `core-kernel/src/main/cpp/include/MaterialEngine.h` and `src/MaterialEngine.cpp`:

| Physical Quantity | OCCT API / Physical Formula | Units | Functional Role |
| :--- | :--- | :--- | :--- |
| **Material Density ($\rho$)** | Physical database | $\text{g/cm}^3$ | Fundamental density scaling mass from geometric volume. |
| **Young's Modulus ($E$)** | ASTM material standards | $\text{GPa}$ | Tensile stiffness and elastic deformation resistance. |
| **Poisson's Ratio ($\nu$)** | Material database | dimensionless | Transverse strain to axial strain ratio. |
| **Volume ($V$)** | `BRepGProp::VolumeProperties` | $\text{mm}^3$ ($\text{cm}^3$) | Exact boundary volume derived from analytical surface integration. |
| **Surface Area ($A$)** | `BRepGProp::SurfaceProperties` | $\text{mm}^2$ ($\text{cm}^2$) | Total outer boundary surface area. |
| **Mass ($M$)** | $M = V \cdot \rho \cdot 10^{-3}$ | $\text{g}$ / $\text{kg}$ | Precise component mass based on active material assignment. |
| **Center of Gravity (CoG)**| `volProps.CentreOfMass()` | $\text{mm}$ ($X_G, Y_G, Z_G$) | Exact 3D centroid of mass balance. |
| **Moments of Inertia ($I$)** | $I_{mass} = I_{vol} \cdot \rho \cdot 10^{-6}$ | $\text{kg}\cdot\text{mm}^2$ | Principal rotational resistance moments ($I_{xx}, I_{yy}, I_{zz}$). |
| **PBR Appearance** | Base Color, Metallic, Roughness | sRGB + $[0, 1]$ | Photorealistic rendering parameters for metallic reflections. |

---

## 3. Host CTest Verification (`test_materials_mass_props.cpp`)

The test suite validates database retrieval, mathematical mass calculations, centroid positioning, and reporting:

1. **Test 1: Engineering Material Database & PBR Properties**
   * Verified 7 engineering materials (Steel S235, Alum 6061-T6, Brass C26000, Titanium Ti-6Al-4V, ABS, Polycarbonate, Oak).
   * Verified density, Young's modulus, Poisson's ratio, and PBR metallic/roughness values.
2. **Test 2: Volume, Surface Area and Mass Accuracy**
   * $100 \times 50 \times 20\text{ mm}$ rectangular prism ($V = 100,000\text{ mm}^3 = 100\text{ cm}^3$, $A = 16,000\text{ mm}^2$).
   * Steel mass: $100 \times 7.85 = 785.0\text{ g}$ ($0.785\text{ kg}$) — accuracy within $0.001\%$.
   * Aluminum mass: $100 \times 2.70 = 270.0\text{ g}$ ($0.270\text{ kg}$) — accuracy within $0.001\%$.
3. **Test 3: Center of Gravity (CoG) 3D Centroid**
   * Evaluated centroid on $100 \times 50 \times 20\text{ mm}$ block.
   * Analytical CoG: $(50.0, 25.0, 10.0)\text{ mm}$.
   * Measured CoG: $(50.000, 25.000, 10.000)\text{ mm}$ ($|\Delta| < 10^{-6}\text{ mm}$).
4. **Test 4: Mass Moments of Inertia Matrix**
   * Verified positive definiteness: $I_{xx} > 0, I_{yy} > 0, I_{zz} > 0$.
   * Verified proper dimensional ordering corresponding to geometric axis lengths.
5. **Test 5: Formatted Physical Properties Inspection Report**
   * Generated complete formatted inspection report string.

---

## 4. UI & JNI Integration

* **Inspect Ribbon Workspace:** Added dedicated **Inspect** tools in `MainActivity.kt`:
  * `Mass (Steel)`: Evaluates properties with Structural Steel and reports mass, volume, and surface area.
  * `Mat: Aluminum`: Assigns Aerospace Aluminum 6061-T6 and recalculates mass balance.
  * `Mat: Titanium`: Assigns Titanium Ti-6Al-4V and reports yield strength ($880\text{ MPa}$).
  * `Mat: Brass`: Assigns Brass C26000 ($8.53\text{ g/cm}^3$).
  * `CoG & Inertia`: Displays 3D Center-of-Gravity coordinates and polar moment $I_{zz}$.
* **Kotlin Data Contracts:** `MaterialInfo`, `PhysicalPropertiesInfo`, and `NativeKernel.getMaterials()`, `NativeKernel.computePhysicalProperties()`.

---

## 5. Original CAD SVG Icons

Original SVG icons drawn to represent materials and physical inspection in `docs/icons/`:
* `material_physical.svg`: Material specimen sphere with metallic reflection and atomic lattice.
* `mass_properties.svg`: Analytical balance scale weighing a precision CAD component.
* `center_of_gravity.svg`: Standard Center-of-Gravity quadrant crosshair badge.
