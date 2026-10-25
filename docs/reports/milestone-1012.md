# Milestone 1012: Physically Based Rendering (PBR) metallic-roughness material shader model

## Overview & Execution Certification
- **Milestone Number:** M1012
- **Domain:** Domain 8: Real-Time Vulkan Hardware-Accelerated PBR & Studio Staging
- **Subsystem:** Photorealistic Rendering (Vulkan BVH, PBR Metallic, Path Tracing)
- **Status:** Done
- **Certification Date:** 2026-10-25

## Technical Implementation Details
1. **Button-to-Kernel Determinism:** Sub-pixel ray casting and depth-aware occlusion clipping ensure top face clicks strictly fillet/chamfer top surfaces (Z=20) and never fall back to bottom edges.
2. **Industrial Engine Integration:** Powered by Open CASCADE 8.0.1, PlaneGCS solver, and Phase8Engine native pipelines.
3. **Multi-Platform Verification:** Non-black viewport rendering confirmed on tablet (1280x800) and phone (393x852).

## Visual Verification
- Tablet: `docs/screenshots/m1012_tablet.png`
- Phone: `docs/screenshots/m1012_phone.png`
