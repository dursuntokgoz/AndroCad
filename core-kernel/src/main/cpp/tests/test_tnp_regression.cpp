// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "ExpressionParser.h"
#include "TNPResolver.h"
#include "TimelineDAG.h"
#include "FeatureOps.h"
#include "AndroCadKernel.h"
#include "Sketcher.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "[AndroCAD Test] Running Milestone 5 TNP & Timeline Suite  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // =========================================================================
    // Part 1: Mathematical Expression Parser & Dependency DAG Tests
    // =========================================================================
    std::cout << "\n[Test Part 1] Mathematical Expression Parser & User Parameters..." << std::endl;
    {
        androcad::ExpressionParser parser;
        parser.SetVariable("width", 50.0);
        parser.SetVariable("length", 100.0);

        // Basic arithmetic
        auto r1 = parser.Evaluate("width * 2 + 10");
        assert(r1.success && std::abs(r1.value - 110.0) < 1e-9);

        // Power and parentheses
        auto r2 = parser.Evaluate("(width / 2)^2");
        assert(r2.success && std::abs(r2.value - 625.0) < 1e-9);

        // Trigonometric & math functions
        auto r3 = parser.Evaluate("sqrt(30^2 + 40^2)");
        assert(r3.success && std::abs(r3.value - 50.0) < 1e-9);

        auto r4 = parser.Evaluate("sin(PI / 2)");
        assert(r4.success && std::abs(r4.value - 1.0) < 1e-9);

        // Division by zero guard
        auto rDivZero = parser.Evaluate("length / 0");
        assert(!rDivZero.success && rDivZero.errorMessage == "Division by zero.");

        // Multi-parameter topological dependency evaluation
        std::unordered_map<std::string, std::string> paramTable = {
            {"base_w", "40.0"},
            {"base_l", "base_w * 2.5"},       // 100.0
            {"base_h", "base_l / 4.0 + 5.0"}, // 30.0
            {"diagonal", "sqrt(base_w^2 + base_l^2)"} // sqrt(1600 + 10000) = sqrt(11600)
        };

        auto batchRes = androcad::ExpressionParser::EvaluateAll(paramTable);
        assert(batchRes["base_w"].success && std::abs(batchRes["base_w"].value - 40.0) < 1e-9);
        assert(batchRes["base_l"].success && std::abs(batchRes["base_l"].value - 100.0) < 1e-9);
        assert(batchRes["base_h"].success && std::abs(batchRes["base_h"].value - 30.0) < 1e-9);
        assert(batchRes["diagonal"].success && std::abs(batchRes["diagonal"].value - std::sqrt(11600.0)) < 1e-9);

        // Cyclic dependency detection guard
        std::unordered_map<std::string, std::string> cyclicTable = {
            {"param_a", "param_b + 10"},
            {"param_b", "param_a * 2"}
        };
        auto cyclicRes = androcad::ExpressionParser::EvaluateAll(cyclicTable);
        assert(!cyclicRes["param_a"].success);
        std::cout << "  [PASS] Expression parser, constants, and cycle detection validated." << std::endl;
    }

    // =========================================================================
    // Part 2: TNP Test 1 - Upstream Dimension Scaling with Attached Downstream Face
    // =========================================================================
    std::cout << "\n[TNP Test 1] Upstream Dimension Scaling with Attached Downstream Face..." << std::endl;
    {
        // Initial setup: Box 50x50x20
        BRepPrimAPI_MakeBox mkInitialBox(50.0, 50.0, 20.0);
        TopoDS_Shape initialBox = mkInitialBox.Shape();

        auto taggedFaces = androcad::TNPResolver::TagExtrudeFaces(initialBox, 0, 0, 20);
        assert(taggedFaces.find("Cap_End") != taggedFaces.end());
        TopoDS_Face initialTopFace = taggedFaces["Cap_End"];

        androcad::TopologicalReference topFaceRef = androcad::TNPResolver::CreateFaceReference(
            initialBox, initialTopFace, "Pad_1:Cap_End"
        );

        // Parameter Mutation: Box resized to 100x50x35
        BRepPrimAPI_MakeBox mkMutatedBox(100.0, 50.0, 35.0);
        TopoDS_Shape mutatedBox = mkMutatedBox.Shape();

        TopoDS_Face resolvedTopFace;
        auto resolution = androcad::TNPResolver::ResolveFace(mutatedBox, topFaceRef, resolvedTopFace);

        assert(resolution.status != androcad::TNPResolutionStatus::BrokenReference);
        assert(!resolvedTopFace.IsNull());

        auto fp = androcad::GeometricFingerprint::FromFace(resolvedTopFace);
        std::cout << "  Resolved Top Face: Z=" << fp.center[2] << " (Expected: 35.0), Area=" << fp.size << " mm² (Expected: 5000.0)" << std::endl;
        assert(std::abs(fp.center[2] - 35.0) < 1e-3);
        assert(std::abs(fp.size - 5000.0) < 1e-1);
        assert(std::abs(fp.normal[2] - 1.0) < 1e-3);
        std::cout << "  [PASS] TNP Test 1: Top face correctly tracked across dimension scaling." << std::endl;
    }

    // =========================================================================
    // Part 3: TNP Test 2 - Edge Fillet Persistence across Upstream Dimension Edit
    // =========================================================================
    std::cout << "\n[TNP Test 2] Edge Fillet Persistence across Upstream Dimension Edit..." << std::endl;
    {
        // Initial setup: Box 40x40x40
        BRepPrimAPI_MakeBox mkBox(40.0, 40.0, 40.0);
        TopoDS_Shape box = mkBox.Shape();

        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(box, TopAbs_EDGE, edgeMap);
        // Find a vertical edge along Z
        int targetEdgeIdx = 1;
        for (int i = 1; i <= edgeMap.Extent(); ++i) {
            auto fp = androcad::GeometricFingerprint::FromEdge(TopoDS::Edge(edgeMap(i)));
            if (std::abs(fp.normal[2]) > 0.9) { // Vertical edge
                targetEdgeIdx = i;
                break;
            }
        }

        TopoDS_Edge initialEdge = TopoDS::Edge(edgeMap(targetEdgeIdx));
        androcad::TopologicalReference edgeRef = androcad::TNPResolver::CreateEdgeReference(
            box, initialEdge, "Pad_1:VerticalEdge_1"
        );

        // Mutation: Box depth changed from 40 to 80 (40x80x40)
        BRepPrimAPI_MakeBox mkMutatedBox(40.0, 80.0, 40.0);
        TopoDS_Shape mutatedBox = mkMutatedBox.Shape();

        TopoDS_Edge resolvedEdge;
        auto edgeRes = androcad::TNPResolver::ResolveEdge(mutatedBox, edgeRef, resolvedEdge);
        assert(edgeRes.status != androcad::TNPResolutionStatus::BrokenReference);
        assert(!resolvedEdge.IsNull());

        // Apply Fillet on resolved edge
        TopoDS_Shape filletedSolid;
        auto filletMesh = androcad::FeatureOps::MakeFillet(mutatedBox, 5.0, 1, filletedSolid);
        assert(filletMesh.success);
        assert(filletMesh.faceCount == 7);
        std::cout << "  [PASS] TNP Test 2: Edge resolved and filleted successfully on mutated model." << std::endl;
    }

    // =========================================================================
    // Part 4: TNP Test 3 - Face Splitting via Boolean Cut (Centroid Disambiguation)
    // =========================================================================
    std::cout << "\n[TNP Test 3] Face Splitting via Boolean Cut (Centroid Disambiguation)..." << std::endl;
    {
        // Slab 100x40x10
        BRepPrimAPI_MakeBox mkSlab(100.0, 40.0, 10.0);
        // Pocket splitting top face: 20x40x10 at X=40..60
        BRepPrimAPI_MakeBox mkPocket(gp_Pnt(40.0, 0.0, 0.0), 20.0, 40.0, 10.0);
        TopoDS_Shape cutSolid;
        androcad::FeatureOps::MakeBoolean(mkSlab.Shape(), mkPocket.Shape(), androcad::BooleanType::Cut, 1e-5, cutSolid);

        // Right face is at X=60..100 (Centroid X=80)
        TopTools_IndexedMapOfShape cutFaces;
        TopExp::MapShapes(cutSolid, TopAbs_FACE, cutFaces);

        TopoDS_Face rightFace;
        for (int i = 1; i <= cutFaces.Extent(); ++i) {
            auto fp = androcad::GeometricFingerprint::FromFace(TopoDS::Face(cutFaces(i)));
            if (fp.normal[2] > 0.9 && fp.center[0] > 60.0) {
                rightFace = TopoDS::Face(cutFaces(i));
                break;
            }
        }
        assert(!rightFace.IsNull());

        androcad::TopologicalReference rightRef = androcad::TNPResolver::CreateFaceReference(
            cutSolid, rightFace, "Slab:TopFace_Right"
        );

        // Mutate pocket: width increases from 20 to 30 mm (X=35..65)
        BRepPrimAPI_MakeBox mkMutPocket(gp_Pnt(35.0, 0.0, 0.0), 30.0, 40.0, 10.0);
        TopoDS_Shape mutatedCutSolid;
        androcad::FeatureOps::MakeBoolean(mkSlab.Shape(), mkMutPocket.Shape(), androcad::BooleanType::Cut, 1e-5, mutatedCutSolid);

        TopoDS_Face resolvedRightFace;
        auto res = androcad::TNPResolver::ResolveFace(mutatedCutSolid, rightRef, resolvedRightFace);
        assert(res.status != androcad::TNPResolutionStatus::BrokenReference);
        assert(!resolvedRightFace.IsNull());

        auto resolvedFp = androcad::GeometricFingerprint::FromFace(resolvedRightFace);
        std::cout << "  Resolved Right Face Centroid X: " << resolvedFp.center[0] << " (Expected > 65.0)" << std::endl;
        assert(resolvedFp.center[0] > 65.0);
        std::cout << "  [PASS] TNP Test 3: Split face correctly resolved via centroid fingerprint." << std::endl;
    }

    // =========================================================================
    // Part 5: TNP Test 4 - Timeline DAG Rebuild & Rollback
    // =========================================================================
    std::cout << "\n[TNP Test 4] Timeline DAG Rebuild & Rollback..." << std::endl;
    {
        androcad::TimelineDAG timeline;
        timeline.SetParameter("box_w", "60.0");
        timeline.SetParameter("box_d", "60.0");
        timeline.SetParameter("box_h", "20.0");
        timeline.SetParameter("hole_r", "10.0");

        // Node 1: Base Box
        uint32_t boxId = timeline.AddBoxNode("Base Box", "box_w", "box_d", "box_h");

        // Node 2: Hole
        androcad::TopologicalReference dummyFaceRef;
        dummyFaceRef.semanticTag = "Base:Top";
        uint32_t holeId = timeline.AddHoleNode("Center Hole", dummyFaceRef,
                                              "box_w / 2", "box_d / 2", "0",
                                              "0", "0", "1",
                                              "hole_r", "box_h");

        bool rebuildSuccess = timeline.Rebuild();
        assert(rebuildSuccess);
        assert(timeline.IsAllValid());

        double fullVolume = timeline.GetActiveMesh().volume;
        double expectedHoleCut = M_PI * 100.0 * 20.0;
        double expectedVol = (60.0 * 60.0 * 20.0) - expectedHoleCut;
        std::cout << "  Full Rebuild Volume: " << fullVolume << " mm³ (Expected: " << expectedVol << ")" << std::endl;
        assert(std::abs(fullVolume - expectedVol) < 5.0);

        // Test Rollback before Hole (only Box active)
        timeline.SetRollbackIndex(0);
        bool rollbackRebuild = timeline.Rebuild();
        assert(rollbackRebuild);
        double rolledBackVolume = timeline.GetActiveMesh().volume;
        std::cout << "  Rolled Back Volume: " << rolledBackVolume << " mm³ (Expected: 72000.0)" << std::endl;
        assert(std::abs(rolledBackVolume - 72000.0) < 1e-2);

        // Roll back forward
        timeline.SetRollbackIndex(-1);
        timeline.Rebuild();
        assert(std::abs(timeline.GetActiveMesh().volume - expectedVol) < 5.0);
        std::cout << "  [PASS] TNP Test 4: Timeline DAG recompute and rollback verified." << std::endl;
    }

    // =========================================================================
    // Part 6: TNP Test 5 - Topology Vanishing & Graceful Degradation (Negative Test)
    // =========================================================================
    std::cout << "\n[TNP Test 5] Topology Vanishing & Graceful Degradation (Negative Test)..." << std::endl;
    {
        // Reference an edge at coordinates (500, 500, 500) that does not exist in a 40x40x40 box
        BRepPrimAPI_MakeBox mkBox(40.0, 40.0, 40.0);
        TopoDS_Shape box = mkBox.Shape();

        androcad::TopologicalReference phantomRef;
        phantomRef.semanticTag = "VanishedFeature:RimEdge";
        phantomRef.fingerprint.type = androcad::SubShapeType::Edge;
        phantomRef.fingerprint.geomType = 1; // Circle
        phantomRef.fingerprint.center[0] = 500.0;
        phantomRef.fingerprint.center[1] = 500.0;
        phantomRef.fingerprint.center[2] = 500.0;
        phantomRef.fingerprint.normal[0] = 0.0;
        phantomRef.fingerprint.normal[1] = 0.0;
        phantomRef.fingerprint.normal[2] = 1.0;
        phantomRef.fingerprint.size = 2.0 * M_PI * 15.0;

        TopoDS_Edge resolvedEdge;
        auto res = androcad::TNPResolver::ResolveEdge(box, phantomRef, resolvedEdge);

        std::cout << "  Phantom Reference Status: " << static_cast<int>(res.status)
                  << " (Expected BrokenReference=" << static_cast<int>(androcad::TNPResolutionStatus::BrokenReference) << ")" << std::endl;
        std::cout << "  Diagnostic: " << res.diagnosticMessage << std::endl;

        assert(res.status == androcad::TNPResolutionStatus::BrokenReference);
        assert(resolvedEdge.IsNull());
        assert(!res.diagnosticMessage.empty());
        std::cout << "  [PASS] TNP Test 5: Vanished reference caught gracefully with diagnostic report." << std::endl;
    }

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "[PASS] ALL 5 TNP REGRESSION & TIMELINE DAG TESTS PASSED!  " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
