// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "AssemblyManager.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <iostream>
#include <cassert>
#include <cmath>

using namespace androcad;

void TestRigidAssembly() {
    std::cout << "[Test 1/5] Testing Rigid Multi-Component Assembly..." << std::endl;
    AssemblyManager mgr;

    // Component 1: Base block 100 x 100 x 20
    TopoDS_Shape baseShape = BRepPrimAPI_MakeBox(100.0, 100.0, 20.0).Shape();
    int baseId = mgr.AddComponent("Base", baseShape);
    assert(baseId == 1);

    // Component 2: Upper post 20 x 20 x 80
    TopoDS_Shape postId = BRepPrimAPI_MakeBox(20.0, 20.0, 80.0).Shape();
    int childId = mgr.AddComponent("Post", postId);
    assert(childId == 2);

    // Rigid joint attaching child to base at (40, 40, 20)
    int jId = mgr.AddJoint("RigidJoint", JointType::Rigid, baseId, childId,
                           gp_Pnt(40.0, 40.0, 20.0), gp_Dir(0, 0, 1), 0.0, 0.0);
    assert(jId == 1);

    MeshPayload payload = mgr.BuildAssemblyMeshPayload();
    assert(payload.success);
    // Expected combined volume: (100*100*20) + (20*20*80) = 200,000 + 32,000 = 232,000 mm³
    assert(std::abs(payload.volume - 232000.0) < 10.0);
    std::cout << "  ✓ Combined assembly volume: " << payload.volume << " mm³" << std::endl;
}

void TestRevoluteJointLimits() {
    std::cout << "[Test 2/5] Testing Revolute Joint Limits & Angular Clamping..." << std::endl;
    AssemblyManager mgr;

    TopoDS_Shape base = BRepPrimAPI_MakeBox(50.0, 50.0, 20.0).Shape();
    int baseId = mgr.AddComponent("Base", base);

    TopoDS_Shape arm = BRepPrimAPI_MakeBox(80.0, 20.0, 20.0).Shape();
    int armId = mgr.AddComponent("RotatingArm", arm);

    // Limit between -PI/2 (-1.5708) and +PI/2 (+1.5708)
    double minAngle = -M_PI * 0.5;
    double maxAngle = M_PI * 0.5;
    int jointId = mgr.AddJoint("Hinge", JointType::Revolute, baseId, armId,
                               gp_Pnt(25.0, 25.0, 20.0), gp_Dir(0, 0, 1),
                               minAngle, maxAngle);

    // Test value clamping beyond limits
    mgr.SetJointValue(jointId, 3.14159); // Exceeds maxAngle
    KinematicJoint* j = mgr.GetJoint(jointId);
    assert(std::abs(j->currentValue - maxAngle) < 1e-4 && "Joint value must clamp to max limit");

    mgr.SetJointValue(jointId, -3.14159); // Below minAngle
    assert(std::abs(j->currentValue - minAngle) < 1e-4 && "Joint value must clamp to min limit");

    mgr.SetJointValue(jointId, 0.5);
    assert(std::abs(j->currentValue - 0.5) < 1e-4);

    std::cout << "  ✓ Revolute angle clamping verified within bounds [" << minAngle << ", " << maxAngle << "]" << std::endl;
}

void TestSliderJointDisplacement() {
    std::cout << "[Test 3/5] Testing Slider Joint Prismatic Motion..." << std::endl;
    AssemblyManager mgr;

    TopoDS_Shape track = BRepPrimAPI_MakeBox(200.0, 40.0, 20.0).Shape();
    int trackId = mgr.AddComponent("Track", track);

    TopoDS_Shape carriage = BRepPrimAPI_MakeBox(30.0, 30.0, 20.0).Shape();
    int carriageId = mgr.AddComponent("Carriage", carriage);

    int jointId = mgr.AddJoint("LinearAxis", JointType::Slider, trackId, carriageId,
                               gp_Pnt(0, 0, 20), gp_Dir(1, 0, 0), 0.0, 150.0);

    mgr.SetJointValue(jointId, 75.0);
    Component* carriageComp = mgr.GetComponent(carriageId);
    assert(carriageComp != nullptr);

    // Check that world transform translation component along X is 75.0
    gp_XYZ trans = carriageComp->worldTransform.TranslationPart();
    assert(std::abs(trans.X() - 75.0) < 1e-4);
    std::cout << "  ✓ Slider translation verified at X = " << trans.X() << " mm" << std::endl;
}

