// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include <jni.h>
#include "AndroCadKernel.h"
#include "Sketcher.h"

#include <memory>
#include <vector>

namespace {

struct NativeMeshSession {
    androcad::MeshPayload payload;
};

struct NativeSketchSession {
    std::unique_ptr<androcad::Sketcher> sketcher;
    androcad::SketchResult lastResult;
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
    
    androcad::AndroCadKernel kernel;
    auto session = std::make_unique<NativeMeshSession>();
    session->payload = kernel.CreateBox(dx, dy, dz, linearDeflection, angularDeflection);

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

} // extern "C"
