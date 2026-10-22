// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "AndroCadKernel.h"
#include "GCS.h"
#include "Sketcher.h"
#include "FeatureOps.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    std::cout << "[AndroCAD Test] Running Kernel Smoke Test (Box, Picking, PlaneGCS & Sketcher)..." << std::endl;

    androcad::AndroCadKernel kernel;
    double dx = 100.0;
    double dy = 50.0;
    double dz = 20.0;

    androcad::MeshPayload box = kernel.CreateBox(dx, dy, dz, 0.1, 0.5);

    if (!box.success) {
        std::cerr << "[FAIL] Kernel failed to create box: " << box.errorMessage << std::endl;
        return 1;
    }

    // 1. Verify analytical volume
    double expectedVolume = dx * dy * dz;
    double volumeError = std::abs(box.volume - expectedVolume);
    std::cout << "  Calculated Volume: " << box.volume << " mm³ (Expected: " << expectedVolume << " mm³)" << std::endl;
    if (volumeError > 1e-3) {
        std::cerr << "[FAIL] Volume mismatch! Error: " << volumeError << std::endl;
        return 1;
    }

    // 2. Verify topological face count (standard rectangular prism has 6 faces)
    std::cout << "  Boundary Face Count: " << box.faceCount << " (Expected: 6)" << std::endl;
    if (box.faceCount != 6) {
        std::cerr << "[FAIL] Face count mismatch! Expected 6, got " << box.faceCount << std::endl;
        return 1;
    }

    // 3. Verify mesh tessellation output
    std::cout << "  Triangles Generated: " << box.triangleCount << std::endl;
    std::cout << "  Vertices Generated: " << box.vertices.size() << std::endl;
    std::cout << "  Indices Generated: " << box.indices.size() << std::endl;

    if (box.triangleCount < 12) {
        std::cerr << "[FAIL] Box tessellation must have at least 12 triangles (2 per face)." << std::endl;
        return 1;
    }

    if (box.indices.size() != static_cast<size_t>(box.triangleCount * 3)) {
        std::cerr << "[FAIL] Index count does not equal triangleCount * 3." << std::endl;
        return 1;
    }

    // 4. Validate vertex normal integrity and face ID tagging
    for (size_t i = 0; i < box.vertices.size(); ++i) {
        const auto& v = box.vertices[i];
        if (v.faceId < 1 || v.faceId > 6) {
            std::cerr << "[FAIL] Vertex " << i << " has invalid FaceID: " << v.faceId << std::endl;
            return 1;
        }

        float normLen = std::sqrt(v.nx * v.nx + v.ny * v.ny + v.nz * v.nz);
        if (normLen < 0.9f || normLen > 1.1f) {
            std::cerr << "[FAIL] Vertex " << i << " has unnormalized normal vector: " << normLen << std::endl;
            return 1;
        }
    }

    // 5. Verify topological boundary edge extraction (Milestone 2)
    std::cout << "  Boundary Edge Count: " << box.edgeCount << " (Expected: 12)" << std::endl;
    if (box.edgeCount != 12) {
        std::cerr << "[FAIL] Edge count mismatch! Expected 12, got " << box.edgeCount << std::endl;
        return 1;
    }

    std::cout << "  Discretized Edge Vertices: " << box.edgeVertices.size() << " (Expected >= 24)" << std::endl;
    if (box.edgeVertices.size() < 24) {
        std::cerr << "[FAIL] Edge vertex count is too low! Expected >= 24, got " << box.edgeVertices.size() << std::endl;
        return 1;
    }

    // 6. Verify Bounding Box
    if (std::abs(box.bounds.maxX - 100.0f) > 0.1f ||
        std::abs(box.bounds.maxY - 50.0f) > 0.1f ||
        std::abs(box.bounds.maxZ - 20.0f) > 0.1f) {
        std::cerr << "[FAIL] Bounding box dimensions incorrect!" << std::endl;
        return 1;
    }

    // 7. Verify Ray-Casting Entity Picking (Milestone 2)
    androcad::Ray topRay{50.0f, 25.0f, 100.0f, 0.0f, 0.0f, -1.0f};
    androcad::PickResult pickFace = androcad::AndroCadKernel::PickEntityByRay(box, topRay, 1.0f);
    if (!pickFace.hit || pickFace.isEdge) {
        std::cerr << "[FAIL] Ray aimed at face interior failed!" << std::endl;
        return 1;
    }

    androcad::Ray edgeRay{100.0f, 25.0f, 100.0f, 0.0f, 0.0f, -1.0f};
    androcad::PickResult pickEdge = androcad::AndroCadKernel::PickEntityByRay(box, edgeRay, 1.5f);
    if (!pickEdge.hit || !pickEdge.isEdge) {
        std::cerr << "[FAIL] Ray aimed directly at edge failed to pick edge!" << std::endl;
        return 1;
    }

    // =========================================================================
    // Milestone 3: PlaneGCS Constraint Solver & Sketcher Tests
    // =========================================================================
    std::cout << "\n[AndroCAD Test] Running Milestone 3 Tests (PlaneGCS & Sketcher)..." << std::endl;

    // Test 3.1: GCS solver on 2D rectangle with initial perturbation
    androcad::gcs::System solver;
    int p0 = solver.AddPoint(0.0, 0.0, true);          // Fixed origin at (0, 0)
    int p1 = solver.AddPoint(105.0, 3.0, false);       // Perturbed
    int p2 = solver.AddPoint(98.0, 53.0, false);       // Perturbed
    int p3 = solver.AddPoint(-2.0, 48.0, false);       // Perturbed

    solver.AddConstraint(androcad::gcs::ConstraintType::Horizontal, p0, p1);
    solver.AddConstraint(androcad::gcs::ConstraintType::Vertical, p1, p2);
    solver.AddConstraint(androcad::gcs::ConstraintType::Horizontal, p3, p2);
    solver.AddConstraint(androcad::gcs::ConstraintType::Vertical, p0, p3);
    solver.AddConstraint(androcad::gcs::ConstraintType::Distance, p0, p1, -1, -1, 100.0);
    solver.AddConstraint(androcad::gcs::ConstraintType::Distance, p1, p2, -1, -1, 50.0);

    androcad::gcs::SolveStatus status = solver.Solve();
    std::cout << "  GCS Solver Status: " << static_cast<int>(status) 
              << ", DoF: " << solver.GetDegreesOfFreedom() 
              << ", Max Residual: " << solver.GetMaxResidual() << std::endl;

    if (status != androcad::gcs::SolveStatus::FullyConstrained) {
        std::cerr << "[FAIL] Expected FullyConstrained status (DoF == 0), got status " << static_cast<int>(status) << std::endl;
        return 1;
    }

    auto solvedP1 = solver.GetPoint(p1);
    auto solvedP2 = solver.GetPoint(p2);
    auto solvedP3 = solver.GetPoint(p3);

    std::cout << "  Solved Points: P1(" << solvedP1.x << ", " << solvedP1.y << ")"
              << ", P2(" << solvedP2.x << ", " << solvedP2.y << ")"
              << ", P3(" << solvedP3.x << ", " << solvedP3.y << ")" << std::endl;

    if (std::abs(solvedP1.x - 100.0) > 1e-3 || std::abs(solvedP1.y - 0.0) > 1e-3 ||
        std::abs(solvedP2.x - 100.0) > 1e-3 || std::abs(solvedP2.y - 50.0) > 1e-3 ||
        std::abs(solvedP3.x - 0.0) > 1e-3 || std::abs(solvedP3.y - 50.0) > 1e-3) {
        std::cerr << "[FAIL] GCS solver did not converge to exact rectangle coordinates!" << std::endl;
        return 1;
    }

    // Test 3.2: Over-constrained conflict detection
    solver.AddConstraint(androcad::gcs::ConstraintType::Distance, p0, p2, -1, -1, 1000.0); // Contradicts sqrt(100^2 + 50^2)
    androcad::gcs::SolveStatus overStatus = solver.Solve();
    std::cout << "  Conflicting Constraint Solve Status: " << static_cast<int>(overStatus) << " (Expected: OverConstrained)" << std::endl;
    if (overStatus != androcad::gcs::SolveStatus::OverConstrained) {
        std::cerr << "[FAIL] Expected OverConstrained status for conflicting constraint!" << std::endl;
        return 1;
    }

    // Test 3.3: Sketcher Profile Loop Detection & OCCT Face Construction (Rectangle on XY)
    androcad::Sketcher rectSketch(androcad::SketchPlane::XY());
    int rP0 = rectSketch.AddPoint(0.0, 0.0, true);
    int rP1 = rectSketch.AddPoint(100.0, 0.0, false);
    int rP2 = rectSketch.AddPoint(100.0, 50.0, false);
    int rP3 = rectSketch.AddPoint(0.0, 50.0, false);

    rectSketch.AddLine(rP0, rP1);
    rectSketch.AddLine(rP1, rP2);
    rectSketch.AddLine(rP2, rP3);
    rectSketch.AddLine(rP3, rP0);

    rectSketch.AddHorizontal(rP0, rP1);
    rectSketch.AddVertical(rP1, rP2);
    rectSketch.AddHorizontal(rP3, rP2);
    rectSketch.AddVertical(rP0, rP3);
    rectSketch.AddDistance(rP0, rP1, 100.0);
    rectSketch.AddDistance(rP1, rP2, 50.0);

    androcad::SketchResult rectFaceResult = rectSketch.BuildProfileFace();
    std::cout << "  Rect Face Build Result: success=" << rectFaceResult.success
              << ", isClosed=" << rectFaceResult.isClosedProfile
              << ", area=" << rectFaceResult.profileArea << " mm²"
              << ", DoF=" << rectFaceResult.degreesOfFreedom << std::endl;

    if (!rectFaceResult.success || !rectFaceResult.isClosedProfile) {
        std::cerr << "[FAIL] Rectangle sketch failed to build closed profile face: " << rectFaceResult.errorMessage << std::endl;
        return 1;
    }
    if (std::abs(rectFaceResult.profileArea - 5000.0) > 1e-2) {
        std::cerr << "[FAIL] Rectangle face area mismatch! Expected 5000.0, got " << rectFaceResult.profileArea << std::endl;
        return 1;
    }

    // Test 3.4: Sketcher Circle Profile Face Construction
    androcad::Sketcher circleSketch(androcad::SketchPlane::XY());
    int cCenter = circleSketch.AddPoint(0.0, 0.0, true);
    circleSketch.AddCircle(cCenter, 25.0);

    androcad::SketchResult circleFaceResult = circleSketch.BuildProfileFace();
    double expectedCircleArea = M_PI * 25.0 * 25.0;
    std::cout << "  Circle Face Build Result: success=" << circleFaceResult.success
              << ", area=" << circleFaceResult.profileArea << " mm² (Expected: " << expectedCircleArea << " mm²)" << std::endl;

    if (!circleFaceResult.success || !circleFaceResult.isClosedProfile) {
        std::cerr << "[FAIL] Circle sketch failed to build closed profile face: " << circleFaceResult.errorMessage << std::endl;
        return 1;
    }
    if (std::abs(circleFaceResult.profileArea - expectedCircleArea) > 0.5) {
        std::cerr << "[FAIL] Circle face area mismatch! Expected " << expectedCircleArea << ", got " << circleFaceResult.profileArea << std::endl;
        return 1;
    }

    // =========================================================================
    // Milestone 4: 3D Parametric Features Tests
    // =========================================================================
    std::cout << "\n[AndroCAD Test] Running Milestone 4 Tests (Extrude, Revolve, Fillet, Chamfer, Hole, Shell, Combine)..." << std::endl;

    // Test 4.1: Extrude 2D profile face into 3D solid prism
    TopoDS_Shape extrudeSolid;
    auto extrudeMesh = androcad::FeatureOps::MakeExtrude(rectFaceResult.face, 0.0, 0.0, 30.0, extrudeSolid);
    std::cout << "  Extrude Result: success=" << extrudeMesh.success
              << ", volume=" << extrudeMesh.volume << " mm³ (Expected: 150000.0)"
              << ", faces=" << extrudeMesh.faceCount << ", edges=" << extrudeMesh.edgeCount
              << ", triangles=" << extrudeMesh.triangleCount << std::endl;

    if (!extrudeMesh.success) {
        std::cerr << "[FAIL] MakeExtrude failed: " << extrudeMesh.errorMessage << std::endl;
        return 1;
    }
    if (std::abs(extrudeMesh.volume - 150000.0) > 1e-2) {
        std::cerr << "[FAIL] Extrude volume mismatch! Expected 150000.0, got " << extrudeMesh.volume << std::endl;
        return 1;
    }
    if (extrudeMesh.faceCount != 6 || extrudeMesh.edgeCount != 12) {
        std::cerr << "[FAIL] Extrude topology mismatch! Expected 6 faces and 12 edges, got " 
                  << extrudeMesh.faceCount << " faces, " << extrudeMesh.edgeCount << " edges" << std::endl;
        return 1;
    }

    // Test 4.2: Fillet target edge with constant radius
    TopoDS_Shape filletSolid;
    auto filletMesh = androcad::FeatureOps::MakeFillet(extrudeSolid, 5.0, 1, filletSolid);
    std::cout << "  Fillet Result: success=" << filletMesh.success
              << ", volume=" << filletMesh.volume << " mm³ (Pre-fillet: " << extrudeMesh.volume << ")"
              << ", faces=" << filletMesh.faceCount << " (Expected: 7)" << std::endl;

    if (!filletMesh.success) {
        std::cerr << "[FAIL] MakeFillet failed: " << filletMesh.errorMessage << std::endl;
        return 1;
    }
    if (filletMesh.faceCount != 7) {
        std::cerr << "[FAIL] Fillet face count mismatch! Expected 7, got " << filletMesh.faceCount << std::endl;
        return 1;
    }
    if (filletMesh.volume >= extrudeMesh.volume || filletMesh.volume < 149000.0) {
        std::cerr << "[FAIL] Fillet volume out of expected range: " << filletMesh.volume << std::endl;
        return 1;
    }

    // Test 4.3: Chamfer target edge with distance bevel
    TopoDS_Shape chamferSolid;
    auto chamferMesh = androcad::FeatureOps::MakeChamfer(extrudeSolid, 3.0, 2, chamferSolid);
    std::cout << "  Chamfer Result: success=" << chamferMesh.success
              << ", volume=" << chamferMesh.volume << " mm³"
              << ", faces=" << chamferMesh.faceCount << " (Expected: 7)" << std::endl;

    if (!chamferMesh.success) {
        std::cerr << "[FAIL] MakeChamfer failed: " << chamferMesh.errorMessage << std::endl;
        return 1;
    }
    if (chamferMesh.faceCount != 7) {
        std::cerr << "[FAIL] Chamfer face count mismatch! Expected 7, got " << chamferMesh.faceCount << std::endl;
        return 1;
    }
    if (chamferMesh.volume >= extrudeMesh.volume) {
        std::cerr << "[FAIL] Chamfer volume should be less than original box: " << chamferMesh.volume << std::endl;
        return 1;
    }

    // Test 4.4: Hole (cylindrical cut through solid)
    TopoDS_Shape holeSolid;
    auto holeMesh = androcad::FeatureOps::MakeHole(extrudeSolid, 50.0, 25.0, 0.0, 0.0, 0.0, 1.0, 10.0, 30.0, holeSolid);
    double expectedHoleCutVolume = M_PI * 100.0 * 30.0;
    double expectedRemainingVolume = 150000.0 - expectedHoleCutVolume;
    std::cout << "  Hole Result: success=" << holeMesh.success
              << ", volume=" << holeMesh.volume << " mm³ (Expected: " << expectedRemainingVolume << ")"
              << ", faces=" << holeMesh.faceCount << std::endl;

    if (!holeMesh.success) {
        std::cerr << "[FAIL] MakeHole failed: " << holeMesh.errorMessage << std::endl;
        return 1;
    }
    if (std::abs(holeMesh.volume - expectedRemainingVolume) > 1.0) {
        std::cerr << "[FAIL] Hole cut volume mismatch! Expected " << expectedRemainingVolume << ", got " << holeMesh.volume << std::endl;
        return 1;
    }

    // Test 4.5: Revolve 2D profile face around an axis
    androcad::Sketcher revSketch(androcad::SketchPlane::XY());
    int rp1 = revSketch.AddPoint(20.0, 0.0, true);
    int rp2 = revSketch.AddPoint(30.0, 0.0, true);
    int rp3 = revSketch.AddPoint(30.0, 10.0, true);
    int rp4 = revSketch.AddPoint(20.0, 10.0, true);
    revSketch.AddLine(rp1, rp2);
    revSketch.AddLine(rp2, rp3);
    revSketch.AddLine(rp3, rp4);
    revSketch.AddLine(rp4, rp1);
    auto revProfileFace = revSketch.BuildProfileFace();

    TopoDS_Shape revSolid;
    auto revMesh = androcad::FeatureOps::MakeRevolve(revProfileFace.face, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0 * M_PI, revSolid);
    double expectedRevVolume = 2.0 * M_PI * 25.0 * 100.0; // Pappus's centroid theorem
    std::cout << "  Revolve Result: success=" << revMesh.success
              << ", volume=" << revMesh.volume << " mm³ (Expected: " << expectedRevVolume << ")"
              << ", triangles=" << revMesh.triangleCount << std::endl;

    if (!revMesh.success) {
        std::cerr << "[FAIL] MakeRevolve failed: " << revMesh.errorMessage << std::endl;
        return 1;
    }
    if (std::abs(revMesh.volume - expectedRevVolume) > 15.0) {
        std::cerr << "[FAIL] Revolve volume mismatch! Expected " << expectedRevVolume << ", got " << revMesh.volume << std::endl;
        return 1;
    }

    // Test 4.6: Boolean Combine (Join, Cut, Intersect)
    BRepPrimAPI_MakeBox mkBoxA(40.0, 40.0, 40.0);
    TopoDS_Shape shapeA = mkBoxA.Shape();
    BRepPrimAPI_MakeBox mkBoxB(gp_Pnt(10.0, 10.0, 10.0), 20.0, 20.0, 20.0);
    TopoDS_Shape shapeB = mkBoxB.Shape();

    // Boolean Cut: A \ B = 64000 - 8000 = 56000 mm³
    TopoDS_Shape boolCutSolid;
    auto boolCutMesh = androcad::FeatureOps::MakeBoolean(shapeA, shapeB, androcad::BooleanType::Cut, 1e-5, boolCutSolid);
    std::cout << "  Boolean Cut Volume: " << boolCutMesh.volume << " mm³ (Expected: 56000.0)" << std::endl;
    if (!boolCutMesh.success || std::abs(boolCutMesh.volume - 56000.0) > 1e-2) {
        std::cerr << "[FAIL] Boolean Cut failed or volume mismatch!" << std::endl;
        return 1;
    }

    // Boolean Intersect: A n B = 8000 mm³
    TopoDS_Shape boolIntSolid;
    auto boolIntMesh = androcad::FeatureOps::MakeBoolean(shapeA, shapeB, androcad::BooleanType::Intersect, 1e-5, boolIntSolid);
    std::cout << "  Boolean Intersect Volume: " << boolIntMesh.volume << " mm³ (Expected: 8000.0)" << std::endl;
    if (!boolIntMesh.success || std::abs(boolIntMesh.volume - 8000.0) > 1e-2) {
        std::cerr << "[FAIL] Boolean Intersect failed or volume mismatch!" << std::endl;
        return 1;
    }

    // Test 4.7: Shell (Hollow out solid with opening face)
    BRepPrimAPI_MakeBox mkShellBox(50.0, 50.0, 50.0);
    TopoDS_Shape shellBaseShape = mkShellBox.Shape();
    TopoDS_Shape shellOutSolid;
    auto shellMesh = androcad::FeatureOps::MakeShell(shellBaseShape, 2.0, 1, shellOutSolid);
    std::cout << "  Shell Result: success=" << shellMesh.success
              << ", volume=" << shellMesh.volume << " mm³ (Original: 125000.0)"
              << ", faces=" << shellMesh.faceCount << std::endl;

    if (!shellMesh.success) {
        std::cerr << "[FAIL] MakeShell failed: " << shellMesh.errorMessage << std::endl;
        return 1;
    }
    if (shellMesh.volume >= 125000.0 || shellMesh.volume <= 0.0) {
        std::cerr << "[FAIL] Shell volume out of expected bounds: " << shellMesh.volume << std::endl;
        return 1;
    }

    // Test 4.8: Negative / Error Handling Validation
    TopoDS_Shape dummyOut;
    auto negExtrude = androcad::FeatureOps::MakeExtrude(TopoDS_Face(), 0, 0, 10, dummyOut);
    if (negExtrude.success) {
        std::cerr << "[FAIL] Expected failure for null face extrude!" << std::endl;
        return 1;
    }
    auto negZeroVecExtrude = androcad::FeatureOps::MakeExtrude(rectFaceResult.face, 0, 0, 0, dummyOut);
    if (negZeroVecExtrude.success) {
        std::cerr << "[FAIL] Expected failure for zero-vector extrude!" << std::endl;
        return 1;
    }
    auto negFillet = androcad::FeatureOps::MakeFillet(extrudeSolid, -2.0, 1, dummyOut);
    if (negFillet.success) {
        std::cerr << "[FAIL] Expected failure for negative radius fillet!" << std::endl;
        return 1;
    }

    auto negChamferNull = androcad::FeatureOps::MakeChamfer(TopoDS_Shape(), 2.0, 1, dummyOut);
    if (negChamferNull.success) {
        std::cerr << "[FAIL] Expected failure for null shape chamfer!" << std::endl;
        return 1;
    }

    auto negChamferDistance = androcad::FeatureOps::MakeChamfer(extrudeSolid, -1.0, 1, dummyOut);
    if (negChamferDistance.success) {
        std::cerr << "[FAIL] Expected failure for negative distance chamfer!" << std::endl;
        return 1;
    }

    // Out-of-bounds edge IDs should NOT crash and should either fallback safely or return graceful failure
    TopoDS_Shape outOfBoundsSolid;
    auto oobChamfer = androcad::FeatureOps::MakeChamfer(extrudeSolid, 2.0, 9999, outOfBoundsSolid);
    std::cout << "  Out-of-bounds edge chamfer handled safely: success=" << oobChamfer.success << std::endl;

    auto oobFillet = androcad::FeatureOps::MakeFillet(extrudeSolid, 2.0, 9999, outOfBoundsSolid);
    std::cout << "  Out-of-bounds edge fillet handled safely: success=" << oobFillet.success << std::endl;

    std::cout << "[PASS] All AndroCAD Milestone 3 & 4 Tests passed successfully!" << std::endl;
    return 0;
}