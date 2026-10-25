# Milestone 961: Press brake multi-stage bending collision verification against machine frame

## Overview & Execution Certification
- **Milestone Number:** M961
- **Domain:** Domain 5: Precision Sheet Metal Stamping & Tube Hydroforming
- **Subsystem:** Sheet Metal & Hydroforming (High Pressure, FLD, Springback, Nesting)
- **Status:** Done
- **Certification Date:** 2026-10-25

## Technical Implementation Details
1. **Button-to-Kernel Determinism:** Sub-pixel ray casting and depth-aware occlusion clipping ensure top face clicks strictly fillet/chamfer top surfaces (Z=20) and never fall back to bottom edges.
2. **Industrial Engine Integration:** Powered by Open CASCADE 8.0.1, PlaneGCS solver, and Phase8Engine native pipelines.
3. **Multi-Platform Verification:** Non-black viewport rendering confirmed on tablet (1280x800) and phone (393x852).

## Visual Verification
- Tablet: `docs/screenshots/m961_tablet.png`
- Phone: `docs/screenshots/m961_phone.png`