void TestTwoLinkKinematicChain() {
    std::cout << "[Test 4/5] Testing 2-Link Robot Arm Forward Kinematics..." << std::endl;
    AssemblyManager mgr;

    // Base pillar at origin
    TopoDS_Shape baseShape = BRepPrimAPI_MakeBox(20.0, 20.0, 20.0).Shape();
    int baseId = mgr.AddComponent("Base", baseShape);

    // Link 1: 100mm long along X
    TopoDS_Shape link1Shape = BRepPrimAPI_MakeBox(100.0, 10.0, 10.0).Shape();
    int link1Id = mgr.AddComponent("Link1", link1Shape);

    // Link 2: 80mm long along X
    TopoDS_Shape link2Shape = BRepPrimAPI_MakeBox(80.0, 10.0, 10.0).Shape();
    int link2Id = mgr.AddComponent("Link2", link2Shape);

    // Joint 1: Base to Link 1, Z-rotation
    int j1 = mgr.AddJoint("Shoulder", JointType::Revolute, baseId, link1Id,
                          gp_Pnt(0, 0, 20), gp_Dir(0, 0, 1), -M_PI, M_PI);

    // Joint 2: Link 1 to Link 2 at end of Link 1 (X=100), Z-rotation
    int j2 = mgr.AddJoint("Elbow", JointType::Revolute, link1Id, link2Id,
                          gp_Pnt(100.0, 0, 0), gp_Dir(0, 0, 1), -M_PI, M_PI);

    // Set Joint 1 = +90 deg (PI/2), Joint 2 = 0 deg
    mgr.SetJointValue(j1, M_PI * 0.5);
    mgr.SetJointValue(j2, 0.0);

    Component* l2 = mgr.GetComponent(link2Id);
    assert(l2 != nullptr);
    gp_XYZ t2 = l2->worldTransform.TranslationPart();
    // After 90 deg rotation, Link 1 points along +Y
    assert(std::abs(t2.X()) < 1.0 && "X coordinate should be ~0 after 90 deg rotation");
    assert(t2.Y() > 50.0 && "Y coordinate should be extended along +Y");

    std::cout << "  ✓ 2-Link Forward Kinematics End Position: ("
              << t2.X() << ", " << t2.Y() << ", " << t2.Z() << ")" << std::endl;
}

void TestAssemblyInterference() {
    std::cout << "[Test 5/5] Testing Component Interference & Collision Detection..." << std::endl;
    AssemblyManager mgr;

    // Component A at origin: 50 x 50 x 50
    TopoDS_Shape boxA = BRepPrimAPI_MakeBox(50.0, 50.0, 50.0).Shape();
    int idA = mgr.AddComponent("BoxA", boxA);

    // Component B: 50 x 50 x 50
    TopoDS_Shape boxB = BRepPrimAPI_MakeBox(50.0, 50.0, 50.0).Shape();
    int idB = mgr.AddComponent("BoxB", boxB);

    // Slider joint moving BoxB along X from 0 to 100mm
    int slider = mgr.AddJoint("Track", JointType::Slider, idA, idB,
                              gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0), 0.0, 100.0);

    // At X = 20mm (Overlapping: 20 < 50) -> Must detect interference!
    mgr.SetJointValue(slider, 20.0);
    bool collides = mgr.CheckInterference(idA, idB);
    assert(collides && "Collision must be detected when components overlap (X = 20mm)");

    // At X = 80mm (Separated: 80 > 50) -> No interference!
    mgr.SetJointValue(slider, 80.0);
    bool clear = mgr.CheckInterference(idA, idB);
    assert(!clear && "No collision should be detected when components are separated (X = 80mm)");

    std::cout << "  ✓ Interference detection validated: Collision at X=20mm, Clear at X=80mm" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running AndroCAD Kinematic Assemblies Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        TestRigidAssembly();
        TestRevoluteJointLimits();
        TestSliderJointDisplacement();
        TestTwoLinkKinematicChain();
        TestAssemblyInterference();

        std::cout << "\n>>> ALL 5 ASSEMBLY & KINEMATIC JOINT TESTS PASSED! <<<" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAILED: Exception: " << e.what() << std::endl;
        return 1;
    }
}
