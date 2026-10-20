// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <TopoDS_Shape.hxx>
#include "TimelineDAG.h"
#include "AndroCadKernel.h"

namespace androcad {

struct IOResult {
    bool success = false;
    std::string errorMessage;
    std::string filePath;
    int entitiesProcessed = 0;
    double volume = 0.0;
};

struct ProjectMetadata {
    int formatVersion = 1;
    std::string appName = "AndroCAD";
    std::string appVersion = "0.1.0";
    std::string units = "mm";
    std::string schema = "androcad.parametric.project.v1";
};

class ProjectIO {
public:
    // Native .androcad Project File Serialization (JSON)
    static IOResult SaveProject(const TimelineDAG& dag, const std::string& filePath);
    static IOResult LoadProject(TimelineDAG& dag, const std::string& filePath);

    // STEP AP214 / AP242 Data Exchange
    static IOResult ExportSTEP(const TopoDS_Shape& shape,
                              const std::string& filePath,
                              const std::string& schema = "AP214");
    static IOResult ImportSTEP(const std::string& filePath, TopoDS_Shape& outShape);

    // STL (Stereolithography - Binary & ASCII) Data Exchange
    static IOResult ExportSTL(const TopoDS_Shape& shape,
                             const std::string& filePath,
                             double linearDeflection = 0.1,
                             double angularDeflection = 0.5,
                             bool binary = true);
    static IOResult ImportSTL(const std::string& filePath, TopoDS_Shape& outShape);

    // Helper: Compute volume for verification
    static double ComputeVolume(const TopoDS_Shape& shape);
};

} // namespace androcad
