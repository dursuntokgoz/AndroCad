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

#include <BRepPrimAPI_MakeBox.hxx>
#include <TopoDS_Shape.hxx>
#include <Standard_Failure.hxx>

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
    }

    return reinterpret_cast<jlong>(session.release());
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeIsSuccess(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
    return (session && session->payload.success) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jdouble JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetVolume(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
    return session ? session->payload.volume : 0.0;
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetFaceCount(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
    return session ? session->payload.faceCount : 0;
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetEdgeCount(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
    return session ? session->payload.edgeCount : 0;
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetTriangleCount(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
    return session ? session->payload.triangleCount : 0;
}

JNIEXPORT jstring JNICALL
Java_com_androcad_kernel_NativeKernel_nativeGetErrorMessage(
        JNIEnv* env,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
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
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
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
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
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
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
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
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
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
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
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
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
    delete session;
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

    return reinterpret_cast<jlong>(session.release());
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchAddPoint(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jdouble u, jdouble v,
        jboolean isFixed) {
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
    if (!session || !session->sketcher) return -1;
    return session->sketcher->AddPoint(u, v, isFixed == JNI_TRUE);
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchAddLine(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jint pStart, jint pEnd) {
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
    if (!session || !session->sketcher) return -1;
    return session->sketcher->AddLine(pStart, pEnd);
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchAddCircle(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle,
        jint pCenter, jdouble radius) {
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
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
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
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
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
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
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
    if (!session || !session->sketcher) return -1;
    androcad::gcs::SolveStatus status = session->sketcher->Solve();
    return static_cast<jint>(status);
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSketchGetDoF(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
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
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
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
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
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
    auto* session = reinterpret_cast<NativeSketchSession*>(handle);
    delete session;
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
    auto* sketchSession = reinterpret_cast<NativeSketchSession*>(sketchHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!sketchSession || !sketchSession->sketcher) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid sketch handle.";
        return reinterpret_cast<jlong>(meshSession.release());
    }

    if (sketchSession->lastResult.face.IsNull()) {
        sketchSession->lastResult = sketchSession->sketcher->BuildProfileFace();
    }
    if (sketchSession->lastResult.face.IsNull() || !sketchSession->lastResult.success) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Sketch does not contain a valid closed profile face.";
        return reinterpret_cast<jlong>(meshSession.release());
    }

    meshSession->payload = androcad::FeatureOps::MakeExtrude(
        sketchSession->lastResult.face,
        dx, dy, dz,
        meshSession->shape,
        linearDeflection,
        angularDeflection
    );
    return reinterpret_cast<jlong>(meshSession.release());
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
    auto* sketchSession = reinterpret_cast<NativeSketchSession*>(sketchHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!sketchSession || !sketchSession->sketcher) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid sketch handle.";
        return reinterpret_cast<jlong>(meshSession.release());
    }

    if (sketchSession->lastResult.face.IsNull()) {
        sketchSession->lastResult = sketchSession->sketcher->BuildProfileFace();
    }
    if (sketchSession->lastResult.face.IsNull() || !sketchSession->lastResult.success) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Sketch does not contain a valid closed profile face.";
        return reinterpret_cast<jlong>(meshSession.release());
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
    return reinterpret_cast<jlong>(meshSession.release());
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
    auto* solidSession = reinterpret_cast<NativeMeshSession*>(solidHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidSession || solidSession->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handle or null shape.";
        return reinterpret_cast<jlong>(meshSession.release());
    }

    meshSession->payload = androcad::FeatureOps::MakeFillet(
        solidSession->shape,
        radius,
        targetEdgeId,
        meshSession->shape,
        linearDeflection,
        angularDeflection
    );
    return reinterpret_cast<jlong>(meshSession.release());
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
    auto* solidSession = reinterpret_cast<NativeMeshSession*>(solidHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidSession || solidSession->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handle or null shape.";
        return reinterpret_cast<jlong>(meshSession.release());
    }

    meshSession->payload = androcad::FeatureOps::MakeChamfer(
        solidSession->shape,
        distance,
        targetEdgeId,
        meshSession->shape,
        linearDeflection,
        angularDeflection
    );
    return reinterpret_cast<jlong>(meshSession.release());
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
    auto* solidSession = reinterpret_cast<NativeMeshSession*>(solidHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidSession || solidSession->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handle or null shape.";
        return reinterpret_cast<jlong>(meshSession.release());
    }

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
    return reinterpret_cast<jlong>(meshSession.release());
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
    auto* solidSession = reinterpret_cast<NativeMeshSession*>(solidHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidSession || solidSession->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handle or null shape.";
        return reinterpret_cast<jlong>(meshSession.release());
    }

    meshSession->payload = androcad::FeatureOps::MakeShell(
        solidSession->shape,
        thickness,
        removeFaceId,
        meshSession->shape,
        linearDeflection,
        angularDeflection
    );
    return reinterpret_cast<jlong>(meshSession.release());
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
    auto* solidA = reinterpret_cast<NativeMeshSession*>(solidAHandle);
    auto* solidB = reinterpret_cast<NativeMeshSession*>(solidBHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!solidA || solidA->shape.IsNull() || !solidB || solidB->shape.IsNull()) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid solid handles for Boolean operation.";
        return reinterpret_cast<jlong>(meshSession.release());
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
    return reinterpret_cast<jlong>(meshSession.release());
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
    return reinterpret_cast<jlong>(session.release());
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeTimelineSetParameter(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring nameStr,
        jstring exprStr) {
    auto* session = reinterpret_cast<NativeTimelineSession*>(handle);
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
    auto* session = reinterpret_cast<NativeTimelineSession*>(handle);
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
    auto* session = reinterpret_cast<NativeTimelineSession*>(handle);
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
    auto* session = reinterpret_cast<NativeTimelineSession*>(handle);
    if (!session || !session->timeline) return;
    session->timeline->SetRollbackIndex(rollbackIndex);
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeTimelineRebuild(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeTimelineSession*>(handle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!session || !session->timeline) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid timeline handle.";
        return reinterpret_cast<jlong>(meshSession.release());
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

    return reinterpret_cast<jlong>(meshSession.release());
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeReleaseTimeline(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeTimelineSession*>(handle);
    delete session;
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
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
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

    return reinterpret_cast<jlong>(meshSession.release());
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
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
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

    return reinterpret_cast<jlong>(meshSession.release());
}

JNIEXPORT jboolean JNICALL
Java_com_androcad_kernel_NativeKernel_nativeSaveProject(
        JNIEnv* env,
        jobject /* this */,
        jlong handle,
        jstring filePathStr,
        jobjectArray outError) {
    auto* session = reinterpret_cast<NativeTimelineSession*>(handle);
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
    auto* session = reinterpret_cast<NativeTimelineSession*>(handle);
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
    }
    return reinterpret_cast<jlong>(session.release());
}

JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeThicken(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong solidHandle,
        jdouble thickness,
        jdouble linearDeflection,
        jdouble angularDeflection) {
    auto* inSession = reinterpret_cast<NativeMeshSession*>(solidHandle);
    auto outSession = std::make_unique<NativeMeshSession>();
    if (!inSession || inSession->shape.IsNull()) {
        outSession->payload.success = false;
        outSession->payload.errorMessage = "Invalid input shape for Thicken";
        return reinterpret_cast<jlong>(outSession.release());
    }
    try {
        outSession->payload = androcad::SurfaceOps::MakeThicken(inSession->shape, thickness, outSession->shape, linearDeflection, angularDeflection);
    } catch (const std::exception& e) {
        outSession->payload.success = false;
        outSession->payload.errorMessage = e.what();
    }
    return reinterpret_cast<jlong>(outSession.release());
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
    auto* inSession = reinterpret_cast<NativeMeshSession*>(solidHandle);
    auto outSession = std::make_unique<NativeMeshSession>();
    if (!inSession || inSession->shape.IsNull()) {
        outSession->payload.success = false;
        outSession->payload.errorMessage = "Invalid input shape for Press-Pull";
        return reinterpret_cast<jlong>(outSession.release());
    }
    try {
        outSession->payload = androcad::SurfaceOps::PressPullFace(inSession->shape, faceId, distance, outSession->shape, linearDeflection, angularDeflection);
    } catch (const std::exception& e) {
        outSession->payload.success = false;
        outSession->payload.errorMessage = e.what();
    }
    return reinterpret_cast<jlong>(outSession.release());
}

// Milestone 9: Assemblies & Kinematic Joints
JNIEXPORT jlong JNICALL
Java_com_androcad_kernel_NativeKernel_nativeCreateAssembly(
        JNIEnv* /* env */,
        jobject /* this */) {
    auto session = std::make_unique<NativeAssemblySession>();
    session->assembly = std::make_unique<androcad::AssemblyManager>();
    return reinterpret_cast<jlong>(session.release());
}

JNIEXPORT jint JNICALL
Java_com_androcad_kernel_NativeKernel_nativeAssemblyAddComponent(
        JNIEnv* env,
        jobject /* this */,
        jlong asmHandle,
        jstring nameStr,
        jlong meshHandle) {
    auto* asmSession = reinterpret_cast<NativeAssemblySession*>(asmHandle);
    auto* meshSession = reinterpret_cast<NativeMeshSession*>(meshHandle);
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
    auto* asmSession = reinterpret_cast<NativeAssemblySession*>(asmHandle);
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
    auto* asmSession = reinterpret_cast<NativeAssemblySession*>(asmHandle);
    if (!asmSession || !asmSession->assembly) return JNI_FALSE;
    return asmSession->assembly->SetJointValue(jointId, value) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeAssemblyStepAnimation(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong asmHandle,
        jdouble phase) {
    auto* asmSession = reinterpret_cast<NativeAssemblySession*>(asmHandle);
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
    auto* asmSession = reinterpret_cast<NativeAssemblySession*>(asmHandle);
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
    auto* asmSession = reinterpret_cast<NativeAssemblySession*>(asmHandle);
    auto meshSession = std::make_unique<NativeMeshSession>();
    if (!asmSession || !asmSession->assembly) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = "Invalid assembly handle";
        return reinterpret_cast<jlong>(meshSession.release());
    }

    try {
        meshSession->payload = asmSession->assembly->BuildAssemblyMeshPayload(linearDeflection, angularDeflection);
    } catch (const std::exception& e) {
        meshSession->payload.success = false;
        meshSession->payload.errorMessage = e.what();
    }
    return reinterpret_cast<jlong>(meshSession.release());
}

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeReleaseAssembly(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong asmHandle) {
    auto* asmSession = reinterpret_cast<NativeAssemblySession*>(asmHandle);
    delete asmSession;
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
    }
    return reinterpret_cast<jlong>(session.release());
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

} // extern "C"