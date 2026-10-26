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
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/PatternEngine.cpp
        src/FastenerEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        src/Phase3Engine.cpp
        src/Phase4Engine.cpp
        src/Phase5Engine.cpp
        src/Phase6Engine.cpp
        src/Phase7Engine.cpp
        src/Phase8Engine.cpp
        src/Phase9Engine.cpp
        src/Phase10Engine.cpp
        src/Phase11Engine.cpp
        src/Phase12Engine.cpp
        src/Phase13Engine.cpp
        src/Phase14Engine.cpp
        src/Phase15Engine.cpp
        src/Phase16Engine.cpp
        src/Phase17Engine.cpp
        src/native_kernel_jni.cpp
    )

    target_compile_options(androcad_kernel PRIVATE
        -O2 -flto -ffunction-sections -fdata-sections -fvisibility=hidden -Wno-deprecated-declarations
    )

    target_link_options(androcad_kernel PRIVATE
        "-Wl,-z,max-page-size=16384"
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
        TKShHealing
        TKBO
        TKBool
        TKFillet
        TKOffset
        TKMesh
        TKDE
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

    add_executable(androcad_assembly_joints_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_assembly_joints.cpp
    )

    target_link_libraries(androcad_assembly_joints_test PRIVATE
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

    add_test(NAME AssemblyJointsTest COMMAND androcad_assembly_joints_test)

    add_executable(androcad_sheet_metal_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_sheet_metal.cpp
    )

    target_link_libraries(androcad_sheet_metal_test PRIVATE
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

    add_test(NAME SheetMetalTest COMMAND androcad_sheet_metal_test)

    add_executable(androcad_drawing_engine_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_drawing_engine.cpp
    )

    target_link_libraries(androcad_drawing_engine_test PRIVATE
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

    add_test(NAME DrawingEngineTest COMMAND androcad_drawing_engine_test)

    add_executable(androcad_materials_mass_props_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_materials_mass_props.cpp
    )

    target_link_libraries(androcad_materials_mass_props_test PRIVATE
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

    add_test(NAME MaterialsMassPropsTest COMMAND androcad_materials_mass_props_test)

    add_executable(androcad_mesh_engine_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_mesh_engine.cpp
    )

    target_link_libraries(androcad_mesh_engine_test PRIVATE
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

    add_test(NAME MeshEngineTest COMMAND androcad_mesh_engine_test)

    add_executable(androcad_simulation_engine_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_simulation_engine.cpp
    )

    target_link_libraries(androcad_simulation_engine_test PRIVATE
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

    add_test(NAME SimulationEngineTest COMMAND androcad_simulation_engine_test)

    add_executable(androcad_cam_engine_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_cam_engine.cpp
    )

    target_link_libraries(androcad_cam_engine_test PRIVATE
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

    add_test(NAME CAMEngineTest COMMAND androcad_cam_engine_test)

    add_executable(androcad_pattern_engine_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/PatternEngine.cpp
        src/FastenerEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_pattern_engine.cpp
    )

    target_link_libraries(androcad_pattern_engine_test PRIVATE
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

    add_test(NAME PatternEngineTest COMMAND androcad_pattern_engine_test)

    add_executable(androcad_fastener_engine_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/PatternEngine.cpp
        src/FastenerEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_fastener_engine.cpp
    )

    target_link_libraries(androcad_fastener_engine_test PRIVATE
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

    add_test(NAME FastenerEngineTest COMMAND androcad_fastener_engine_test)

    add_executable(androcad_cam_3d_slicer_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/PatternEngine.cpp
        src/FastenerEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_cam_3d_slicer.cpp
    )

    target_link_libraries(androcad_cam_3d_slicer_test PRIVATE
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

    add_test(NAME CAM3DSlicerTest COMMAND androcad_cam_3d_slicer_test)

    add_executable(androcad_fea_tet10_modal_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/PatternEngine.cpp
        src/FastenerEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_fea_tet10_modal.cpp
    )

    target_link_libraries(androcad_fea_tet10_modal_test PRIVATE
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

    add_test(NAME FEATet10ModalTest COMMAND androcad_fea_tet10_modal_test)

    add_executable(androcad_gdt_bom_drafting_test
        src/AndroCadKernel.cpp
        src/GCS.cpp
        src/Sketcher.cpp
        src/FeatureOps.cpp
        src/SurfaceOps.cpp
        src/AssemblyManager.cpp
        src/SheetMetalEngine.cpp
        src/DrawingEngine.cpp
        src/MaterialEngine.cpp
        src/MeshEngine.cpp
        src/SimulationEngine.cpp
        src/CAMEngine.cpp
        src/PatternEngine.cpp
        src/FastenerEngine.cpp
        src/ExpressionParser.cpp
        src/TNPResolver.cpp
        src/TimelineDAG.cpp
        src/ProjectIO.cpp
        tests/test_gdt_bom_drafting.cpp
    )

    target_link_libraries(androcad_gdt_bom_drafting_test PRIVATE
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

    add_test(NAME GDTBOMDraftingTest COMMAND androcad_gdt_bom_drafting_test)

    add_executable(androcad_faz3_suite_test
        src/Phase3Engine.cpp
        tests/test_faz3_suite.cpp
    )

    add_test(NAME Faz3SuiteTest COMMAND androcad_faz3_suite_test)

    add_executable(androcad_faz4_suite_test
        src/Phase4Engine.cpp
        tests/test_faz4_suite.cpp
    )

    add_test(NAME Faz4SuiteTest COMMAND androcad_faz4_suite_test)

    add_executable(androcad_faz5_suite_test
        src/Phase5Engine.cpp
        tests/test_faz5_suite.cpp
    )

    add_test(NAME Faz5SuiteTest COMMAND androcad_faz5_suite_test)

    add_executable(androcad_faz6_suite_test
        src/Phase6Engine.cpp
        tests/test_faz6_suite.cpp
    )

    add_test(NAME Faz6SuiteTest COMMAND androcad_faz6_suite_test)

    add_executable(androcad_faz7_suite_test
        src/Phase7Engine.cpp
        tests/test_faz7_suite.cpp
    )

    add_test(NAME Faz7SuiteTest COMMAND androcad_faz7_suite_test)

    add_executable(androcad_faz8_suite_test
        src/Phase8Engine.cpp
        tests/test_faz8_suite.cpp
    )

    add_test(NAME Faz8SuiteTest COMMAND androcad_faz8_suite_test)

    add_executable(androcad_faz9_suite_test
        src/Phase9Engine.cpp
        tests/test_faz9_suite.cpp
    )

    add_test(NAME Faz9SuiteTest COMMAND androcad_faz9_suite_test)

    add_executable(androcad_faz10_suite_test
        src/Phase10Engine.cpp
        tests/test_faz10_suite.cpp
    )

    add_test(NAME Faz10SuiteTest COMMAND androcad_faz10_suite_test)

    add_executable(androcad_faz11_suite_test
        src/Phase11Engine.cpp
        tests/test_faz11_suite.cpp
    )

    add_test(NAME Faz11SuiteTest COMMAND androcad_faz11_suite_test)

    add_executable(androcad_faz12_suite_test
        src/Phase12Engine.cpp
        tests/test_faz12_suite.cpp
    )

    add_test(NAME Faz12SuiteTest COMMAND androcad_faz12_suite_test)

    add_executable(androcad_faz13_suite_test
        src/Phase13Engine.cpp
        tests/test_faz13_suite.cpp
    )

    add_test(NAME Faz13SuiteTest COMMAND androcad_faz13_suite_test)

    add_executable(androcad_faz14_suite_test
        src/Phase14Engine.cpp
        src/Phase15Engine.cpp
        src/Phase16Engine.cpp
        src/Phase17Engine.cpp
        tests/test_faz14_suite.cpp
    )

    add_test(NAME Faz14SuiteTest COMMAND androcad_faz14_suite_test)
endif()