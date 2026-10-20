# Copyright (c) 2026 dursuntokgoz. All rights reserved.
cmake_minimum_required(VERSION 3.22.1)
project(androcad_kernel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(ANDROID)
    message(STATUS "[AndroCAD] Configuring Android target for ABI: ${ANDROID_ABI}")

    if(NOT DEFINED OCCT_INSTALL_DIR)
        set(OCCT_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../occt-install-android-${ANDROID_ABI}")
    endif()

    message(STATUS "[AndroCAD] Using OCCT Install Dir: ${OCCT_INSTALL_DIR}")

    # Search headers in standard paths
    if(EXISTS "${OCCT_INSTALL_DIR}/include/opencascade")
        include_directories("${OCCT_INSTALL_DIR}/include/opencascade")
    elseif(EXISTS "${OCCT_INSTALL_DIR}/include")
        include_directories("${OCCT_INSTALL_DIR}/include")
    endif()

    # Search libraries in standard and sub-architecture paths
    link_directories("${OCCT_INSTALL_DIR}/lib")
    link_directories("${OCCT_INSTALL_DIR}/lin64/gcc/lib")
    link_directories("${OCCT_INSTALL_DIR}/android/clang/lib")

    add_library(androcad_kernel SHARED
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        src/native_kernel_jni.cpp
    )

    target_compile_options(androcad_kernel PRIVATE
        -O2 -flto -ffunction-sections -fdata-sections -fvisibility=hidden
    )

    target_link_libraries(androcad_kernel PRIVATE
        TKernel
        TKMath
        TKG2d
        TKG3d
        TKGeomBase
        TKGeomAlgo
        TKBRep
        TKTopAlgo
        TKPrim
        TKBO
        TKBool
        TKFillet
        TKOffset
        TKMesh
        TKDESTEP
        TKDESTL
        TKXSBase
        log
        android
    )

else()
    message(STATUS "[AndroCAD] Configuring Host-Side Test Suite")

    find_package(OpenCASCADE QUIET)
    include_directories(
        /usr/include/opencascade
        ${OpenCASCADE_INCLUDE_DIRS}
        ${OpenCASCADE_INCLUDE_DIR}
    )
    if(OpenCASCADE_LIBRARY_DIRS)
        link_directories(${OpenCASCADE_LIBRARY_DIRS})
    endif()

    add_executable(androcad_host_smoke_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        tests/test_kernel_smoke.cpp
    )

    target_link_libraries(androcad_host_smoke_test PRIVATE
        TKernel
        TKMath
        TKG2d
        TKG3d
        TKGeomBase
        TKGeomAlgo
        TKBRep
        TKTopAlgo
        TKPrim
        TKBO
        TKBool
        TKFillet
        TKOffset
        TKMesh
    )

    enable_testing()
    add_test(NAME KernelSmokeTest COMMAND androcad_host_smoke_test)

    add_executable(androcad_tnp_regression_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        tests/test_tnp_regression.cpp
    )

    target_link_libraries(androcad_tnp_regression_test PRIVATE
        TKernel
        TKMath
        TKG2d
        TKG3d
        TKGeomBase
        TKGeomAlgo
        TKBRep
        TKTopAlgo
        TKPrim
        TKBO
        TKBool
        TKFillet
        TKOffset
        TKMesh
    )

    add_test(NAME TNPRegressionTest COMMAND androcad_tnp_regression_test)

    # Find Data Exchange libraries on host (TKDESTEP/TKDESTL on OCCT 7.8+, or TKSTEP/TKSTL on OCCT 7.6)
    find_library(LIB_TKDESTEP NAMES TKDESTEP PATHS /usr/lib /usr/lib/x86_64-linux-gnu ${OpenCASCADE_LIBRARY_DIRS})
    if(LIB_TKDESTEP)
        set(HOST_DATA_EXCHANGE_LIBS TKDESTEP TKDESTL TKXSBase)
    else()
        set(HOST_DATA_EXCHANGE_LIBS TKSTEP TKSTEPBase TKSTEPAttr TKSTEP209 TKSTL TKXSBase)
    endif()

    add_executable(androcad_project_io_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_project_io.cpp
    )

    target_link_libraries(androcad_project_io_test PRIVATE
        TKernel
        TKMath
        TKG2d
        TKG3d
        TKGeomBase
        TKGeomAlgo
        TKBRep
        TKTopAlgo
        TKPrim
        TKBO
        TKBool
        TKFillet
        TKOffset
        TKMesh
        ${HOST_DATA_EXCHANGE_LIBS}
    )

    add_test(NAME ProjectIOTest COMMAND androcad_project_io_test)

    add_executable(androcad_surface_ops_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_surface_ops.cpp
    )

    target_link_libraries(androcad_surface_ops_test PRIVATE
        TKernel
        TKMath
        TKG2d
        TKG3d
        TKGeomBase
        TKGeomAlgo
        TKBRep
        TKTopAlgo
        TKPrim
        TKBO
        TKBool
        TKFillet
        TKOffset
        TKMesh
        ${HOST_DATA_EXCHANGE_LIBS}
    )

    add_test(NAME SurfaceOpsTest COMMAND androcad_surface_ops_test)
endif()