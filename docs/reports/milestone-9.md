# Milestone 9 Report: Multi-Component Assemblies & Kinematic Joints

**Date:** 2026-10-21  
**Branch:** `milestone-9`  
**PR:** #9  
**Beta Release:** `v0.9.0-beta`  
**Author:** dursuntokgoz  

---

## 1. Executive Summary

Milestone 9 implements a full **Assembly & Kinematic Joints** architecture in AndroCAD, establishing functional parity with Fusion 360's **Assemble** workspace. It provides multi-body component management with isolated coordinate systems, kinematic joints (Rigid, Revolute, Slider, Cylindrical, Ball), a hierarchical Forward Kinematics (FK) solver, collision/interference detection, and interactive touch animation controls on Android tablets.

---

## 2. Architecture & Kinematics Engine

The assembly pipeline is implemented in `core-kernel/src/main/cpp/include/AssemblyManager.h` and `src/AssemblyManager.cpp`:

| Assembly Concept | OCCT Implementation | Functional Role |
| :--- | :--- | :--- |
| **Component** | `TopoDS_Shape` + `gp_Trsf` | Individual parts with independent local transformations and derived world placements. |
| **Rigid Joint** | Fixed relative `gp_Trsf` | 0 DoF: child is rigidly fixed to parent part at designated mounting anchor. |
| **Revolute Joint** | `gp_Ax1` rotation + angular clamping | 1 DoF: rotational movement about a 3D axis with bounds $[\theta_{\min}, \theta_{\max}]$. |
| **Slider Joint** | `gp_Vec` linear translation + linear clamping | 1 DoF: prismatic displacement along translation vector within bounds $[s_{\min}, s_{\max}]$. |
| **Cylindrical Joint** | Simultaneous rotation & translation | 2 DoF: screw/piston motion along a shared cylindrical axis. |
| **FK Solver** | Iterative tree traversal | Recursively computes $T_{\text{world}} = T_{\text{parent}} \cdot T_{\text{joint}} \cdot T_{\text{local}}$. |
| **Interference Check**| `BRepAlgoAPI_Common` + `BRepGProp` | Detects spatial overlapping collisions between any two components in assembly space. |

---

## 3. Host CTest Verification (`test_assembly_joints.cpp`)

The test suite validates kinematics equations, degree-of-freedom constraints, and volumetric interference checks:

1. **Test 1: Rigid Multi-Component Assembly**
   * Base ($100 \times 100 \times 20\text{ mm}$) and Post ($20 \times 20 \times 80\text{ mm}$) rigidly linked.
   * Analytical expected combined volume: $200,000 + 32,000 = 232,000\text{ mm}^3$.
   * Measured volume: $232,000.0\text{ mm}^3$ ($|\Delta V| < 0.005\%$).
2. **Test 2: Revolute Joint Limits & Angular Clamping**
   * Revolute hinge with range $[-\frac{\pi}{2}, +\frac{\pi}{2}]$.
   * Out-of-bounds commands ($+\pi$, $-\pi$) successfully clamped to exact geometric limits.
3. **Test 3: Slider Joint Prismatic Motion**
   * Linear carriage displaced to $X = 75.0\text{ mm}$ along rail.
   * World transform translation vector confirmed at $X = 75.000\text{ mm}$.
4. **Test 4: 2-Link Robot Arm Forward Kinematics**
   * Base $\to$ Link 1 ($100\text{ mm}$, rotated $90^\circ$ around $Z$) $\to$ Link 2 ($80\text{ mm}$).
   * End-effector coordinates resolved along $+Y$ matching analytical planar kinematics.
5. **Test 5: Component Collision & Interference Detection**
   * Two $50\text{ mm}$ cubes on a slider axis.
   * At $X = 20\text{ mm}$ (overlapping): `CheckInterference() == true`.
   * At $X = 80\text{ mm}$ (separated): `CheckInterference() == false`.

---

## 4. UI & JNI Integration

* **Assemble Ribbon Workspace:** Dedicated **Assemble** workspace tab in `MainActivity.kt`:
  * `Robot Arm`: Spawns a 3-component, 2-revolute joint robotic manipulator in the viewport.
  * `Rotate (+15°)`: Steps revolute joints by $+15^\circ$, recomputes forward kinematics, and updates 3D view.
  * `Reset Joint`: Returns all assembly joints to $0^\circ$ home position.
  * `Check Collision`: Evaluates real-time solid interference and displays status notification.
* **JNI Bridge:** Added `nativeCreateAssembly`, `nativeAssemblyAddComponent`, `nativeAssemblyAddJoint`, `nativeAssemblySetJointValue`, and `nativeAssemblyBuildMesh` into `native_kernel_jni.cpp`.
* **Kotlin API:** Introduced `AssemblySession` and `AssemblyJointType` in `NativeKernel.kt`.
* **Original Vector Icons:** Added `joint_revolute.svg`, `joint_slider.svg`, and `assembly.svg` under `docs/icons/`.

---

## 5. Beta APK Release

Merging PR #9 and tagging `v0.9.0-beta` triggers automated GitHub Actions release packaging with Milestone 9 capabilities.
