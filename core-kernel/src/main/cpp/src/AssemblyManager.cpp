// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "AssemblyManager.h"
#include "FeatureOps.h"

#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopLoc_Location.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <gp_Ax1.hxx>
#include <Standard_Failure.hxx>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace androcad {

AssemblyManager::AssemblyManager() = default;

int AssemblyManager::AddComponent(const std::string& name, const TopoDS_Shape& shape) {
    if (shape.IsNull()) {
        throw std::invalid_argument("Cannot add null shape as assembly component");
    }

    Component comp;
    comp.id = m_nextComponentId++;
    comp.name = name;
    comp.localShape = shape;
    comp.localTransform = gp_Trsf();
    comp.worldTransform = gp_Trsf();
    comp.parentJointId = -1;

    m_components[comp.id] = comp;
    return comp.id;
}

bool AssemblyManager::RemoveComponent(int componentId) {
    auto it = m_components.find(componentId);
    if (it == m_components.end()) return false;

    // Remove any joints associated with this component
    std::vector<int> jointsToRemove;
    for (const auto& pair : m_joints) {
        if (pair.second.parentComponentId == componentId || pair.second.childComponentId == componentId) {
            jointsToRemove.push_back(pair.first);
        }
    }
    for (int jId : jointsToRemove) {
        m_joints.erase(jId);
    }

    m_components.erase(it);
    SolveForwardKinematics();
    return true;
}

Component* AssemblyManager::GetComponent(int componentId) {
    auto it = m_components.find(componentId);
    return (it != m_components.end()) ? &it->second : nullptr;
}

int AssemblyManager::AddJoint(const std::string& name,
                              JointType type,
                              int parentComponentId,
                              int childComponentId,
                              const gp_Pnt& pivot,
                              const gp_Dir& axis,
                              double minLimit,
                              double maxLimit) {
    if (m_components.find(childComponentId) == m_components.end()) {
        throw std::invalid_argument("Child component does not exist in assembly");
    }
    if (parentComponentId != -1 && m_components.find(parentComponentId) == m_components.end()) {
        throw std::invalid_argument("Parent component does not exist in assembly");
    }

    KinematicJoint joint;
    joint.id = m_nextJointId++;
    joint.name = name;
    joint.type = type;
    joint.parentComponentId = parentComponentId;
    joint.childComponentId = childComponentId;
    joint.pivotPoint = pivot;
    joint.axis = axis;
    joint.minLimit = minLimit;
    joint.maxLimit = maxLimit;
    joint.currentValue = (minLimit < maxLimit) ? std::clamp(0.0, minLimit, maxLimit) : 0.0;

    m_joints[joint.id] = joint;
    m_components[childComponentId].parentJointId = joint.id;

    SolveForwardKinematics();
    return joint.id;
}

bool AssemblyManager::SetJointValue(int jointId, double value) {
    auto it = m_joints.find(jointId);
    if (it == m_joints.end()) return false;

    KinematicJoint& j = it->second;
    if (j.minLimit < j.maxLimit) {
        j.currentValue = std::clamp(value, j.minLimit, j.maxLimit);
    } else {
        j.currentValue = value;
    }

    SolveForwardKinematics();
    return true;
}

KinematicJoint* AssemblyManager::GetJoint(int jointId) {
    auto it = m_joints.find(jointId);
    return (it != m_joints.end()) ? &it->second : nullptr;
}

