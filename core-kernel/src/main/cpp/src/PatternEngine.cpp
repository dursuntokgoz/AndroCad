// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "PatternEngine.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeCompound.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Standard_Failure.hxx>

#include <cmath>

namespace androcad {

namespace {

constexpr double M_PI_VAL = 3.14159265358979323846;

double ComputeVolume(const TopoDS_Shape& shape) {
    if (shape.IsNull()) return 0.0;
    try {
        GProp_GProps props;
        BRepGProp::VolumeProperties(shape, props);
        return props.Mass();
    } catch (...) {
        return 0.0;
    }
}

} // namespace

TopoDS_Shape PatternEngine::TransformShape(const TopoDS_Shape& shape, const gp_Trsf& trsf) {
    if (shape.IsNull()) return shape;
    BRepBuilderAPI_Transform transformer(shape, trsf, Standard_True);
    return transformer.Shape();
}

PatternResult PatternEngine::RectangularPattern(
        const TopoDS_Shape& baseShape,
        int countU, double spacingU, const gp_Vec& dirU,
        int countV, double spacingV, const gp_Vec& dirV,
        bool fuseInstances) {
    PatternResult result;
    if (baseShape.IsNull() || countU < 1 || countV < 1) {
        result.success = false;
        result.errorMessage = "Invalid base shape or instance counts";
        return result;
    }

    try {
        gp_Vec uNorm = dirU;
        if (uNorm.Magnitude() > 1e-6) uNorm.Normalize();

        gp_Vec vNorm = dirV;
        if (vNorm.Magnitude() > 1e-6) vNorm.Normalize();

        std::vector<TopoDS_Shape> instances;
        instances.reserve(countU * countV);

        for (int u = 0; u < countU; ++u) {
            for (int v = 0; v < countV; ++v) {
                if (u == 0 && v == 0) {
                    instances.push_back(baseShape);
                    continue;
                }

                gp_Vec shift = uNorm * (u * spacingU) + vNorm * (v * spacingV);
                gp_Trsf t;
                t.SetTranslation(shift);
                TopoDS_Shape copy = TransformShape(baseShape, t);
                instances.push_back(copy);
            }
        }

        if (fuseInstances && instances.size() > 1) {
            TopoDS_Shape merged = instances[0];
            for (size_t i = 1; i < instances.size(); ++i) {
                BRepAlgoAPI_Fuse fuser(merged, instances[i]);
                fuser.Build();
                if (fuser.IsDone()) {
                    merged = fuser.Shape();
                }
            }
            result.shape = merged;
        } else {
            BRepBuilderAPI_MakeCompound compoundMaker;
            for (const auto& inst : instances) {
                compoundMaker.Add(inst);
            }
            result.shape = compoundMaker.Shape();
        }

        result.success = true;
        result.instanceCount = static_cast<int>(instances.size());
        result.totalVolumeMm3 = ComputeVolume(result.shape);
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = e.GetMessageString();
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

PatternResult PatternEngine::CircularPattern(
        const TopoDS_Shape& baseShape,
        const gp_Ax1& axis,
        int count,
        double totalAngleDeg,
        bool fuseInstances) {
    PatternResult result;
    if (baseShape.IsNull() || count < 1) {
        result.success = false;
        result.errorMessage = "Invalid base shape or count";
        return result;
    }

    try {
        double stepAngleRad = (totalAngleDeg / (std::abs(totalAngleDeg - 360.0) < 1e-4 ? count : (count - 1))) * (M_PI_VAL / 180.0);
        if (count == 1) stepAngleRad = 0.0;

        std::vector<TopoDS_Shape> instances;
        instances.reserve(count);

        for (int i = 0; i < count; ++i) {
            if (i == 0) {
                instances.push_back(baseShape);
                continue;
            }

            gp_Trsf t;
            t.SetRotation(axis, i * stepAngleRad);
            TopoDS_Shape copy = TransformShape(baseShape, t);
            instances.push_back(copy);
        }

        if (fuseInstances && instances.size() > 1) {
            TopoDS_Shape merged = instances[0];
            for (size_t i = 1; i < instances.size(); ++i) {
                BRepAlgoAPI_Fuse fuser(merged, instances[i]);
                fuser.Build();
                if (fuser.IsDone()) {
                    merged = fuser.Shape();
                }
            }
            result.shape = merged;
        } else {
            BRepBuilderAPI_MakeCompound compoundMaker;
            for (const auto& inst : instances) {
                compoundMaker.Add(inst);
            }
            result.shape = compoundMaker.Shape();
        }

        result.success = true;
        result.instanceCount = static_cast<int>(instances.size());
        result.totalVolumeMm3 = ComputeVolume(result.shape);
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = e.GetMessageString();
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

PatternResult PatternEngine::Mirror(
        const TopoDS_Shape& baseShape,
        const gp_Ax2& mirrorPlane,
        bool keepOriginal,
        bool fuseInstances) {
    PatternResult result;
    if (baseShape.IsNull()) {
        result.success = false;
        result.errorMessage = "Invalid base shape";
        return result;
    }

    try {
        gp_Trsf t;
        t.SetMirror(mirrorPlane);
        TopoDS_Shape mirrored = TransformShape(baseShape, t);

        if (!keepOriginal) {
            result.shape = mirrored;
            result.instanceCount = 1;
        } else {
            if (fuseInstances) {
                BRepAlgoAPI_Fuse fuser(baseShape, mirrored);
                fuser.Build();
                if (fuser.IsDone()) {
                    result.shape = fuser.Shape();
                } else {
                    BRepBuilderAPI_MakeCompound comp;
                    comp.Add(baseShape);
                    comp.Add(mirrored);
                    result.shape = comp.Shape();
                }
            } else {
                BRepBuilderAPI_MakeCompound comp;
                comp.Add(baseShape);
                comp.Add(mirrored);
                result.shape = comp.Shape();
            }
            result.instanceCount = 2;
        }

        result.success = true;
        result.totalVolumeMm3 = ComputeVolume(result.shape);
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = e.GetMessageString();
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

PatternResult PatternEngine::ApplyDraft(
        const TopoDS_Shape& baseShape,
        const gp_Dir& pullDirection,
        double draftAngleDeg,
        const std::vector<int>& targetFaceIds) {
    PatternResult result;
    if (baseShape.IsNull() || targetFaceIds.empty()) {
        result.success = false;
        result.errorMessage = "Invalid base shape or empty target face list";
        return result;
    }

    try {
        BRepOffsetAPI_DraftAngle drafter(baseShape);
        double angleRad = draftAngleDeg * (M_PI_VAL / 180.0);
        gp_Pln neutralPlane(gp_Pnt(0, 0, 0), pullDirection);

        int currentId = 1;
        for (TopExp_Explorer exp(baseShape, TopAbs_FACE); exp.More(); exp.Next(), ++currentId) {
            bool matches = false;
            for (int fid : targetFaceIds) {
                if (fid == currentId) { matches = true; break; }
            }
            if (matches) {
                TopoDS_Face face = TopoDS::Face(exp.Current());
                drafter.Add(face, pullDirection, angleRad, neutralPlane);
            }
        }

        drafter.Build();
        if (!drafter.IsDone()) {
            result.success = false;
            result.errorMessage = "Draft operation failed to solve";
            return result;
        }

        result.shape = drafter.Shape();
        result.success = true;
        result.instanceCount = 1;
        result.totalVolumeMm3 = ComputeVolume(result.shape);
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = e.GetMessageString();
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }

    return result;
}

} // namespace androcad
