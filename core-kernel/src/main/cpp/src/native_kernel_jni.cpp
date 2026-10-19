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
        JNIEnv* env,
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
    return session ? session->payload.success : JNI_FALSE;
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

JNIEXPORT void JNICALL
Java_com_androcad_kernel_NativeKernel_nativeReleaseMesh(
        JNIEnv* /* env */,
        jobject /* this */,
        jlong handle) {
    auto* session = reinterpret_cast<NativeMeshSession*>(handle);
    delete session;
}

} // extern "C"
