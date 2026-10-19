#!/usr/bin/env bash
# Copyright (c) 2026 dursuntokgoz. All rights reserved.
# Build pruned Open CASCADE Technology (OCCT) 7.8.0 for Android arm64-v8a / x86_64
set -euo pipefail

OCCT_VERSION="V7_8_0"
TARGET_ABI="${1:-arm64-v8a}"
ANDROID_API="${2:-24}"
BUILD_DIR="build-occt-${TARGET_ABI}"
INSTALL_DIR="occt-install-android-${TARGET_ABI}"

if [ -z "${ANDROID_NDK_HOME:-}" ]; then
  if [ -n "${ANDROID_NDK_ROOT:-}" ]; then
    ANDROID_NDK_HOME="${ANDROID_NDK_ROOT}"
  elif [ -d "/usr/local/lib/android/sdk/ndk/26.3.11579264" ]; then
    ANDROID_NDK_HOME="/usr/local/lib/android/sdk/ndk/26.3.11579264"
  elif [ -d "${ANDROID_HOME:-}/ndk/26.3.11579264" ]; then
    ANDROID_NDK_HOME="${ANDROID_HOME}/ndk/26.3.11579264"
  else
    # Find newest installed NDK
    ANDROID_NDK_HOME=$(find "${ANDROID_HOME:-/usr/local/lib/android/sdk}/ndk" -mindepth 1 -maxdepth 1 -type d | sort -V | tail -n 1)
  fi
fi

echo "=========================================================="
echo " Building OCCT ${OCCT_VERSION} for Android ${TARGET_ABI} "
echo " NDK: ${ANDROID_NDK_HOME} (API ${ANDROID_API})"
echo "=========================================================="

# 1. Fetch OCCT source if not present
if [ ! -d "occt-src" ]; then
  echo "Cloning OCCT ${OCCT_VERSION} (shallow)..."
  git clone --depth 1 --branch "${OCCT_VERSION}" https://github.com/Open-Cascade-SAS/OCCT.git occt-src
fi

# 2. Configure with CMake
mkdir -p "${BUILD_DIR}"
mkdir -p "${INSTALL_DIR}"

CMAKE_TOOLCHAIN="${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake"
if [ ! -f "${CMAKE_TOOLCHAIN}" ]; then
  echo "ERROR: NDK CMake toolchain not found at ${CMAKE_TOOLCHAIN}"
  exit 1
fi

# Test first with ApplicationFramework=OFF
echo "Attempting CMake configuration with ApplicationFramework=OFF..."
ENABLE_OCAF="OFF"

set +e
cmake -B "${BUILD_DIR}" -S occt-src -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN}" \
  -DANDROID_ABI="${TARGET_ABI}" \
  -DANDROID_PLATFORM="android-${ANDROID_API}" \
  -DANDROID_STL="c++_shared" \
  -DCMAKE_BUILD_TYPE="Release" \
  -DBUILD_LIBRARY_TYPE="Shared" \
  -DBUILD_MODULE_FoundationClasses=ON \
  -DBUILD_MODULE_ModelingData=ON \
  -DBUILD_MODULE_ModelingAlgorithms=ON \
  -DBUILD_MODULE_Visualization=OFF \
  -DBUILD_MODULE_ApplicationFramework="${ENABLE_OCAF}" \
  -DBUILD_MODULE_DataExchange=ON \
  -DBUILD_MODULE_Draw=OFF \
  -DBUILD_MODULE_DETests=OFF \
  -DUSE_TBB=OFF \
  -DUSE_FREEIMAGE=OFF \
  -DUSE_FREETYPE=OFF \
  -DUSE_VTK=OFF \
  -DUSE_OPENVR=OFF \
  -DUSE_DRACO=OFF \
  -DCMAKE_CXX_FLAGS_RELEASE="-O2 -flto -ffunction-sections -fdata-sections -fvisibility=hidden" \
  -DCMAKE_SHARED_LINKER_FLAGS_RELEASE="-Wl,--gc-sections" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
