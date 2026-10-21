// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdio>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include "ProjectIO.h"
#include "FeatureOps.h"
#include "TimelineDAG.h"
#include "TNPResolver.h"

using namespace androcad;

void TestSTEPExportImportRoundtrip() {
    std::cout << "[AndroCAD Test] Running STEP AP214 Export & Import Roundtrip Test..." << std::endl;

    // 1. Create a box 100 x 50 x 30 mm
    BRepPrimAPI_MakeBox boxMaker(100.0, 50.0, 30.0);
    boxMaker.Build();
    assert(boxMaker.IsDone());
    TopoDS_Shape box = boxMaker.Shape();

    // 2. Drill a hole R=10 mm, H=30 mm along Z at (50, 25, 0)
    gp_Ax2 holeAx(gp_Pnt(50.0, 25.0, -1.0), gp_Dir(0, 0, 1));
    BRepPrimAPI_MakeCylinder cylMaker(holeAx, 10.0, 32.0);
    cylMaker.Build();
    assert(cylMaker.IsDone());

    BRepAlgoAPI_Cut cutOp(box, cylMaker.Shape());
    cutOp.Build();
    assert(cutOp.IsDone());
    TopoDS_Shape originalShape = cutOp.Shape();

    double originalVolume = ProjectIO::ComputeVolume(originalShape);
    double expectedVolume = (100.0 * 50.0 * 30.0) - (M_PI * 10.0 * 10.0 * 30.0);
    std::cout << "  Original Model Volume: " << originalVolume << " mm³ (Expected: ~" << expectedVolume << " mm³)" << std::endl;
    assert(std::abs(originalVolume - expectedVolume) < 1.0);

    // 3. Export to STEP file
    std::string stepPath = "test_model_roundtrip.step";
    IOResult expRes = ProjectIO::ExportSTEP(originalShape, stepPath, "AP214");
    std::cout << "  Export STEP result: " << (expRes.success ? "SUCCESS" : "FAILED") << std::endl;
    if (!expRes.success) std::cout << "    Error: " << expRes.errorMessage << std::endl;
    assert(expRes.success);

    // 4. Import from STEP file
    TopoDS_Shape importedShape;
    IOResult impRes = ProjectIO::ImportSTEP(stepPath, importedShape);
    std::cout << "  Import STEP result: " << (impRes.success ? "SUCCESS" : "FAILED") << std::endl;
    if (!impRes.success) std::cout << "    Error: " << impRes.errorMessage << std::endl;
    assert(impRes.success);
    assert(!importedShape.IsNull());

    // 5. Verify geometric invariants (Volume, Face Count)
    double importedVolume = ProjectIO::ComputeVolume(importedShape);
    std::cout << "  Imported Model Volume: " << importedVolume << " mm³" << std::endl;
    double volDiff = std::abs(originalVolume - importedVolume);
    std::cout << "  Volume Conservation Delta: " << volDiff << " mm³" << std::endl;
    assert(volDiff < 1e-2);

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(importedShape, TopAbs_FACE, faceMap);
    std::cout << "  Imported Face Count: " << faceMap.Extent() << " (Expected: 7)" << std::endl;
    assert(faceMap.Extent() == 7);

    // Clean up temporary file
    std::remove(stepPath.c_str());
    std::cout << "  [PASS] STEP AP214 roundtrip test verified successfully!" << std::endl;
}

void TestSTLExportImportRoundtrip() {
    std::cout << "[AndroCAD Test] Running STL Binary Export & Import Roundtrip Test..." << std::endl;

    BRepPrimAPI_MakeBox boxMaker(60.0, 40.0, 20.0);
    boxMaker.Build();
    assert(boxMaker.IsDone());
    TopoDS_Shape box = boxMaker.Shape();

    std::string stlPath = "test_model_roundtrip.stl";

    // 1. Export to binary STL
    IOResult expRes = ProjectIO::ExportSTL(box, stlPath, 0.1, 0.5, true);
    std::cout << "  Export STL result: " << (expRes.success ? "SUCCESS" : "FAILED") << std::endl;
    assert(expRes.success);

    // 2. Import from STL
    TopoDS_Shape importedStl;
    IOResult impRes = ProjectIO::ImportSTL(stlPath, importedStl);
    std::cout << "  Import STL result: " << (impRes.success ? "SUCCESS" : "FAILED") << std::endl;
    assert(impRes.success);
    assert(!importedStl.IsNull());

    // 3. Verify Bounding Box
    Bnd_Box bndBox;
    BRepBndLib::Add(importedStl, bndBox);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    bndBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    std::cout << "  STL Bounding Box: [" << (xmax - xmin) << " x " << (ymax - ymin) << " x " << (zmax - zmin) << "]" << std::endl;

    assert(std::abs((xmax - xmin) - 60.0) < 0.5);
    assert(std::abs((ymax - ymin) - 40.0) < 0.5);
    assert(std::abs((zmax - zmin) - 20.0) < 0.5);

    std::remove(stlPath.c_str());
    std::cout << "  [PASS] STL roundtrip test verified successfully!" << std::endl;
}

