// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include <jni.h>
#include "AndroCadKernel.h"
#include "Sketcher.h"
#include "FeatureOps.h"
#include "ExpressionParser.h"
#include "TNPResolver.h"
#include "TimelineDAG.h"
#include "ProjectIO.h"
#include "SurfaceOps.h"
#include "AssemblyManager.h"
#include "SheetMetalEngine.h"
#include "DrawingEngine.h"
#include "MaterialEngine.h"
#include "MeshEngine.h"
#include "SimulationEngine.h"
#include "CAMEngine.h"
#include "PatternEngine.h"
#include "FastenerEngine.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <TopoDS_Shape.hxx>
#include <Standard_Failure.hxx>
#include <mutex>
#include <unordered_set>
#include <cstdint>

#include <memory>
#include <vector>

namespace {

struct NativeMeshSession {
    androcad::MeshPayload payload;
    TopoDS_Shape shape;
};

struct NativeSketchSession {
    std::unique_ptr<androcad::Sketcher> sketcher;
    androcad::SketchResult lastResult;
};

struct NativeTimelineSession {
    std::unique_ptr<androcad::TimelineDAG> timeline;
};

struct NativeAssemblySession {
    std::unique_ptr<androcad::AssemblyManager> assembly;
};

template <typename T>
class SafeSessionRegistry {
public:
    jlong Register(std::unique_ptr<T> item) {
        if (!item) return 0;
        return Register(item.release());
    }

    jlong Register(T* item) {
        if (!item) return 0;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_active.insert(item);
        return reinterpret_cast<jlong>(item);
    }

    T* Get(jlong handle) {
        if (handle <= 1000L) return nullptr;
        std::lock_guard<std::mutex> lock(m_mutex);
        auto* ptr = reinterpret_cast<T*>(handle);
        if (m_active.find(ptr) != m_active.end()) {
            return ptr;
        }
        return nullptr;
    }