CMAKE_RES=$?
set -e

if [ ${CMAKE_RES} -ne 0 ]; then
  echo "Configuration with ApplicationFramework=OFF failed. Enabling minimal ApplicationFramework for DataExchange dependencies..."
  ENABLE_OCAF="ON"
  cmake -B "${BUILD_DIR}" -S occt-src -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN}" \
    -DANDROID_ABI="${TARGET_ABI}" \
    -DANDROID_PLATFORM="android-${ANDROID_API}" \
    -DANDROID_STL="c++_shared" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DBUILD_LIBRARY_TYPE="Shared" \
    -DBUILD_MODULE_FoundationClasses=ON \
    -DBUILD_MODULE_ModelingData=ON \
    -DBUILD_MODULE_ModelingAlgorithms=ON \
    -DBUILD_MODULE_Visualization=OFF \
    -DBUILD_MODULE_ApplicationFramework="${ENABLE_OCAF}" \
    -DBUILD_MODULE_DataExchange=ON \
    -DBUILD_MODULE_Draw=OFF \
    -DBUILD_MODULE_DETests=OFF \
    -DUSE_TBB=OFF \
    -DUSE_FREEIMAGE=OFF \
    -DUSE_FREETYPE=OFF \
    -DUSE_VTK=OFF \
    -DUSE_OPENVR=OFF \
    -DUSE_DRACO=OFF \
    -DCMAKE_CXX_FLAGS_RELEASE="-O2 -flto -ffunction-sections -fdata-sections -fvisibility=hidden" \
    -DCMAKE_SHARED_LINKER_FLAGS_RELEASE="-Wl,--gc-sections" \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}"
fi

# 3. Compile and Install
echo "Building with Ninja..."
ninja -C "${BUILD_DIR}" -j"$(nproc)" install

# 4. Measure and Strip Libraries
STRIP_TOOL="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip"
if [ ! -f "${STRIP_TOOL}" ]; then
  STRIP_TOOL="llvm-strip"
fi

echo "=========================================================="
echo " OCCT Libraries Built for ${TARGET_ABI} "
echo "=========================================================="
mkdir -p "${INSTALL_DIR}/reports"
REPORT_FILE="${INSTALL_DIR}/reports/so_sizes.txt"
echo "Library Name | Unstripped (bytes) | Stripped (bytes)" > "${REPORT_FILE}"
echo "---|---|---" >> "${REPORT_FILE}"

LIB_DIR="${INSTALL_DIR}/lib"
if [ ! -d "${LIB_DIR}" ] && [ -d "${INSTALL_DIR}/lin64/gcc/lib" ]; then
  LIB_DIR="${INSTALL_DIR}/lin64/gcc/lib"
elif [ ! -d "${LIB_DIR}" ] && [ -d "${INSTALL_DIR}/android/clang/lib" ]; then
  LIB_DIR="${INSTALL_DIR}/android/clang/lib"
fi

for so_file in "${LIB_DIR}"/*.so; do
  if [ -f "${so_file}" ]; then
    fname=$(basename "${so_file}")
    size_unstripped=$(stat -c%s "${so_file}")
    "${STRIP_TOOL}" --strip-unneeded "${so_file}" || true
    size_stripped=$(stat -c%s "${so_file}")
    echo "${fname} | ${size_unstripped} | ${size_stripped}" >> "${REPORT_FILE}"
    echo "  ${fname}: ${size_stripped} bytes (stripped)"
  fi
done

TOTAL_STRIPPED=$(find "${LIB_DIR}" -name "*.so" -exec stat -c%s {} + | awk '{s+=$1} END {print s}')
echo "Total Stripped .so Size: $((TOTAL_STRIPPED / 1024 / 1024)) MB (${TOTAL_STRIPPED} bytes)" | tee -a "${REPORT_FILE}"
echo "OCAF Status: ${ENABLE_OCAF}" >> "${REPORT_FILE}"
echo "Build completed successfully."