void TestProjectJsonSerializationRoundtrip() {
    std::cout << "[AndroCAD Test] Running Native Project (.androcad JSON) Roundtrip Test..." << std::endl;

    // 1. Construct parametric model in TimelineDAG
    TimelineDAG origDag;
    origDag.SetParameter("width", "120.0");
    origDag.SetParameter("depth", "60.0");
    origDag.SetParameter("height", "width / 3"); // 40.0
    origDag.SetParameter("filletRad", "5.0");

    uint32_t boxId = origDag.AddBoxNode("BaseBox", "width", "depth", "height");
    assert(boxId > 0);

    bool ok = origDag.Rebuild();
    assert(ok);
    assert(origDag.IsAllValid());

    TopoDS_Shape boxShape = origDag.GetActiveShape();
    TopTools_IndexedMapOfShape edgeMap;
    TopExp::MapShapes(boxShape, TopAbs_EDGE, edgeMap);
    assert(edgeMap.Extent() == 12);

    TopoDS_Edge topEdge = TopoDS::Edge(edgeMap(1));
    TopologicalReference edgeRef = TNPResolver::CreateEdgeReference(boxShape, topEdge, "BaseBox:Edge_1");

    uint32_t filletId = origDag.AddFilletNode("EdgeFillet", edgeRef, "filletRad");
    assert(filletId > 0);

    ok = origDag.Rebuild();
    assert(ok);
    assert(origDag.IsAllValid());

    double origVolume = ProjectIO::ComputeVolume(origDag.GetActiveShape());
    std::cout << "  Original Project Volume: " << origVolume << " mm³" << std::endl;

    // 2. Save project to file
    std::string projPath = "test_parametric_project.androcad";
    IOResult saveRes = ProjectIO::SaveProject(origDag, projPath);
    std::cout << "  SaveProject result: " << (saveRes.success ? "SUCCESS" : "FAILED") << ", nodes=" << saveRes.entitiesProcessed << std::endl;
    assert(saveRes.success);
    assert(saveRes.entitiesProcessed == 2);

    // 3. Load project into fresh TimelineDAG
    TimelineDAG loadedDag;
    IOResult loadRes = ProjectIO::LoadProject(loadedDag, projPath);
    std::cout << "  LoadProject result: " << (loadRes.success ? "SUCCESS" : "FAILED") << ", nodes=" << loadRes.entitiesProcessed << std::endl;
    if (!loadRes.success) std::cout << "    Error: " << loadRes.errorMessage << std::endl;
    assert(loadRes.success);

    // 4. Verify parameter restoration & DAG rebuild
    assert(loadedDag.HasParameter("width"));
    assert(loadedDag.GetParameterExpression("height") == "width / 3");
    assert(loadedDag.GetNodeCount() == 2);
    assert(loadedDag.IsAllValid());

    double loadedVolume = ProjectIO::ComputeVolume(loadedDag.GetActiveShape());
    std::cout << "  Loaded Project Recomputed Volume: " << loadedVolume << " mm³" << std::endl;
    assert(std::abs(origVolume - loadedVolume) < 1e-2);

    std::remove(projPath.c_str());
    std::cout << "  [PASS] Native .androcad project serialization verified successfully!" << std::endl;
}