void AssemblyManager::SolveForwardKinematics() {
    // 1. Reset all components to initial local transforms
    for (auto& pair : m_components) {
        if (pair.second.parentJointId == -1) {
            pair.second.worldTransform = pair.second.localTransform;
        }
    }

    // 2. Resolve joint kinematics down the hierarchy
    bool changed = true;
    int maxIterations = static_cast<int>(m_components.size()) + 2;
    int iter = 0;

    while (changed && iter < maxIterations) {
        changed = false;
        iter++;

        for (const auto& jPair : m_joints) {
            const KinematicJoint& j = jPair.second;
            Component* child = GetComponent(j.childComponentId);
            if (!child) continue;

            gp_Trsf parentWorld;
            if (j.parentComponentId != -1) {
                Component* parent = GetComponent(j.parentComponentId);
                if (parent) {
                    parentWorld = parent->worldTransform;
                }
            }

            gp_Trsf jointDelta;

            switch (j.type) {
                case JointType::Rigid:
                    jointDelta = gp_Trsf();
                    break;

                case JointType::Revolute: {
                    gp_Ax1 rotAxis(j.pivotPoint, j.axis);
                    jointDelta.SetRotation(rotAxis, j.currentValue);
                    break;
                }

                case JointType::Slider: {
                    gp_Vec shift(j.axis.X() * j.currentValue,
                                 j.axis.Y() * j.currentValue,
                                 j.axis.Z() * j.currentValue);
                    jointDelta.SetTranslation(shift);
                    break;
                }

                case JointType::Cylindrical: {
                    gp_Ax1 rotAxis(j.pivotPoint, j.axis);
                    gp_Trsf rot, trans;
                    rot.SetRotation(rotAxis, j.currentValue);
                    gp_Vec shift(j.axis.X() * j.currentValue,
                                 j.axis.Y() * j.currentValue,
                                 j.axis.Z() * j.currentValue);
                    trans.SetTranslation(shift);
                    jointDelta = trans * rot;
                    break;
                }

                case JointType::Ball: {
                    gp_Ax1 rotAxis(j.pivotPoint, j.axis);
                    jointDelta.SetRotation(rotAxis, j.currentValue);
                    break;
                }
            }

            // childWorld = parentWorld * jointDelta * childLocal
            gp_Trsf newWorld = parentWorld * jointDelta * child->localTransform;
            child->worldTransform = newWorld;
        }
    }
}

bool AssemblyManager::CheckInterference(int componentIdA, int componentIdB) {
    Component* compA = GetComponent(componentIdA);
    Component* compB = GetComponent(componentIdB);
    if (!compA || !compB) return false;

    try {
        TopoDS_Shape shapeA = compA->localShape.Moved(TopLoc_Location(compA->worldTransform));
        TopoDS_Shape shapeB = compB->localShape.Moved(TopLoc_Location(compB->worldTransform));

        BRepAlgoAPI_Common common(shapeA, shapeB);
        common.SetFuzzyValue(1e-5);
        common.Build();

        if (common.IsDone() && !common.Shape().IsNull()) {
            GProp_GProps gprops;
            BRepGProp::VolumeProperties(common.Shape(), gprops);
            return gprops.Mass() > 1e-4; // Substantial volume intersection
        }
    } catch (const Standard_Failure&) {
        return false;
    }

    return false;
}

void AssemblyManager::StepAnimation(double phase) {
    for (auto& pair : m_joints) {
        KinematicJoint& j = pair.second;
        if (j.minLimit < j.maxLimit) {
            double normalized = 0.5 * (1.0 + std::sin(phase * 2.0 * M_PI));
            j.currentValue = j.minLimit + (j.maxLimit - j.minLimit) * normalized;
        }
    }
    SolveForwardKinematics();
}

MeshPayload AssemblyManager::BuildAssemblyMeshPayload(double linearDeflection,
                                                      double angularDeflection) {
    if (m_components.empty()) {
        MeshPayload empty;
        empty.success = true;
        return empty;
    }

    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    for (const auto& pair : m_components) {
        const Component& comp = pair.second;
        TopoDS_Shape worldShape = comp.localShape.Moved(TopLoc_Location(comp.worldTransform));
        builder.Add(compound, worldShape);
    }

    return FeatureOps::ShapeToMeshPayload(compound, linearDeflection, angularDeflection);
}

} // namespace androcad
