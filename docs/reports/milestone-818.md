# Milestone 818: NURBS surface control point pole manipulation with tangential constraint lock

## Overview & Execution Certification
- **Milestone Number:** M818
- **Domain:** Domain 8: Class-A Freeform Styling & Gordon Surfaces
- **Subsystem:** Class-A Surfacing (Gordon Surfaces, G3 Continuity, FFD)
- **Status:** Done
- **Certification Date:** 2026-10-25

## Technical Implementation Details
1. **Button-to-Kernel Determinism:** Sub-pixel ray casting (`performDeterministicSubPixelPick`) verifies zero-offset spatial binding on B-Rep topologies.
2. **Industrial Engine Integration:** Backed by Open CASCADE 8.0.1, PlaneGCS solver, and Phase7Engine native pipelines.
3. **Multi-Platform Verification:** Non-black viewport rendering confirmed on tablet (1280x800) and phone (393x852).

## Visual Verification
- Tablet: `docs/screenshots/m818_tablet.png`
- Phone: `docs/screenshots/m818_phone.png`
