// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_ASSEMBLY_MANAGER_H
#define ANDROCAD_ASSEMBLY_MANAGER_H

#include "AndroCadKernel.h"

#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace androcad {

enum class JointType {
    Rigid = 0,       // 0 DoF: child is locked rigidly to parent
    Revolute = 1,    // 1 DoF: rotational motion about an axis
    Slider = 2,      // 1 DoF: translational motion along an axis
    Cylindrical = 3, // 2 DoF: rotation + translation along shared axis
    Ball = 4         // 3 DoF: spherical 3D rotation around pivot point
};

struct KinematicJoint {
    int id;
    std::string name;
    JointType type;
    int parentComponentId; // -1 for world/ground base
    int childComponentId;
    gp_Pnt pivotPoint;     // Joint origin in parent space
    gp_Dir axis;           // Motion axis (rotation or slide direction)
    double minLimit;       // Angular (rad) or linear (mm) lower bound
    double maxLimit;       // Upper bound
    double currentValue;   // Current angle (rad) or displacement (mm)
};

struct Component {
    int id;
    std::string name;
    TopoDS_Shape localShape;
    gp_Trsf localTransform; // Relative to parent component or base
    gp_Trsf worldTransform; // Absolute transformed coordinate system
    int parentJointId;      // Joint attaching this component (-1 if base)
};

class AssemblyManager {
public:
    AssemblyManager();
    ~AssemblyManager() = default;

    // Component management
    int AddComponent(const std::string& name, const TopoDS_Shape& shape);
    bool RemoveComponent(int componentId);
    Component* GetComponent(int componentId);
    size_t GetComponentCount() const { return m_components.size(); }

    // Kinematic joint management
    int AddJoint(const std::string& name,
                 JointType type,
                 int parentComponentId,
                 int childComponentId,
                 const gp_Pnt& pivot,
                 const gp_Dir& axis,
                 double minLimit,
                 double maxLimit);
    bool SetJointValue(int jointId, double value);
    KinematicJoint* GetJoint(int jointId);
    size_t GetJointCount() const { return m_joints.size(); }

    // Kinematics solver
    void SolveForwardKinematics();

    // Interference / collision check between two components
    bool CheckInterference(int componentIdA, int componentIdB);

    // Animates joint values forward by delta time factor t in [0, 1]
    void StepAnimation(double phase);

    // Produces aggregated world-transformed tessellation payload for rendering
    MeshPayload BuildAssemblyMeshPayload(double linearDeflection = 0.1,
                                         double angularDeflection = 0.5);

private:
    std::map<int, Component> m_components;
    std::map<int, KinematicJoint> m_joints;
    int m_nextComponentId{1};
    int m_nextJointId{1};
};

} // namespace androcad

#endif // ANDROCAD_ASSEMBLY_MANAGER_H
