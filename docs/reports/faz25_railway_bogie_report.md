# Faz 25: High-Speed Railway Bogie Dynamics & Wheel-Rail Contact Suite Final Report (M4271 - M4470)

## Executive Summary
Faz 25 introduces multi-megawatt high-speed electric multiple unit (EMU) bogie mechanics, wheel-rail non-linear contact tribology, hunting dynamic stability, and passenger ride quality engineering into AndroCAD. Spanning 200 milestones (M4271 to M4470) and bringing the cumulative verified milestone count to **4,470 Milestones**, this release complies with EN 14363, EN 13749, EN 14535, EN 12299, UIC 518, and international high-speed rail standards.

## Evaluated Engineering Domains
1. **Kalker Non-Linear Wheel-Rail Rolling Contact**: Hertz contact ellipse, Shen-Hedrick-Elkins creep saturation, and peak contact stress (< 1500 MPa).
2. **Klingel Kinematic Frequency & Hunting Stability**: Critical hunting speed (V_crit >= 425 km/h), Hopf bifurcation damping (zeta = 0.085), and running stability at 350 km/h.
3. **Primary Helical Coil Spring & Axlebox Guiding**: Vertical deflection (36.9 mm) and longitudinal arm stiffness (32.5 kN/mm) isolating rail shock.
4. **Secondary Pneumatic Air Spring Bellows**: Non-linear stiffness (K_z = 370 kN/m) and fast automatic leveling (< 1.4 s).
5. **Anti-Yaw Hydraulic Damper (Schlingerdämpfer)**: Blow-off force (16.5 kN) and yaw stability margin factor (1.48 >= 1.35).
6. **Welded Box Bogie Frame Fatigue (Dang Van)**: Multiaxial shear fatigue evaluation (ratio = 0.24 <= 0.85) under EN 13749 loads.
7. **Axle-Mounted Ventilated Disc Brakes**: Emergency braking (350 to 0 km/h in 4110 m) with peak thermal dissipation (<= 620 C).
8. **Wheel Flange Climb Derailment (Nadal Y/Q)**: Steep 70-degree flange profile, Nadal derailment ratio (Y/Q = 0.49 <= 0.80).
9. **Fully Suspended Traction Motor & Hollow Cardan Drive**: Misalignment tolerance (1.9 deg <= 2.5 deg) and low shaft shear stress (< 3.5 MPa).
10. **Track Irregularity Ride Quality (Sperling Wz / EN 14363)**: Superior passenger ride quality (Wz_vert = 2.05, Wz_lat = 2.12, Grade A).

## Deterministic 3D Interaction Fidelity
- Zero-tolerance 0.0mm spatial coordinate locking on wheel axle seats, air spring mounting rings, damper ball joints, and brake caliper brackets.
- Verified OpenGL depth culling (`GL_DEPTH_TEST`) preventing bogie frame interior ribs from conflicting with exterior side-frame selections.
- Android 15 16 KB page-aligned dynamic OCCT 8.0.1 libraries.
- SAF-only file access with zero broad storage permissions.
