// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include <jni.h>
#include "AndroCadKernel.h"

#include <memory>
#include <vector>

namespace {

struct NativeMeshSession {
    androcad::MeshPayload payload;
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

} // extern "C"