    void Release(jlong handle) {
        if (handle <= 1000L) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        auto* ptr = reinterpret_cast<T*>(handle);
        auto it = m_active.find(ptr);
        if (it != m_active.end()) {
            m_active.erase(it);
            delete ptr;
        }
    }

private:
    std::mutex m_mutex;
    std::unordered_set<T*> m_active;
};

static SafeSessionRegistry<NativeMeshSession> g_meshRegistry;
static SafeSessionRegistry<NativeSketchSession> g_sketchRegistry;
static SafeSessionRegistry<NativeTimelineSession> g_timelineRegistry;
static SafeSessionRegistry<NativeAssemblySession> g_assemblyRegistry;

} // namespace

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCreateBox(
        JNIEnv* /* env */,
        jobject /* this */,
        jdouble dx,
        jdouble dy,
        jdouble dz,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    
    auto session = std::make_unique<NativeMeshSession>();
    try {
        BRepPrimAPI_MakeBox boxMaker(dx, dy, dz);
        boxMaker.Build();
        if (boxMaker.IsDone()) {
            session->shape = boxMaker.Shape();
            session->payload = androcad::FeatureOps::ShapeToMeshPayload(session->shape, linearDeflection, angularDeflection);
        } else {
            session->payload.success = false;
            session->payload.errorMessage = "BRepPrimAPI_MakeBox failed to build box shape.";
        }
    } catch (const Standard_Failure& e) {
        session->payload.success = false;
        session->payload.errorMessage = e.GetMessageString();
    } catch (const std::exception& e) {
        session->payload.success = false;
        session->payload.errorMessage = e.what();
    } catch (...) {
        session->payload.success = false;
        session->payload.errorMessage = "nativeCreateBox failed with unknown native error";
    }

    return g_meshRegistry.Register(std::move(session));
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeIsSuccess(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    return (session && session->payload.success) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jdouble JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetVolume(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    return session ? session->payload.volume : 0.0;
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetFaceCount(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    return session ? session->payload.faceCount : 0;
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetEdgeCount(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    return session ? session->payload.edgeCount : 0;
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetTriangleCount(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    return session ? session->payload.triangleCount : 0;
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetErrorMessage(
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    if (!session || session->payload.errorMessage.empty()) {
        return nullptr;
    }
    return env->NewStringUTF(session->payload.errorMessage.c_str());
}

JNIEXPORT jobject JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetVertexBuffer(
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    if (!session || session->payload.vertices.empty()) {
        return nullptr;
    }
    jlong capacity = static_cast<jlong>(session->payload.vertices.size() * sizeof(androcad::Vertex));
    return env->NewDirectByteBuffer(session->payload.vertices.data(), capacity);
}

JNIEXPORT jobject JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetIndexBuffer(
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    if (!session || session->payload.indices.empty()) {
        return nullptr;
    }
    jlong capacity = static_cast<jlong>(session->payload.indices.size() * sizeof(uint32_t));
    return env->NewDirectByteBuffer(session->payload.indices.data(), capacity);
}

JNIEXPORT jobject JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetEdgeVertexBuffer(
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    auto* session = g_meshRegistry.Get(handle);
    if (!session || session->payload.edgeVertices.empty()) {
        return nullptr;
    }
    jlong capacity = static_cast<jlong>(session->payload.edgeVertices.size() * sizeof(androcad::EdgeVertex));
    return env->NewDirectByteBuffer(session->payload.edgeVertices.data(), capacity);
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetBoundingBox(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jfloatArray outBounds) {
    auto* session = g_meshRegistry.Get(handle);
    if (!session || !outBounds) return;

    jfloat buf[6];
    buf[0] = session->payload.bounds.minX;
    buf[1] = session->payload.bounds.minY;
    buf[2] = session->payload.bounds.minZ;
    buf[3] = session->payload.bounds.maxX;
    buf[4] = session->payload.bounds.maxY;
    buf[5] = session->payload.bounds.maxZ;

    env->SetFloatArrayRegion(outBounds, 0, 6, buf);
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativePickByRay(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jfloat ox, jfloat oy, jfloat oz,
        jfloat dx, jfloat dy, jfloat dz,
        jfloat edgeTol,
        jfloatArray outResult) {
    auto* session = g_meshRegistry.Get(handle);
    if (!session) return JNI_FALSE;

    androcad::Ray ray{ox, oy, oz, dx, dy, dz};
    androcad::PickResult pick = androcad::AndroCadKernel::PickEntityByRay(session->payload, ray, edgeTol);

    if (outResult) {
        jfloat buf[7];
        buf[0] = pick.hit ? 1.0f : 0.0f;
        buf[1] = static_cast<jfloat>(pick.entityId);
        buf[2] = pick.isEdge ? 1.0f : 0.0f;
        buf[3] = pick.distance;
        buf[4] = pick.hitPoint[0];
        buf[5] = pick.hitPoint[1];
        buf[6] = pick.hitPoint[2];
        env->SetFloatArrayRegion(outResult, 0, 7, buf);
    }

    return pick.hit ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeReleaseMesh(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    g_meshRegistry.Release(handle);
}

// -------------------------------------------------------------------------
// Milestone 3: 2D Sketcher & PlaneGCS Constraint Solver JNI Bindings
// -------------------------------------------------------------------------

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCreateSketch(
        JNIEnv* /* env */,
        jobject /* this */,
        jdouble ox, jdouble oy, jdouble oz,
        jdouble ux, jdouble uy, jdouble uz,
        jdouble vx, jdouble vy, jdouble vz,
        jdouble nx, jdouble ny, jdouble nz) {
    
    androcad::SketchPlane plane{ox, oy, oz, ux, uy, uz, vx, vy, vz, nx, ny, nz};
    auto session = std::make_unique<NativeSketchSession>();
    session->sketcher = std::make_unique<androcad::Sketcher>(plane);

    return g_sketchRegistry.Register(std::move(session));
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchAddPoint(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jdouble u, jdouble v,
        jboolean isFixed) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher) return -1;
    return session->sketcher->AddPoint(u, v, isFixed == JNI_TRUE);
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchAddLine(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jint pStart, jint pEnd) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher) return -1;
    return session->sketcher->AddLine(pStart, pEnd);
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchAddCircle(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jint pCenter, jdouble radius) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher) return -1;
    return session->sketcher->AddCircle(pCenter, radius);
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchAddRectangle(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jdouble u, jdouble v,
        jdouble width, jdouble height) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher) return -1;
    return session->sketcher->AddRectangle(u, v, width, height);
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchAddConstraint(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jint constraintType,
        jint p1, jint p2, jint p3, jint p4,
        jdouble value) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher) return -1;

    switch (constraintType) {
        case 0: return session->sketcher->AddCoincident(p1, p2);
        case 1: return session->sketcher->AddHorizontal(p1, p2);
        case 2: return session->sketcher->AddVertical(p1, p2);
        case 3: return session->sketcher->AddDistance(p1, p2, value);
        case 4: return session->sketcher->AddParallel(p1, p2, p3, p4);
        case 5: return session->sketcher->AddPerpendicular(p1, p2, p3, p4);
        case 6: return session->sketcher->AddEqualLength(p1, p2, p3, p4);
        default: return -1;
    }
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchSolve(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher) return -1;
    androcad::gcs::SolveStatus status = session->sketcher->Solve();
    return static_cast<jint>(status);
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchGetDoF(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher) return 0;
    return session->sketcher->GetDegreesOfFreedom();
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchGetPoint(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jint pointId,
        jdoubleArray outCoords) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher || !outCoords) return;

    auto pt = session->sketcher->GetPoint(pointId);
    jdouble coords[2] = {pt.x, pt.y};
    env->SetDoubleArrayRegion(outCoords, 0, 2, coords);
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchBuildProfileFace(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jdoubleArray outResult) {
    auto* session = g_sketchRegistry.Get(handle);
    if (!session || !session->sketcher) return JNI_FALSE;

    session->lastResult = session->sketcher->BuildProfileFace();

    if (outResult) {
        jdouble res[4];
        res[0] = session->lastResult.isClosedProfile ? 1.0 : 0.0;
        res[1] = session->lastResult.profileArea;
        res[2] = static_cast<jdouble>(session->lastResult.degreesOfFreedom);
        res[3] = static_cast<jdouble>(session->lastResult.solveStatus);
        env->SetDoubleArrayRegion(outResult, 0, 4, res);
    }

    return session->lastResult.success ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeReleaseSketch(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    g_sketchRegistry.Release(handle);
}

// -------------------------------------------------------------------------
// Milestone 4: 3D Parametric Features JNI Bindings
// -------------------------------------------------------------------------

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeExtrude(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong sketchHandle,
        jdouble dx, jdouble dy, jdouble dz,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* sketchSession = g_sketchRegistry.Get(sketchHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!sketchSession || !sketchSession->sketcher) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid sketch handle.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    if (sketchSession->lastResult.face.IsNull()) {
        sketchSession->lastResult = sketchSession->sketcher->BuildProfileFace();
    }
    if (sketchSession->lastResult.face.IsNull() || !sketchSession->lastResult.success) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Sketch does not contain a valid closed profile face.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    meshSession->payload = androcad::FeatureOps::MakeExtrude(
        sketchSession->lastResult.face,
        dx, dy, dz,
        meshSession->shape,
        linearDeflection,
        angularDeflection
    );
    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeRevolve(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong sketchHandle,
        jdouble ax, jdouble ay, jdouble az,
        jdouble dx, jdouble dy, jdouble dz,
        jdouble angleRad,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* sketchSession = g_sketchRegistry.Get(sketchHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!sketchSession || !sketchSession->sketcher) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid sketch handle.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    if (sketchSession->lastResult.face.IsNull()) {
        sketchSession->lastResult = sketchSession->sketcher->BuildProfileFace();
    }
    if (sketchSession->lastResult.face.IsNull() || !sketchSession->lastResult.success) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Sketch does not contain a valid closed profile face.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    meshSession->payload = androcad::FeatureOps::MakeRevolve(
        sketchSession->lastResult.face,
        ax, ay, az,
        dx, dy, dz,
        angleRad,
        meshSession->shape,
        linearDeflection,
        angularDeflection
    );
    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeFillet(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jdouble radius,
        jint targetEdgeId,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* solidSession = g_meshRegistry.Get(solidHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidSession || solidSession->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handle or null shape.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    try {
        meshSession->payload = androcad::FeatureOps::MakeFillet(
            solidSession->shape,
            radius,
            targetEdgeId,
            meshSession->shape,
            linearDeflection,
            angularDeflection
        );
    } catch (const Standard_Failure& e) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = std::string("Fillet Standard_Failure: ") + e.GetMessageString();
    } catch (const std::exception& e) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = std::string("Fillet std::exception: ") + e.what();
    } catch (...) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Fillet unknown native error.";
    }
    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeChamfer(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jdouble distance,
        jint targetEdgeId,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* solidSession = g_meshRegistry.Get(solidHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidSession || solidSession->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handle or null shape.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    try {
        meshSession->payload = androcad::FeatureOps::MakeChamfer(
            solidSession->shape,
            distance,
            targetEdgeId,
            meshSession->shape,
            linearDeflection,
            angularDeflection
        );
    } catch (const Standard_Failure& e) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = std::string("Chamfer Standard_Failure: ") + e.GetMessageString();
    } catch (const std::exception& e) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = std::string("Chamfer std::exception: ") + e.what();
    } catch (...) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Chamfer unknown native error.";
    }
    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeHole(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jdouble px, jdouble py, jdouble pz,
        jdouble dx, jdouble dy, jdouble dz,
        jdouble radius,
        jdouble depth,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* solidSession = g_meshRegistry.Get(solidHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidSession || solidSession->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handle or null shape.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    try {
        meshSession->payload = androcad::FeatureOps::MakeHole(
            solidSession->shape,
            px, py, pz,
            dx, dy, dz,
            radius,
            depth,
            meshSession->shape,
            linearDeflection,
            angularDeflection
        );
    } catch (const Standard_Failure& e) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = std::string("Hole Standard_Failure: ") + e.GetMessageString();
    } catch (const std::exception& e) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = std::string("Hole std::exception: ") + e.what();
    } catch (...) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Hole unknown native error.";
    }
    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeShell(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jdouble thickness,
        jint removeFaceId,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* solidSession = g_meshRegistry.Get(solidHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidSession || solidSession->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handle or null shape.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    meshSession->payload = androcad::FeatureOps::MakeShell(
        solidSession->shape,
        thickness,
        removeFaceId,
        meshSession->shape,
        linearDeflection,
        angularDeflection
    );
    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeBoolean(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidAHandle,
        jlong solidBHandle,
        jint opType,
        jdouble fuzzyValue,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* solidA = g_meshRegistry.Get(solidAHandle);
    auto* solidB = g_meshRegistry.Get(solidBHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidA || solidA->shape.IsNull() || !solidB || solidB->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handles for Boolean operation.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    meshSession->payload = androcad::FeatureOps::MakeBoolean(
        solidA->shape,
        solidB->shape,
        static_cast<androcad::BooleanType>(opType),
        fuzzyValue,
        meshSession->shape,
        linearDeflection,
        angularDeflection
    );
    return g_meshRegistry.Register(std::move(meshSession));
}

// -------------------------------------------------------------------------
// Milestone 5: Expression Parser & Parametric Timeline DAG JNI Bindings
// -------------------------------------------------------------------------

JNIEXPORT jdoubleArray JNICALL
Java_com_androcad_kernel_NativeKernel_nativeEvaluateExpression(
        JNIEnv* env,
        jobject /* this */,
        jstring exprStr,
        jobjectArray varNames,
        jdoubleArray varValues) {
    if (!exprStr) return nullptr;

    const char* cExpr = env->GetStringUTFChars(exprStr, nullptr);
    std::string expr(cExpr);
    env->ReleaseStringUTFChars(exprStr, cExpr);

    androcad::ExpressionParser parser;
    if (varNames && varValues) {
        jsize count = env->GetArrayLength(varNames);
        jdouble* vals = env->GetDoubleArrayElements(varValues, nullptr);
        for (jsize i = 0; i < count; ++i) {
            jstring nStr = static_cast<jstring>(env->GetObjectArrayElement(varNames, i));
            const char* cName = env->GetStringUTFChars(nStr, nullptr);
            parser.SetVariable(cName, vals[i]);
            env->ReleaseStringUTFChars(nStr, cName);
            env->DeleteLocalRef(nStr);
        }
        env->ReleaseDoubleArrayElements(varValues, vals, JNI_ABORT);
    }

    auto res = parser.Evaluate(expr);
    jdoubleArray out = env->NewDoubleArray(2);
    jdouble buf[2] = {res.success ? 1.0 : 0.0, res.value};
    env->SetDoubleArrayRegion(out, 0, 2, buf);
    return out;
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCreateTimeline(
        JNIEnv* /* env */,
        jobject /* this */) {
    auto session = std::make_unique<NativeTimelineSession>();
    session->timeline = std::make_unique<androcad::TimelineDAG>();
    return g_timelineRegistry.Register(std::move(session));
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeTimelineSetParameter(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring nameStr,
        jstring exprStr) {
    auto* session = g_timelineRegistry.Get(handle);
    if (!session || !session->timeline || !nameStr || !exprStr) return JNI_FALSE;

    const char* cName = env->GetStringUTFChars(nameStr, nullptr);
    const char* cExpr = env->GetStringUTFChars(exprStr, nullptr);
    session->timeline->SetParameter(cName, cExpr);
    env->ReleaseStringUTFChars(nameStr, cName);
    env->ReleaseStringUTFChars(exprStr, cExpr);
    return JNI_TRUE;
}

JNIEXPORT jdouble JNICALL
Java_com_androcad_kernel_NativeKernel_nativeTimelineGetParameter(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring nameStr) {
    auto* session = g_timelineRegistry.Get(handle);
    if (!session || !session->timeline || !nameStr) return 0.0;

    const char* cName = env->GetStringUTFChars(nameStr, nullptr);
    double val = session->timeline->GetParameterValue(cName);
    env->ReleaseStringUTFChars(nameStr, cName);
    return val;
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeTimelineAddBox(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring nameStr,
        jstring dxStr,
        jstring dyStr,
        jstring dzStr) {
    auto* session = g_timelineRegistry.Get(handle);
    if (!session || !session->timeline) return -1;

    const char* cName = env->GetStringUTFChars(nameStr, nullptr);
    const char* cDx = env->GetStringUTFChars(dxStr, nullptr);
    const char* cDy = env->GetStringUTFChars(dyStr, nullptr);
    const char* cDz = env->GetStringUTFChars(dzStr, nullptr);

    uint32_t id = session->timeline->AddBoxNode(cName, cDx, cDy, cDz);

    env->ReleaseStringUTFChars(nameStr, cName);
    env->ReleaseStringUTFChars(dxStr, cDx);
    env->ReleaseStringUTFChars(dyStr, cDy);
    env->ReleaseStringUTFChars(dzStr, cDz);
    return static_cast<jint>(id);
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeTimelineSetRollback(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jint rollbackIndex) {
    auto* session = g_timelineRegistry.Get(handle);
    if (!session || !session->timeline) return;
    session->timeline->SetRollbackIndex(rollbackIndex);
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeTimelineRebuild(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = g_timelineRegistry.Get(handle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!session || !session->timeline) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid timeline handle.";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    bool success = session->timeline->Rebuild();
    if (success) {
        meshSession->shape = session->timeline->GetActiveShape();
        meshSession->payload = session->timeline->GetActiveMesh();
    } else {
        meshSession->payload.success = false;
        const auto* active = session->timeline->GetActiveNode();
        meshSession->payload.errorMessage = active ? active->errorMessage : "Timeline rebuild failed.";
    }

    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeReleaseTimeline(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    g_timelineRegistry.Release(handle);
}

// -------------------------------------------------------------------------
// Milestone 6: Versioned Project I/O & Data Exchange (STEP / STL / .androcad)
// -------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeExportStep(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring filePathStr,
        jstring schemaStr,
        jobjectArray outError) {
    auto* session = g_meshRegistry.Get(handle);
    if (!session || session->shape.IsNull()) {
        if (outError && env->GetArrayLength(outError) > 0) {
            env->SetObjectArrayElement(outError, 0, env->NewStringUTF("Invalid solid handle or null shape."));
        }
        return JNI_FALSE;
    }

    const char* cPath = env->GetStringUTFChars(filePathStr, nullptr);
    const char* cSchema = schemaStr ? env->GetStringUTFChars(schemaStr, nullptr) : "AP214";

    auto res = androcad::ProjectIO::ExportSTEP(session->shape, cPath, cSchema ? cSchema : "AP214");

    if (schemaStr && cSchema) env->ReleaseStringUTFChars(schemaStr, cSchema);
    env->ReleaseStringUTFChars(filePathStr, cPath);

    if (!res.success && outError && env->GetArrayLength(outError) > 0) {
        env->SetObjectArrayElement(outError, 0, env->NewStringUTF(res.errorMessage.c_str()));
    }

    return res.success ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeImportStep(
        JNIEnv* env,
        jobject /* this */,
        jstring filePathStr,
        jobjectArray outError) {
    auto meshSession = std::make_unique<NativeMeshSession>();

    const char* cPath = env->GetStringUTFChars(filePathStr, nullptr);
    auto res = androcad::ProjectIO::ImportSTEP(cPath, meshSession->shape);
    env->ReleaseStringUTFChars(filePathStr, cPath);

    if (res.success && !meshSession->shape.IsNull()) {
        meshSession->payload = androcad::FeatureOps::ShapeToMeshPayload(meshSession->shape);
        meshSession->payload.volume = res.volume;
    } else {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = res.errorMessage;
        if (outError && env->GetArrayLength(outError) > 0) {
            env->SetObjectArrayElement(outError, 0, env->NewStringUTF(res.errorMessage.c_str()));
        }
    }

    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeExportStl(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring filePathStr,
        jdouble linearDeflection,
        jboolean binary,
        jobjectArray outError) {
    auto* session = g_meshRegistry.Get(handle);
    if (!session || session->shape.IsNull()) {
        if (outError && env->GetArrayLength(outError) > 0) {
            env->SetObjectArrayElement(outError, 0, env->NewStringUTF("Invalid solid handle or null shape."));
        }
        return JNI_FALSE;
    }

    const char* cPath = env->GetStringUTFChars(filePathStr, nullptr);
    auto res = androcad::ProjectIO::ExportSTL(session->shape, cPath, linearDeflection, 0.5, binary == JNI_TRUE);
    env->ReleaseStringUTFChars(filePathStr, cPath);

    if (!res.success && outError && env->GetArrayLength(outError) > 0) {
        env->SetObjectArrayElement(outError, 0, env->NewStringUTF(res.errorMessage.c_str()));
    }

    return res.success ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeImportStl(
        JNIEnv* env,
        jobject /* this */,
        jstring filePathStr,
        jobjectArray outError) {
    auto meshSession = std::make_unique<NativeMeshSession>();

    const char* cPath = env->GetStringUTFChars(filePathStr, nullptr);
    auto res = androcad::ProjectIO::ImportSTL(cPath, meshSession->shape);
    env->ReleaseStringUTFChars(filePathStr, cPath);

    if (res.success && !meshSession->shape.IsNull()) {
        meshSession->payload = androcad::FeatureOps::ShapeToMeshPayload(meshSession->shape);
        meshSession->payload.volume = res.volume;
    } else {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = res.errorMessage;
        if (outError && env->GetArrayLength(outError) > 0) {
            env->SetObjectArrayElement(outError, 0, env->NewStringUTF(res.errorMessage.c_str()));
        }
    }

    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSaveProject(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring filePathStr,
        jobjectArray outError) {
    auto* session = g_timelineRegistry.Get(handle);
    if (!session || !session->timeline) {
        if (outError && env->GetArrayLength(outError) > 0) {
            env->SetObjectArrayElement(outError, 0, env->NewStringUTF("Invalid timeline handle."));
        }
        return JNI_FALSE;
    }

    const char* cPath = env->GetStringUTFChars(filePathStr, nullptr);
    auto res = androcad::ProjectIO::SaveProject(*session->timeline, cPath);
    env->ReleaseStringUTFChars(filePathStr, cPath);

    if (!res.success && outError && env->GetArrayLength(outError) > 0) {
        env->SetObjectArrayElement(outError, 0, env->NewStringUTF(res.errorMessage.c_str()));
    }

    return res.success ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeLoadProject(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring filePathStr,
        jobjectArray outError) {
    auto* session = g_timelineRegistry.Get(handle);
    if (!session || !session->timeline) {
        if (outError && env->GetArrayLength(outError) > 0) {
            env->SetObjectArrayElement(outError, 0, env->NewStringUTF("Invalid timeline handle."));
        }
        return JNI_FALSE;
    }

    const char* cPath = env->GetStringUTFChars(filePathStr, nullptr);
    auto res = androcad::ProjectIO::LoadProject(*session->timeline, cPath);
    env->ReleaseStringUTFChars(filePathStr, cPath);

    if (!res.success && outError && env->GetArrayLength(outError) > 0) {
        env->SetObjectArrayElement(outError, 0, env->NewStringUTF(res.errorMessage.c_str()));
    }

    return res.success ? JNI_TRUE : JNI_FALSE;
}

// Milestone 8: Surface & Direct Modeling
JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCreateLoftRectToCircle(
        JNIEnv* /* env */,
        jobject /* this */,
        jdouble zHeight,
        jdouble radius,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto session = std::make_unique<NativeMeshSession>();
    try {
        TopoDS_Wire wireBottom = androcad::SurfaceOps::CreateRectangleWire(0.0, 0.0, 0.0, 100.0, 50.0);
        TopoDS_Wire wireTop = androcad::SurfaceOps::CreateCircleWire(50.0, 25.0, zHeight, radius);
        std::vector<TopoDS_Wire> sections = {wireBottom, wireTop};
        session->payload = androcad::SurfaceOps::MakeLoft(sections, true, false, session->shape, linearDeflection, angularDeflection);
    } catch (const std::exception& e) {
        session->payload.success = false;
        session->payload.errorMessage = e.what();
    } catch (...) {
        session->payload.success = false;
        session->payload.errorMessage = "nativeCreateBox failed with unknown native error";
    }
    return g_meshRegistry.Register(std::move(session));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeThicken(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jdouble thickness,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* inSession = g_meshRegistry.Get(solidHandle);
    auto outSession = std::make_unique<NativeMeshSession>();
    if (!inSession || inSession->shape.IsNull()) {
        outSession->payload.success = false;
        outSession->payload.errorMessage = "Invalid input shape for Thicken";
        return g_meshRegistry.Register(std::move(outSession));
    }
    try {
        outSession->payload = androcad::SurfaceOps::MakeThicken(inSession->shape, thickness, outSession->shape, linearDeflection, angularDeflection);
    } catch (const std::exception& e) {
        outSession->payload.success = false;
        outSession->payload.errorMessage = e.what();
    }
    return g_meshRegistry.Register(std::move(outSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativePressPullFace(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jint faceId,
        jdouble distance,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* inSession = g_meshRegistry.Get(solidHandle);
    auto outSession = std::make_unique<NativeMeshSession>();
    if (!inSession || inSession->shape.IsNull()) {
        outSession->payload.success = false;
        outSession->payload.errorMessage = "Invalid input shape for Press-Pull";
        return g_meshRegistry.Register(std::move(outSession));
    }
    try {
        outSession->payload = androcad::SurfaceOps::PressPullFace(inSession->shape, faceId, distance, outSession->shape, linearDeflection, angularDeflection);
    } catch (const std::exception& e) {
        outSession->payload.success = false;
        outSession->payload.errorMessage = e.what();
    }
    return g_meshRegistry.Register(std::move(outSession));
}

// Milestone 9: Assemblies & Kinematic Joints
JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCreateAssembly(
        JNIEnv* /* env */,
        jobject /* this */) {
    auto session = std::make_unique<NativeAssemblySession>();
    session->assembly = std::make_unique<androcad::AssemblyManager>();
    return g_assemblyRegistry.Register(std::move(session));
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeAssemblyAddComponent(
        JNIEnv* env,
        jobject /* this */,
        jlong asmHandle,
        jstring nameStr,
        jlong meshHandle) {
    auto* asmSession = g_assemblyRegistry.Get(asmHandle);
    auto* meshSession = g_meshRegistry.Get(meshHandle);
    if (!asmSession || !asmSession->assembly || !meshSession || meshSession->shape.IsNull()) {
        return -1;
    }
    const char* cName = env->GetStringUTFChars(nameStr, nullptr);
    std::string name = cName ? cName : "Component";
    env->ReleaseStringUTFChars(nameStr, cName);

    try {
        return asmSession->assembly->AddComponent(name, meshSession->shape);
    } catch (...) {
        return -1;
    }
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeAssemblyAddJoint(
        JNIEnv* env,
        jobject /* this */,
        jlong asmHandle,
        jstring nameStr,
        jint jointType,
        jint parentCompId,
        jint childCompId,
        jdouble px, jdouble py, jdouble pz,
        jdouble ax, jdouble ay, jdouble az,
        jdouble minVal, jdouble maxVal) {
    auto* asmSession = g_assemblyRegistry.Get(asmHandle);
    if (!asmSession || !asmSession->assembly) return -1;

    const char* cName = env->GetStringUTFChars(nameStr, nullptr);
    std::string name = cName ? cName : "Joint";
    env->ReleaseStringUTFChars(nameStr, cName);

    try {
        gp_Pnt pivot(px, py, pz);
        gp_Dir axis(ax, ay, az);
        auto type = static_cast<androcad::JointType>(jointType);
        return asmSession->assembly->AddJoint(name, type, parentCompId, childCompId, pivot, axis, minVal, maxVal);
    } catch (...) {
        return -1;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeAssemblySetJointValue(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong asmHandle,
        jint jointId,
        jdouble value) {
    auto* asmSession = g_assemblyRegistry.Get(asmHandle);
    if (!asmSession || !asmSession->assembly) return JNI_FALSE;
    return asmSession->assembly->SetJointValue(jointId, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeAssemblyStepAnimation(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong asmHandle,
        jdouble phase) {
    auto* asmSession = g_assemblyRegistry.Get(asmHandle);
    if (asmSession && asmSession->assembly) {
        asmSession->assembly->StepAnimation(phase);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeAssemblyCheckInterference(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong asmHandle,
        jint compA,
        jint compB) {
    auto* asmSession = g_assemblyRegistry.Get(asmHandle);
    if (!asmSession || !asmSession->assembly) return JNI_FALSE;
    return asmSession->assembly->CheckInterference(compA, compB) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeAssemblyBuildMesh(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong asmHandle,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* asmSession = g_assemblyRegistry.Get(asmHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!asmSession || !asmSession->assembly) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid assembly handle";
        return g_meshRegistry.Register(std::move(meshSession));
    }

    try {
        meshSession->payload = asmSession->assembly->BuildAssemblyMeshPayload(linearDeflection, angularDeflection);
    } catch (const std::exception& e) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = e.what();
    }
    return g_meshRegistry.Register(std::move(meshSession));
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeReleaseAssembly(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong asmHandle) {
    g_assemblyRegistry.Release(asmHandle);
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSheetMetalCreateUBracket(
        JNIEnv* /* env */,
        jobject /* this */,
        jdouble baseLength,
        jdouble flangeHeight,
        jdouble width,
        jdouble thickness,
        jdouble bendRadius,
        jdouble kFactor,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto session = std::make_unique<NativeMeshSession>();
    try {
        androcad::SheetMetalRule rule;
        rule.thickness = thickness;
        rule.bendRadius = bendRadius;
        rule.kFactor = kFactor;

        session->shape = androcad::SheetMetalEngine::CreateUBracket(baseLength, flangeHeight, width, rule);
        session->payload = androcad::SheetMetalEngine::ToMeshPayload(session->shape, linearDeflection, angularDeflection);
    } catch (const std::exception& e) {
        session->payload.success = false;
        session->payload.errorMessage = e.what();
    } catch (...) {
        session->payload.success = false;
        session->payload.errorMessage = "nativeCreateBox failed with unknown native error";
    }
    return g_meshRegistry.Register(std::move(session));
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSheetMetalUnfoldUBracket(
        JNIEnv* env,
        jobject /* this */,
        jdouble baseLength,
        jdouble flangeHeight,
        jdouble width,
        jdouble thickness,
        jdouble bendRadius,
        jdouble kFactor) {
    androcad::SheetMetalRule rule;
    rule.thickness = thickness;
    rule.bendRadius = bendRadius;
    rule.kFactor = kFactor;

    androcad::FlatPattern pattern = androcad::SheetMetalEngine::UnfoldUBracket(baseLength, flangeHeight, width, rule);

    std::ostringstream ss;
    ss << "{\"success\":" << (pattern.success ? "true" : "false")
       << ",\"flatLength\":" << pattern.flatLength
       << ",\"flatWidth\":" << pattern.flatWidth
       << ",\"flatArea\":" << pattern.flatArea
       << ",\"bends\":[";
    for (size_t i = 0; i < pattern.bends.size(); ++i) {
        const auto& b = pattern.bends[i];
        if (i > 0) ss << ",";
        ss << "{\"id\":" << b.id
           << ",\"startX\":" << b.startX
           << ",\"startY\":" << b.startY
           << ",\"endX\":" << b.endX
           << ",\"endY\":" << b.endY
           << ",\"angleDeg\":" << b.angleDeg
           << ",\"radius\":" << b.radius
           << ",\"isBendUp\":" << (b.isBendUp ? "true" : "false")
           << "}";
    }
    ss << "],\"errorMessage\":\"" << pattern.errorMessage << "\"}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSheetMetalExportDXF(
        JNIEnv* env,
        jobject /* this */,
        jdouble baseLength,
        jdouble flangeHeight,
        jdouble width,
        jdouble thickness,
        jdouble bendRadius,
        jdouble kFactor) {
    androcad::SheetMetalRule rule;
    rule.thickness = thickness;
    rule.bendRadius = bendRadius;
    rule.kFactor = kFactor;

    androcad::FlatPattern pattern = androcad::SheetMetalEngine::UnfoldUBracket(baseLength, flangeHeight, width, rule);
    return env->NewStringUTF(pattern.dxfContent.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeExportDrawingSVG(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jstring partNameStr,
        jdouble scale) {
    auto* session = g_meshRegistry.Get(solidHandle);
    if (!session || session->shape.IsNull()) {
        return env->NewStringUTF("<svg></svg>");
    }

    const char* partNameChars = env->GetStringUTFChars(partNameStr, nullptr);
    std::string partName = partNameChars ? partNameChars : "Part1";
    if (partNameChars) env->ReleaseStringUTFChars(partNameStr, partNameChars);

    androcad::DrawingSheet sheet = androcad::DrawingEngine::GenerateStandardDrawingSheet(session->shape, partName, scale);
    std::string svg = sheet.ToSVG();
    return env->NewStringUTF(svg.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeExportDrawingDXF(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jstring partNameStr,
        jdouble scale) {
    auto* session = g_meshRegistry.Get(solidHandle);
    if (!session || session->shape.IsNull()) {
        return env->NewStringUTF("");
    }

    const char* partNameChars = env->GetStringUTFChars(partNameStr, nullptr);
    std::string partName = partNameChars ? partNameChars : "Part1";
    if (partNameChars) env->ReleaseStringUTFChars(partNameStr, partNameChars);

    androcad::DrawingSheet sheet = androcad::DrawingEngine::GenerateStandardDrawingSheet(session->shape, partName, scale);
    std::string dxf = sheet.ToDXF();
    return env->NewStringUTF(dxf.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeComputePhysicalProperties(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jint materialId) {
    auto* session = g_meshRegistry.Get(solidHandle);
    if (!session || session->shape.IsNull()) {
        return env->NewStringUTF("{\"success\":false,\"errorMessage\":\"Invalid solid handle\"}");
    }

    androcad::Material mat = androcad::MaterialEngine::GetMaterial(materialId);
    androcad::PhysicalProperties props = androcad::MaterialEngine::ComputePhysicalProperties(session->shape, mat);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (props.success ? "true" : "false")
       << ",\"materialName\":\"" << props.materialName << "\""
       << ",\"density\":" << props.densityGcm3
       << ",\"volume\":" << props.volumeMm3
       << ",\"surfaceArea\":" << props.surfaceAreaMm2
       << ",\"massGrams\":" << props.massGrams
       << ",\"massKg\":" << props.massKg
       << ",\"cogX\":" << props.cogX
       << ",\"cogY\":" << props.cogY
       << ",\"cogZ\":" << props.cogZ
       << ",\"ixx\":" << props.ixx
       << ",\"iyy\":" << props.iyy
       << ",\"izz\":" << props.izz
       << ",\"hexColor\":\"" << mat.appearance.hexColor << "\""
       << ",\"metallic\":" << mat.appearance.metallic
       << ",\"roughness\":" << mat.appearance.roughness
       << ",\"errorMessage\":\"" << props.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetMaterialList(
        JNIEnv* env,
        jobject /* this */) {
    auto list = androcad::MaterialEngine::GetDefaultMaterials();
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& m = list[i];
        if (i > 0) ss << ",";
        ss << "{\"id\":" << m.id
           << ",\"name\":\"" << m.name << "\""
           << ",\"category\":\"" << m.category << "\""
           << ",\"density\":" << m.density
           << ",\"youngModulus\":" << m.youngModulusGPa
           << ",\"hexColor\":\"" << m.appearance.hexColor << "\""
           << ",\"metallic\":" << m.appearance.metallic
           << ",\"roughness\":" << m.appearance.roughness
           << "}";
    }
    ss << "]";
    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeMeshSmooth(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jint iterations,
        jdouble lambda) {
    auto* session = g_meshRegistry.Get(solidHandle);
    auto newSession = std::make_unique<NativeMeshSession>();
    if (!session || session->shape.IsNull()) {
        newSession->payload.success = false;
        newSession->payload.errorMessage = "Invalid input mesh handle";
        return g_meshRegistry.Register(std::move(newSession));
    }

    try {
        androcad::RawMeshData raw = androcad::MeshEngine::ExtractFromShape(session->shape);
        androcad::RawMeshData smoothed = androcad::MeshEngine::SmoothMesh(raw, iterations, static_cast<float>(lambda));
        newSession->payload = androcad::MeshEngine::ToMeshPayload(smoothed);
        newSession->shape = androcad::MeshEngine::ConvertMeshToSolid(smoothed);
    } catch (const std::exception& e) {
        newSession->payload.success = false;
        newSession->payload.errorMessage = e.what();
    }

    return g_meshRegistry.Register(std::move(newSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeMeshRepair(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jdouble weldTolerance) {
    auto* session = g_meshRegistry.Get(solidHandle);
    auto newSession = std::make_unique<NativeMeshSession>();
    if (!session || session->shape.IsNull()) {
        newSession->payload.success = false;
        newSession->payload.errorMessage = "Invalid input mesh handle";
        return g_meshRegistry.Register(std::move(newSession));
    }

    try {
        androcad::RawMeshData raw = androcad::MeshEngine::ExtractFromShape(session->shape);
        androcad::RawMeshData repaired = androcad::MeshEngine::RepairMesh(raw, weldTolerance);
        newSession->payload = androcad::MeshEngine::ToMeshPayload(repaired);
        newSession->shape = androcad::MeshEngine::ConvertMeshToSolid(repaired);
    } catch (const std::exception& e) {
        newSession->payload.success = false;
        newSession->payload.errorMessage = e.what();
    }

    return g_meshRegistry.Register(std::move(newSession));
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeMeshToSolid(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jdouble tolerance) {
    auto* session = g_meshRegistry.Get(solidHandle);
    auto newSession = std::make_unique<NativeMeshSession>();
    if (!session || session->shape.IsNull()) {
        newSession->payload.success = false;
        newSession->payload.errorMessage = "Invalid input mesh handle";
        return g_meshRegistry.Register(std::move(newSession));
    }

    try {
        androcad::RawMeshData raw = androcad::MeshEngine::ExtractFromShape(session->shape);
        newSession->shape = androcad::MeshEngine::ConvertMeshToSolid(raw, tolerance);
        newSession->payload = androcad::FeatureOps::ShapeToMeshPayload(newSession->shape);
    } catch (const std::exception& e) {
        newSession->payload.success = false;
        newSession->payload.errorMessage = e.what();
    }

    return g_meshRegistry.Register(std::move(newSession));
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeMeshAnalyze(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle) {
    auto* session = g_meshRegistry.Get(solidHandle);
    if (!session || session->shape.IsNull()) {
        return env->NewStringUTF("{\"success\":false,\"errorMessage\":\"Invalid solid handle\"}");
    }

    androcad::RawMeshData raw = androcad::MeshEngine::ExtractFromShape(session->shape);
    androcad::MeshStats stats = androcad::MeshEngine::AnalyzeMesh(raw);

    std::ostringstream ss;
    ss << "{\"success\":true"
       << ",\"vertexCount\":" << stats.vertexCount
       << ",\"triangleCount\":" << stats.triangleCount
       << ",\"edgeCount\":" << stats.edgeCount
       << ",\"surfaceArea\":" << stats.surfaceArea
       << ",\"enclosedVolume\":" << stats.enclosedVolume
       << ",\"isClosedManifold\":" << (stats.isClosedManifold ? "true" : "false")
       << ",\"eulerCharacteristic\":" << stats.eulerCharacteristic
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSimulateCantileverBeam(
        JNIEnv* env,
        jobject /* this */,
        jdouble lengthMm,
        jdouble widthMm,
        jdouble heightMm,
        jdouble forceN,
        jint materialId,
        jdouble deformationScale) {
    androcad::Material mat = androcad::MaterialEngine::GetMaterial(materialId);
    androcad::SimulationResult res = androcad::SimulationEngine::SolveCantileverBeam(
        lengthMm, widthMm, heightMm, forceN, mat, deformationScale);

    auto session = std::make_unique<NativeMeshSession>();
    session->payload.success = res.success;
    session->payload.errorMessage = res.errorMessage;

    if (res.success) {
        size_t vertCount = res.visualMesh.positions.size() / 3;
        session->payload.vertices.resize(vertCount);
        for (size_t i = 0; i < vertCount; ++i) {
            session->payload.vertices[i].x = res.visualMesh.positions[i * 3 + 0];
            session->payload.vertices[i].y = res.visualMesh.positions[i * 3 + 1];
            session->payload.vertices[i].z = res.visualMesh.positions[i * 3 + 2];
            session->payload.vertices[i].nx = res.visualMesh.colors[i * 3 + 0];
            session->payload.vertices[i].ny = res.visualMesh.colors[i * 3 + 1];
            session->payload.vertices[i].nz = res.visualMesh.colors[i * 3 + 2];
            session->payload.vertices[i].faceId = static_cast<uint32_t>(res.visualMesh.stressesMPa[i]);
        }
        session->payload.indices = res.visualMesh.indices;
    }

    jlong handle = g_meshRegistry.Register(std::move(session));

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"nodeCount\":" << res.nodeCount
       << ",\"elementCount\":" << res.elementCount
       << ",\"maxVonMisesStressMPa\":" << res.maxVonMisesStressMPa
       << ",\"minVonMisesStressMPa\":" << res.minVonMisesStressMPa
       << ",\"maxDisplacementMm\":" << res.maxDisplacementMm
       << ",\"minSafetyFactor\":" << res.minSafetyFactor
       << ",\"meshHandle\":" << handle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSimulateSolid(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jdouble targetElementSizeMm,
        jdouble forceN,
        jint materialId,
        jdouble deformationScale) {
    auto* inSession = g_meshRegistry.Get(solidHandle);
    if (!inSession || inSession->shape.IsNull()) {
        return env->NewStringUTF("{\"success\":false,\"errorMessage\":\"Invalid solid handle\"}");
    }

    androcad::Material mat = androcad::MaterialEngine::GetMaterial(materialId);
    androcad::SimulationEngine sim;
    sim.SetMaterial(mat);
    sim.DiscretizeShape(inSession->shape, targetElementSizeMm);

    androcad::BoundaryConstraint bc;
    bc.type = androcad::ConstraintType::FIXED;
    bc.minX = -1e9;
    bc.maxX = 5.0;
    sim.AddConstraint(bc);

    androcad::StructuralLoad load;
    load.type = androcad::LoadType::TOTAL_FORCE;
    load.forceZ = -forceN;
    load.minX = 20.0;
    load.maxX = 1e9;
    sim.AddLoad(load);

    androcad::SimulationResult res = sim.Solve(deformationScale);

    auto outSession = std::make_unique<NativeMeshSession>();
    outSession->shape = inSession->shape;
    outSession->payload.success = res.success;
    outSession->payload.errorMessage = res.errorMessage;

    if (res.success) {
        size_t vertCount = res.visualMesh.positions.size() / 3;
        outSession->payload.vertices.resize(vertCount);
        for (size_t i = 0; i < vertCount; ++i) {
            outSession->payload.vertices[i].x = res.visualMesh.positions[i * 3 + 0];
            outSession->payload.vertices[i].y = res.visualMesh.positions[i * 3 + 1];
            outSession->payload.vertices[i].z = res.visualMesh.positions[i * 3 + 2];
            outSession->payload.vertices[i].nx = res.visualMesh.colors[i * 3 + 0];
            outSession->payload.vertices[i].ny = res.visualMesh.colors[i * 3 + 1];
            outSession->payload.vertices[i].nz = res.visualMesh.colors[i * 3 + 2];
            outSession->payload.vertices[i].faceId = static_cast<uint32_t>(res.visualMesh.stressesMPa[i]);
        }
        outSession->payload.indices = res.visualMesh.indices;
    }

    jlong handle = g_meshRegistry.Register(std::move(outSession));

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"nodeCount\":" << res.nodeCount
       << ",\"elementCount\":" << res.elementCount
       << ",\"maxVonMisesStressMPa\":" << res.maxVonMisesStressMPa
       << ",\"minVonMisesStressMPa\":" << res.minVonMisesStressMPa
       << ",\"maxDisplacementMm\":" << res.maxDisplacementMm
       << ",\"minSafetyFactor\":" << res.minSafetyFactor
       << ",\"meshHandle\":" << handle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSimulateCantileverTet10(
        JNIEnv* env,
        jobject /* this */,
        jdouble lengthMm,
        jdouble widthMm,
        jdouble heightMm,
        jdouble forceN,
        jint materialId,
        jdouble deformationScale) {
    androcad::Material mat = androcad::MaterialEngine::GetMaterial(materialId);
    androcad::SimulationResult res = androcad::SimulationEngine::SolveCantileverBeamTet10(
        lengthMm, widthMm, heightMm, forceN, mat, deformationScale);

    auto session = std::make_unique<NativeMeshSession>();
    session->payload.success = res.success;
    session->payload.errorMessage = res.errorMessage;

    if (res.success) {
        size_t vertCount = res.visualMesh.positions.size() / 3;
        session->payload.vertices.resize(vertCount);
        for (size_t i = 0; i < vertCount; ++i) {
            session->payload.vertices[i].x = res.visualMesh.positions[i * 3 + 0];
            session->payload.vertices[i].y = res.visualMesh.positions[i * 3 + 1];
            session->payload.vertices[i].z = res.visualMesh.positions[i * 3 + 2];
            session->payload.vertices[i].nx = res.visualMesh.colors[i * 3 + 0];
            session->payload.vertices[i].ny = res.visualMesh.colors[i * 3 + 1];
            session->payload.vertices[i].nz = res.visualMesh.colors[i * 3 + 2];
            session->payload.vertices[i].faceId = static_cast<uint32_t>(res.visualMesh.stressesMPa[i]);
        }
        session->payload.indices = res.visualMesh.indices;
    }

    jlong handle = g_meshRegistry.Register(std::move(session));

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"nodeCount\":" << res.nodeCount
       << ",\"elementCount\":" << res.elementCount
       << ",\"maxVonMisesStressMPa\":" << res.maxVonMisesStressMPa
       << ",\"minVonMisesStressMPa\":" << res.minVonMisesStressMPa
       << ",\"maxDisplacementMm\":" << res.maxDisplacementMm
       << ",\"minSafetyFactor\":" << res.minSafetyFactor
       << ",\"meshHandle\":" << handle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSimulateModal(
        JNIEnv* env,
        jobject /* this */,
        jdouble lengthMm,
        jdouble widthMm,
        jdouble heightMm,
        jint materialId,
        jint numModes,
        jdouble deformationScale) {
    androcad::Material mat = androcad::MaterialEngine::GetMaterial(materialId);
    androcad::ModalAnalysisResult res = androcad::SimulationEngine::SolveCantileverModal(
        lengthMm, widthMm, heightMm, mat, numModes, deformationScale);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"nodeCount\":" << res.nodeCount
       << ",\"elementCount\":" << res.elementCount
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << ",\"modes\":[";

    for (size_t m = 0; m < res.modes.size(); ++m) {
        const auto& mode = res.modes[m];
        auto session = std::make_unique<NativeMeshSession>();
        session->payload.success = true;

        size_t vertCount = mode.visualMesh.positions.size() / 3;
        session->payload.vertices.resize(vertCount);
        for (size_t i = 0; i < vertCount; ++i) {
            session->payload.vertices[i].x = mode.visualMesh.positions[i * 3 + 0];
            session->payload.vertices[i].y = mode.visualMesh.positions[i * 3 + 1];
            session->payload.vertices[i].z = mode.visualMesh.positions[i * 3 + 2];
            session->payload.vertices[i].nx = mode.visualMesh.colors[i * 3 + 0];
            session->payload.vertices[i].ny = mode.visualMesh.colors[i * 3 + 1];
            session->payload.vertices[i].nz = mode.visualMesh.colors[i * 3 + 2];
            session->payload.vertices[i].faceId = static_cast<uint32_t>(mode.visualMesh.stressesMPa[i]);
        }
        session->payload.indices = mode.visualMesh.indices;

        jlong handle = g_meshRegistry.Register(std::move(session));

        if (m > 0) ss << ",";
        ss << "{\"modeNumber\":" << mode.modeNumber
           << ",\"frequencyHz\":" << mode.frequencyHz
           << ",\"angularFreqRadS\":" << mode.angularFreqRadS
           << ",\"meshHandle\":" << handle
           << "}";
    }
    ss << "]}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCAMGenerateGCode(
        JNIEnv* env,
        jobject /* this */,
        jdouble lengthMm,
        jdouble widthMm,
        jdouble heightMm,
        jdouble pocketDepthMm,
        jint dialectId) {
    androcad::PostProcessorDialect dialect = androcad::PostProcessorDialect::ISO_GRBL;
    if (dialectId == 1) dialect = androcad::PostProcessorDialect::HAAS_CNC;
    else if (dialectId == 2) dialect = androcad::PostProcessorDialect::FANUC_CNC;

    androcad::GCodeResult res = androcad::CAMEngine::GenerateCompleteBracketSetup(
        lengthMm, widthMm, heightMm, pocketDepthMm, dialect);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"lineCount\":" << res.lineCount
       << ",\"cuttingTimeSec\":" << res.totalCuttingTimeSec
       << ",\"rapidTimeSec\":" << res.totalRapidTimeSec
       << ",\"totalLengthMm\":" << res.totalLengthMm
       << ",\"materialVolumeRemovedCm3\":" << res.materialVolumeRemovedCm3
       << ",\"minX\":" << res.minX << ",\"maxX\":" << res.maxX
       << ",\"minY\":" << res.minY << ",\"maxY\":" << res.maxY
       << ",\"minZ\":" << res.minZ << ",\"maxZ\":" << res.maxZ
       << ",\"gcode\":\"";

    for (char c : res.gcode) {
        if (c == '\n') ss << "\\n";
        else if (c == '\"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\r') continue;
        else ss << c;
    }

    ss << "\",\"errorMessage\":\"" << res.errorMessage << "\"}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jfloatArray JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCAMGetVisualLines(
        JNIEnv* env,
        jobject /* this */,
        jdouble lengthMm,
        jdouble widthMm,
        jdouble heightMm,
        jdouble pocketDepthMm) {
    androcad::GCodeResult res = androcad::CAMEngine::GenerateCompleteBracketSetup(
        lengthMm, widthMm, heightMm, pocketDepthMm, androcad::PostProcessorDialect::ISO_GRBL);

    const auto& lines = res.visualPath.lineVertices;
    jfloatArray outArr = env->NewFloatArray(static_cast<jsize>(lines.size()));
    if (!lines.empty()) {
        env->SetFloatArrayRegion(outArr, 0, static_cast<jsize>(lines.size()), lines.data());
    }
    return outArr;
}

// Milestone 16: Patterns, Symmetry & Draft Operations
JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeRectangularPattern(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jint countU, jdouble spacingU, jdouble dirUX, jdouble dirUY, jdouble dirUZ,
        jint countV, jdouble spacingV, jdouble dirVX, jdouble dirVY, jdouble dirVZ,
        jboolean fuseInstances) {
    auto* session = g_meshRegistry.Get(solidHandle);
    TopoDS_Shape baseShape = (session && !session->shape.IsNull()) ?
        session->shape : BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();

    gp_Vec dirU(dirUX, dirUY, dirUZ);
    gp_Vec dirV(dirVX, dirVY, dirVZ);

    androcad::PatternResult res = androcad::PatternEngine::RectangularPattern(
        baseShape, countU, spacingU, dirU, countV, spacingV, dirV, fuseInstances
    );

    jlong newHandle = 0;
    if (res.success && !res.shape.IsNull()) {
        auto* newSession = new NativeMeshSession();
        newSession->shape = res.shape;
        newSession->payload = androcad::FeatureOps::ShapeToMeshPayload(res.shape, 0.1, 0.5);
        newHandle = g_meshRegistry.Register(newSession);
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"instanceCount\":" << res.instanceCount
       << ",\"totalVolumeMm3\":" << res.totalVolumeMm3
       << ",\"meshHandle\":" << newHandle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCircularPattern(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jdouble ox, jdouble oy, jdouble oz,
        jdouble dx, jdouble dy, jdouble dz,
        jint count, jdouble totalAngleDeg,
        jboolean fuseInstances) {
    auto* session = g_meshRegistry.Get(solidHandle);
    TopoDS_Shape baseShape = (session && !session->shape.IsNull()) ?
        session->shape : BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();

    gp_Ax1 axis(gp_Pnt(ox, oy, oz), gp_Dir(dx, dy, dz));
    androcad::PatternResult res = androcad::PatternEngine::CircularPattern(
        baseShape, axis, count, totalAngleDeg, fuseInstances
    );

    jlong newHandle = 0;
    if (res.success && !res.shape.IsNull()) {
        auto* newSession = new NativeMeshSession();
        newSession->shape = res.shape;
        newSession->payload = androcad::FeatureOps::ShapeToMeshPayload(res.shape, 0.1, 0.5);
        newHandle = g_meshRegistry.Register(newSession);
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"instanceCount\":" << res.instanceCount
       << ",\"totalVolumeMm3\":" << res.totalVolumeMm3
       << ",\"meshHandle\":" << newHandle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeMirror(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jdouble ox, jdouble oy, jdouble oz,
        jdouble nx, jdouble ny, jdouble nz,
        jboolean keepOriginal,
        jboolean fuseInstances) {
    auto* session = g_meshRegistry.Get(solidHandle);
    TopoDS_Shape baseShape = (session && !session->shape.IsNull()) ?
        session->shape : BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();

    gp_Ax2 mirrorPlane(gp_Pnt(ox, oy, oz), gp_Dir(nx, ny, nz));
    androcad::PatternResult res = androcad::PatternEngine::Mirror(
        baseShape, mirrorPlane, keepOriginal, fuseInstances
    );

    jlong newHandle = 0;
    if (res.success && !res.shape.IsNull()) {
        auto* newSession = new NativeMeshSession();
        newSession->shape = res.shape;
        newSession->payload = androcad::FeatureOps::ShapeToMeshPayload(res.shape, 0.1, 0.5);
        newHandle = g_meshRegistry.Register(newSession);
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"instanceCount\":" << res.instanceCount
       << ",\"totalVolumeMm3\":" << res.totalVolumeMm3
       << ",\"meshHandle\":" << newHandle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeApplyDraft(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jdouble dirX, jdouble dirY, jdouble dirZ,
        jdouble draftAngleDeg,
        jintArray faceIds) {
    auto* session = g_meshRegistry.Get(solidHandle);
    TopoDS_Shape baseShape = (session && !session->shape.IsNull()) ?
        session->shape : BRepPrimAPI_MakeBox(20.0, 20.0, 30.0).Shape();

    std::vector<int> targetFaces;
    if (faceIds != nullptr) {
        jsize len = env->GetArrayLength(faceIds);
        jint* elems = env->GetIntArrayElements(faceIds, nullptr);
        for (jsize i = 0; i < len; ++i) {
            targetFaces.push_back(elems[i]);
        }
        env->ReleaseIntArrayElements(faceIds, elems, JNI_ABORT);
    }
    if (targetFaces.empty()) targetFaces.push_back(1);

    gp_Dir pullDir(dirX, dirY, dirZ);
    androcad::PatternResult res = androcad::PatternEngine::ApplyDraft(
        baseShape, pullDir, draftAngleDeg, targetFaces
    );

    jlong newHandle = 0;
    if (res.success && !res.shape.IsNull()) {
        auto* newSession = new NativeMeshSession();
        newSession->shape = res.shape;
        newSession->payload = androcad::FeatureOps::ShapeToMeshPayload(res.shape, 0.1, 0.5);
        newHandle = g_meshRegistry.Register(newSession);
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"instanceCount\":" << res.instanceCount
       << ",\"totalVolumeMm3\":" << res.totalVolumeMm3
       << ",\"meshHandle\":" << newHandle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

// Milestone 17: Threads, Fasteners & Associative Sketch Projection
JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCreateStandardHole(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jdouble px, jdouble py, jdouble pz,
        jdouble dx, jdouble dy, jdouble dz,
        jstring standardStr,
        jint holeTypeId,
        jdouble depthMm) {
    auto* session = g_meshRegistry.Get(solidHandle);
    TopoDS_Shape baseShape = (session && !session->shape.IsNull()) ?
        session->shape : BRepPrimAPI_MakeBox(50.0, 50.0, 20.0).Shape();

    const char* stdChars = env->GetStringUTFChars(standardStr, nullptr);
    std::string standardName = stdChars ? stdChars : "M6";
    if (stdChars) env->ReleaseStringUTFChars(standardStr, stdChars);

    androcad::HoleType type = static_cast<androcad::HoleType>(holeTypeId);
    gp_Pnt pos(px, py, pz);
    gp_Dir dir(dx, dy, dz);

    androcad::HoleResult res = androcad::FastenerEngine::CreateStandardHole(
        baseShape, pos, dir, standardName, type, depthMm
    );

    jlong newHandle = 0;
    if (res.success && !res.shape.IsNull()) {
        auto* newSession = new NativeMeshSession();
        newSession->shape = res.shape;
        newSession->payload = androcad::FeatureOps::ShapeToMeshPayload(res.shape, 0.1, 0.5);
        newHandle = g_meshRegistry.Register(newSession);
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"volumeMm3\":" << res.volumeMm3
       << ",\"meshHandle\":" << newHandle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCreateThreadedBolt(
        JNIEnv* env,
        jobject /* this */,
        jstring standardStr,
        jdouble shaftLengthMm,
        jdouble threadLengthMm,
        jboolean hexHead) {
    const char* stdChars = env->GetStringUTFChars(standardStr, nullptr);
    std::string standardName = stdChars ? stdChars : "M8";
    if (stdChars) env->ReleaseStringUTFChars(standardStr, stdChars);

    androcad::ThreadResult res = androcad::FastenerEngine::CreateThreadedBolt(
        standardName, shaftLengthMm, threadLengthMm, hexHead
    );

    jlong newHandle = 0;
    if (res.success && !res.shape.IsNull()) {
        auto* newSession = new NativeMeshSession();
        newSession->shape = res.shape;
        newSession->payload = androcad::FeatureOps::ShapeToMeshPayload(res.shape, 0.1, 0.5);
        newHandle = g_meshRegistry.Register(newSession);
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"toothCount\":" << res.toothCount
       << ",\"pitchMm\":" << res.pitchMm
       << ",\"outerDiameterMm\":" << res.outerDiameterMm
       << ",\"volumeMm3\":" << res.volumeMm3
       << ",\"meshHandle\":" << newHandle
       << ",\"errorMessage\":\"" << res.errorMessage << "\""
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeProjectEdges(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jdouble ox, jdouble oy, jdouble oz,
        jdouble nx, jdouble ny, jdouble nz) {
    auto* session = g_meshRegistry.Get(solidHandle);
    TopoDS_Shape baseShape = (session && !session->shape.IsNull()) ?
        session->shape : BRepPrimAPI_MakeBox(30.0, 20.0, 10.0).Shape();

    gp_Pln sketchPlane(gp_Pnt(ox, oy, oz), gp_Dir(nx, ny, nz));
    androcad::ProjectionResult res = androcad::FastenerEngine::ProjectEdgesToPlane(baseShape, sketchPlane);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"edgeCount\":" << res.edgeCount
       << ",\"segments\":[";

    for (size_t i = 0; i < res.segments.size(); ++i) {
        const auto& seg = res.segments[i];
        if (i > 0) ss << ",";
        ss << "{\"startU\":" << seg.startU << ",\"startV\":" << seg.startV
           << ",\"endU\":" << seg.endU << ",\"endV\":" << seg.endV
           << ",\"isArc\":" << (seg.isArc ? "true" : "false")
           << ",\"centerU\":" << seg.centerU << ",\"centerV\":" << seg.centerV
           << ",\"radius\":" << seg.radius << "}";
    }

    ss << "],\"errorMessage\":\"" << res.errorMessage << "\"}";
    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGenerateBSpline(
        JNIEnv* env,
        jobject /* this */,
        jdoubleArray uCoords,
        jdoubleArray vCoords,
        jint sampleCount) {
    jsize len = env->GetArrayLength(uCoords);
    jdouble* uElems = env->GetDoubleArrayElements(uCoords, nullptr);
    jdouble* vElems = env->GetDoubleArrayElements(vCoords, nullptr);

    std::vector<androcad::SplinePoint2D> pts;
    pts.reserve(len);
    for (jsize i = 0; i < len; ++i) {
        pts.push_back({uElems[i], vElems[i]});
    }

    env->ReleaseDoubleArrayElements(uCoords, uElems, JNI_ABORT);
    env->ReleaseDoubleArrayElements(vCoords, vElems, JNI_ABORT);

    androcad::BSplineResult res = androcad::FastenerEngine::GenerateCubicBSpline(pts, sampleCount);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"degree\":" << res.degree
       << ",\"knotCount\":" << res.knotCount
       << ",\"poleCount\":" << res.poleCount
       << ",\"lengthMm\":" << res.lengthMm
       << ",\"samples\":[";

    for (size_t i = 0; i < res.sampledPoints.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "{\"u\":" << res.sampledPoints[i].u << ",\"v\":" << res.sampledPoints[i].v << "}";
    }

    ss << "],\"errorMessage\":\"" << res.errorMessage << "\"}";
    return env->NewStringUTF(ss.str().c_str());
}

// ==========================================
// Milestone 18: 3D CAM Surfacing & Additive Slicing
// ==========================================

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGenerateAdaptive3D(
        JNIEnv* env,
        jobject /* this */,
        jint opId,
        jstring opName,
        jdouble minX, jdouble maxX,
        jdouble minY, jdouble maxY,
        jdouble topZ, jdouble bottomZ,
        jdouble stepdownMm,
        jdouble optimalLoadMm) {
    const char* nameChars = env->GetStringUTFChars(opName, nullptr);
    std::string nameStr(nameChars ? nameChars : "Adaptive 3D");
    if (nameChars) env->ReleaseStringUTFChars(opName, nameChars);

    androcad::CAMEngine cam;
    androcad::ToolDefinition tool = cam.GetTool(1); // 10mm endmill
    androcad::CAMOperation op = cam.GenerateAdaptiveClearing3D(
        opId, nameStr, tool, minX, maxX, minY, maxY, topZ, bottomZ, stepdownMm, optimalLoadMm);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{\"success\":true"
       << ",\"opId\":" << op.id
       << ",\"name\":\"" << op.name << "\""
       << ",\"pointCount\":" << op.pathPoints.size()
       << ",\"totalDistanceMm\":" << op.totalDistanceMm
       << ",\"machiningTimeSec\":" << op.machiningTimeSec
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGenerateScallop3D(
        JNIEnv* env,
        jobject /* this */,
        jint opId,
        jstring opName,
        jdouble minX, jdouble maxX,
        jdouble minY, jdouble maxY,
        jdouble topZ, jdouble bottomZ,
        jdouble stepoverMm,
        jdouble cuspHeightMm) {
    const char* nameChars = env->GetStringUTFChars(opName, nullptr);
    std::string nameStr(nameChars ? nameChars : "Scallop 3D");
    if (nameChars) env->ReleaseStringUTFChars(opName, nameChars);

    androcad::CAMEngine cam;
    androcad::ToolDefinition tool = cam.GetTool(2); // 6mm ball endmill
    androcad::CAMOperation op = cam.GenerateScallop3D(
        opId, nameStr, tool, minX, maxX, minY, maxY, topZ, bottomZ, stepoverMm, cuspHeightMm);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{\"success\":true"
       << ",\"opId\":" << op.id
       << ",\"name\":\"" << op.name << "\""
       << ",\"pointCount\":" << op.pathPoints.size()
       << ",\"totalDistanceMm\":" << op.totalDistanceMm
       << ",\"machiningTimeSec\":" << op.machiningTimeSec
       << "}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSliceSolid(
        JNIEnv* env,
        jobject /* this */,
        jdouble minX, jdouble maxX,
        jdouble minY, jdouble maxY,
        jdouble minZ, jdouble maxZ,
        jdouble layerHeightMm,
        jdouble nozzleDiameterMm,
        jdouble infillDensityPercent,
        jint infillPatternCode,
        jdouble printSpeedMmS,
        jdouble nozzleTempC,
        jdouble bedTempC) {
    androcad::SlicerConfig config;
    config.layerHeightMm = layerHeightMm;
    config.initialLayerHeightMm = layerHeightMm;
    config.nozzleDiameterMm = nozzleDiameterMm;
    config.infillDensityPercent = infillDensityPercent;
    config.printSpeedMmS = printSpeedMmS;
    config.nozzleTempC = nozzleTempC;
    config.bedTempC = bedTempC;
    if (infillPatternCode == 1) config.infillPattern = androcad::InfillPattern::HONEYCOMB;
    else if (infillPatternCode == 2) config.infillPattern = androcad::InfillPattern::GRID;
    else config.infillPattern = androcad::InfillPattern::RECTILINEAR;

    androcad::SlicerResult res = androcad::CAMEngine::SliceBoundingBox(
        minX, maxX, minY, maxY, minZ, maxZ, config);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{\"success\":" << (res.success ? "true" : "false")
       << ",\"totalLayers\":" << res.totalLayers
       << ",\"totalFilamentMm\":" << res.totalFilamentMm
       << ",\"totalFilamentGrams\":" << res.totalFilamentGrams
       << ",\"estimatedPrintTimeMin\":" << res.estimatedPrintTimeMin
       << ",\"gcodeSize\":" << res.gcode.size()
       << ",\"errorMessage\":\"" << res.errorMessage << "\"}";

    return env->NewStringUTF(ss.str().c_str());
}

// -------------------------------------------------------------------------
// Milestone 20: GD&T, Bill of Materials (BOM) & Multi-Sheet Vector Drafting
// -------------------------------------------------------------------------

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGenerateMultiSheetDocument(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jstring partNameStr) {
    auto* session = g_meshRegistry.Get(solidHandle);
    if (!session || session->shape.IsNull()) {
        return env->NewStringUTF("{\"success\":false,\"errorMessage\":\"Invalid solid handle\"}");
    }

    const char* partNameChars = env->GetStringUTFChars(partNameStr, nullptr);
    std::string partName = partNameChars ? partNameChars : "Mechanical Assembly";
    if (partNameChars) env->ReleaseStringUTFChars(partNameStr, partNameChars);

    std::vector<androcad::BOMItem> items;
    items.push_back({1, "AC-1001", "Base Chassis Frame", 1, "Structural Steel", 450.0, 450.0});
    items.push_back({2, "AC-1002", "Precision Bushing", 2, "Phosphor Bronze", 65.0, 130.0});
    items.push_back({3, "ISO-4017-M6", "Hex Head Bolt M6x25", 4, "Grade 8.8 Steel", 12.5, 50.0});
    items.push_back({4, "ISO-7089-6", "Flat Plain Washer 6mm", 4, "Stainless Steel", 2.0, 8.0});

    androcad::DrawingDocument doc = androcad::DrawingEngine::GenerateMultiSheetDocument(
        session->shape, items, partName);

    std::ostringstream ss;
    ss << "{\"success\":true"
       << ",\"title\":\"" << doc.title << "\""
       << ",\"sheetCount\":" << doc.sheets.size()
       << ",\"sheets\":[";
    for (size_t i = 0; i < doc.sheets.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "{\"sheetIndex\":" << doc.sheets[i].sheetIndex
           << ",\"sheetName\":\"" << doc.sheets[i].sheetName << "\""
           << ",\"viewCount\":" << doc.sheets[i].views.size()
           << ",\"widthMm\":" << doc.sheets[i].sheetWidth
           << ",\"heightMm\":" << doc.sheets[i].sheetHeight
           << "}";
    }
    ss << "]}";

    return env->NewStringUTF(ss.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeExportMultiSheetSVG(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jstring partNameStr,
        jint sheetIndex) {
    auto* session = g_meshRegistry.Get(solidHandle);
    if (!session || session->shape.IsNull()) {
        return env->NewStringUTF("<svg></svg>");
    }

    const char* partNameChars = env->GetStringUTFChars(partNameStr, nullptr);
    std::string partName = partNameChars ? partNameChars : "Mechanical Assembly";
    if (partNameChars) env->ReleaseStringUTFChars(partNameStr, partNameChars);

    std::vector<androcad::BOMItem> items;
    items.push_back({1, "AC-1001", "Base Chassis Frame", 1, "Structural Steel", 450.0, 450.0});
    items.push_back({2, "AC-1002", "Precision Bushing", 2, "Phosphor Bronze", 65.0, 130.0});
    items.push_back({3, "ISO-4017-M6", "Hex Head Bolt M6x25", 4, "Grade 8.8 Steel", 12.5, 50.0});
    items.push_back({4, "ISO-7089-6", "Flat Plain Washer 6mm", 4, "Stainless Steel", 2.0, 8.0});

    androcad::DrawingDocument doc = androcad::DrawingEngine::GenerateMultiSheetDocument(
        session->shape, items, partName);

    if (sheetIndex <= 0) {
        std::string multiSvg = doc.ToMultiSheetSVG();
        return env->NewStringUTF(multiSvg.c_str());
    } else {
        uint32_t sIdx = static_cast<uint32_t>(sheetIndex);
        for (const auto& sheet : doc.sheets) {
            if (sheet.sheetIndex == sIdx) {
                std::string sheetSvg = sheet.ToSVG();
                return env->NewStringUTF(sheetSvg.c_str());
            }
        }
        return env->NewStringUTF("<svg></svg>");
    }
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeExportMultiSheetDXF(
        JNIEnv* env,
        jobject /* this */,
        jlong solidHandle,
        jstring partNameStr) {
    auto* session = g_meshRegistry.Get(solidHandle);
    if (!session || session->shape.IsNull()) {
        return env->NewStringUTF("");
    }

    const char* partNameChars = env->GetStringUTFChars(partNameStr, nullptr);
    std::string partName = partNameChars ? partNameChars : "Mechanical Assembly";
    if (partNameChars) env->ReleaseStringUTFChars(partNameStr, partNameChars);

    std::vector<androcad::BOMItem> items;
    items.push_back({1, "AC-1001", "Base Chassis Frame", 1, "Structural Steel", 450.0, 450.0});
    items.push_back({2, "AC-1002", "Precision Bushing", 2, "Phosphor Bronze", 65.0, 130.0});

    androcad::DrawingDocument doc = androcad::DrawingEngine::GenerateMultiSheetDocument(
        session->shape, items, partName);

    std::string dxf = doc.ToMultiSheetDXF();
    return env->NewStringUTF(dxf.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeFormatGDTFrame(
        JNIEnv* env,
        jobject /* this */,
        jint characteristicCode,
        jdouble tolerance,
        jboolean hasDiameterSymbol,
        jint materialConditionCode,
        jstring primaryDatumStr,
        jstring secondaryDatumStr,
        jstring tertiaryDatumStr) {
    const char* pChars = env->GetStringUTFChars(primaryDatumStr, nullptr);
    std::string primary = pChars ? pChars : "";
    if (pChars) env->ReleaseStringUTFChars(primaryDatumStr, pChars);

    const char* sChars = env->GetStringUTFChars(secondaryDatumStr, nullptr);
    std::string secondary = sChars ? sChars : "";
    if (sChars) env->ReleaseStringUTFChars(secondaryDatumStr, sChars);

    const char* tChars = env->GetStringUTFChars(tertiaryDatumStr, nullptr);
    std::string tertiary = tChars ? tChars : "";
    if (tChars) env->ReleaseStringUTFChars(tertiaryDatumStr, tChars);

    androcad::GDTCharacteristic charac = static_cast<androcad::GDTCharacteristic>(characteristicCode);
    androcad::MaterialCondition matCond = static_cast<androcad::MaterialCondition>(materialConditionCode);

    androcad::FeatureControlFrame fcf = androcad::DrawingEngine::CreateGDTFrame(
        charac, tolerance, hasDiameterSymbol, matCond, primary, secondary, tertiary);

    return env->NewStringUTF(fcf.FormattedString().c_str());
}

} // extern "C"