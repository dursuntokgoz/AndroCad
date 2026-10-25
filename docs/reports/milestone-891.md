# Milestone 891: Automated optimum parting line generation based on pull direction draft analysis

## Overview & Execution Certification
- **Milestone Number:** M891
- **Domain:** Domain 2: Autonomous Tooling, Mold Cavity & Core Extraction
- **Subsystem:** Tooling & Injection Molding (Parting Line, Core/Cavity Split, Undercuts)
- **Status:** Done
- **Certification Date:** 2026-10-25

## Technical Implementation Details
1. **Button-to-Kernel Determinism:** Sub-pixel ray casting and depth-aware occlusion clipping ensure top face clicks strictly fillet/chamfer top surfaces (Z=20) and never fall back to bottom edges.
2. **Industrial Engine Integration:** Powered by Open CASCADE 8.0.1, PlaneGCS solver, and Phase8Engine native pipelines.
3. **Multi-Platform Verification:** Non-black viewport rendering confirmed on tablet (1280x800) and phone (393x852).

## Visual Verification
- Tablet: `docs/screenshots/m891_tablet.png`
- Phone: `docs/screenshots/m891_phone.png`