void TestCorruptAndSafetyChecks() {
    std::cout << "[AndroCAD Test] Running Data Exchange Robustness & Safety Checks..." << std::endl;

    // 1. Non-existent file safety
    TopoDS_Shape dummyShape;
    IOResult nonExistentStep = ProjectIO::ImportSTEP("non_existent_file.step", dummyShape);
    assert(!nonExistentStep.success);
    assert(!nonExistentStep.errorMessage.empty());

    IOResult nonExistentStl = ProjectIO::ImportSTL("non_existent_file.stl", dummyShape);
    assert(!nonExistentStl.success);
    assert(!nonExistentStl.errorMessage.empty());

    TimelineDAG dummyDag;
    IOResult nonExistentProj = ProjectIO::LoadProject(dummyDag, "non_existent_file.androcad");
    assert(!nonExistentProj.success);

    // 2. Null shape export safety
    TopoDS_Shape nullShape;
    IOResult nullStepExp = ProjectIO::ExportSTEP(nullShape, "null.step");
    assert(!nullStepExp.success);

    IOResult nullStlExp = ProjectIO::ExportSTL(nullShape, "null.stl");
    assert(!nullStlExp.success);

    std::cout << "  [PASS] All robustness and error guards verified without crashes!" << std::endl;
}

void TestSampleDesignsHostRebuild() {
    std::cout << "[AndroCAD Test] Running Sample Designs Host Rebuild & Parameter Mutation..." << std::endl;

    // 1. L-Bracket
    {
        TimelineDAG dag;
        dag.SetParameter("base_len", "80.0");
        dag.SetParameter("width", "40.0");
        dag.SetParameter("base_thk", "12.0");
        dag.SetParameter("hole_rad", "5.0");

        dag.AddBoxNode("BaseLimb", "base_len", "width", "base_thk");
        TopologicalReference dummyRef;
        dag.AddHoleNode("MountHole1", dummyRef, "20.0", "20.0", "base_thk", "0.0", "0.0", "-1.0", "hole_rad", "15.0");
        dag.AddHoleNode("MountHole2", dummyRef, "60.0", "20.0", "base_thk", "0.0", "0.0", "-1.0", "hole_rad", "15.0");

        assert(dag.Rebuild());
        double vol = ProjectIO::ComputeVolume(dag.GetActiveShape());
        std::cout << "  L-Bracket Initial Volume: " << vol << " mm³ (Expected: ~36515 mm³)" << std::endl;
        assert(std::abs(vol - 36515.04) < 100.0);

        // Mutate parameter
        dag.SetParameter("base_len", "100.0");
        assert(dag.Rebuild());
        double mutatedVol = ProjectIO::ComputeVolume(dag.GetActiveShape());
        std::cout << "  L-Bracket Mutated Volume (base_len=100): " << mutatedVol << " mm³ (Expected: ~46115 mm³)" << std::endl;
        assert(std::abs(mutatedVol - 46115.04) < 100.0);

        // Save and Load roundtrip
        std::string tmpPath = "tmp_l_bracket.androcad";
        assert(ProjectIO::SaveProject(dag, tmpPath).success);
        TimelineDAG loaded;
        assert(ProjectIO::LoadProject(loaded, tmpPath).success);
        assert(loaded.Rebuild());
        double roundtripVol = ProjectIO::ComputeVolume(loaded.GetActiveShape());
        assert(std::abs(roundtripVol - mutatedVol) < 1.0);
        std::remove(tmpPath.c_str());
    }

    // 2. Flanged Bushing
    {
        TimelineDAG dag;
        dag.SetParameter("flange_dx", "60.0");
        dag.SetParameter("flange_dy", "60.0");
        dag.SetParameter("flange_thk", "10.0");
        dag.SetParameter("bore_radius", "12.0");

        dag.AddBoxNode("FlangeBody", "flange_dx", "flange_dy", "flange_thk");
        TopologicalReference dummyRef;
        dag.AddHoleNode("CentralShaftBore", dummyRef, "30.0", "30.0", "flange_thk", "0.0", "0.0", "-1.0", "bore_radius", "15.0");

        assert(dag.Rebuild());
        double vol = ProjectIO::ComputeVolume(dag.GetActiveShape());
        std::cout << "  Flanged Bushing Initial Volume: " << vol << " mm³ (Expected: ~31476 mm³)" << std::endl;
        assert(std::abs(vol - 31476.11) < 100.0);

        // Mutate parameter
        dag.SetParameter("bore_radius", "10.0");
        assert(dag.Rebuild());
        double mutatedVol = ProjectIO::ComputeVolume(dag.GetActiveShape());
        std::cout << "  Flanged Bushing Mutated Volume (bore_radius=10): " << mutatedVol << " mm³ (Expected: ~32858 mm³)" << std::endl;
        assert(std::abs(mutatedVol - 32858.41) < 100.0);
    }

    // 3. Phone Stand
    {
        TimelineDAG dag;
        dag.SetParameter("stand_len", "70.0");
        dag.SetParameter("stand_width", "50.0");
        dag.SetParameter("stand_height", "25.0");
        dag.SetParameter("cable_slot_rad", "6.0");

        dag.AddBoxNode("StandBaseBlock", "stand_len", "stand_width", "stand_height");
        TopologicalReference dummyRef;
        dag.AddHoleNode("CablePassSlot", dummyRef, "35.0", "25.0", "stand_height", "0.0", "0.0", "-1.0", "cable_slot_rad", "30.0");

        assert(dag.Rebuild());
        double vol = ProjectIO::ComputeVolume(dag.GetActiveShape());
        std::cout << "  Phone Stand Initial Volume: " << vol << " mm³ (Expected: ~84672 mm³)" << std::endl;
        assert(std::abs(vol - 84672.57) < 100.0);

        // Mutate parameter
        dag.SetParameter("stand_height", "30.0");
        assert(dag.Rebuild());
        double mutatedVol = ProjectIO::ComputeVolume(dag.GetActiveShape());
        std::cout << "  Phone Stand Mutated Volume (stand_height=30): " << mutatedVol << " mm³ (Expected: ~101607 mm³)" << std::endl;
        assert(std::abs(mutatedVol - 101607.08) < 100.0);
    }

    // 4. Perforated Plate
    {
        TimelineDAG dag;
        dag.SetParameter("plate_len", "100.0");
        dag.SetParameter("plate_width", "80.0");
        dag.SetParameter("plate_thk", "8.0");
        dag.SetParameter("hole_rad", "4.0");

        dag.AddBoxNode("PlateBase", "plate_len", "plate_width", "plate_thk");
        TopologicalReference dummyRef;
        dag.AddHoleNode("Perforation_1", dummyRef, "25.0", "25.0", "plate_thk", "0.0", "0.0", "-1.0", "hole_rad", "12.0");
        dag.AddHoleNode("Perforation_2", dummyRef, "75.0", "25.0", "plate_thk", "0.0", "0.0", "-1.0", "hole_rad", "12.0");
        dag.AddHoleNode("Perforation_3", dummyRef, "25.0", "55.0", "plate_thk", "0.0", "0.0", "-1.0", "hole_rad", "12.0");
        dag.AddHoleNode("Perforation_4", dummyRef, "75.0", "55.0", "plate_thk", "0.0", "0.0", "-1.0", "hole_rad", "12.0");

        assert(dag.Rebuild());
        double vol = ProjectIO::ComputeVolume(dag.GetActiveShape());
        std::cout << "  Perforated Plate Initial Volume: " << vol << " mm³ (Expected: ~62391 mm³)" << std::endl;
        assert(std::abs(vol - 62391.50) < 100.0);

        // Mutate parameter
        dag.SetParameter("plate_thk", "10.0");
        assert(dag.Rebuild());
        double mutatedVol = ProjectIO::ComputeVolume(dag.GetActiveShape());
        std::cout << "  Perforated Plate Mutated Volume (plate_thk=10): " << mutatedVol << " mm³ (Expected: ~77989 mm³)" << std::endl;
        assert(std::abs(mutatedVol - 77989.38) < 100.0);
    }

    std::cout << "  [PASS] All 4 original sample designs rebuild and parameter recomputations passed!" << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << " AndroCAD Milestone 6: Versioned Project I/O & Data Exchange " << std::endl;
    std::cout << "==========================================================" << std::endl;

    TestSTEPExportImportRoundtrip();
    TestSTLExportImportRoundtrip();
    TestProjectJsonSerializationRoundtrip();
    TestCorruptAndSafetyChecks();
    TestSampleDesignsHostRebuild();

    std::cout << "\n[PASS] All Milestone 6 Data Exchange & Project I/O Tests Passed Successfully!" << std::endl;
    return 0;
}
