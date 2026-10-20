// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "ProjectIO.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

#include <Standard_Failure.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

#include <STEPControl_Writer.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_StepModelType.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Interface_Static.hxx>

#include <StlAPI_Writer.hxx>
#include <StlAPI_Reader.hxx>

namespace androcad {

// Helper: Escape string for JSON
static std::string JsonEscape(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else o << c;
    }
    return o.str();
}

static std::string NodeTypeToString(FeatureNodeType type) {
    switch (type) {
        case FeatureNodeType::Box: return "Box";
        case FeatureNodeType::Sketch: return "Sketch";
        case FeatureNodeType::Extrude: return "Extrude";
        case FeatureNodeType::Revolve: return "Revolve";
        case FeatureNodeType::Fillet: return "Fillet";
        case FeatureNodeType::Chamfer: return "Chamfer";
        case FeatureNodeType::Hole: return "Hole";
        case FeatureNodeType::Shell: return "Shell";
        case FeatureNodeType::Combine: return "Combine";
    }
    return "Unknown";
}

static FeatureNodeType StringToNodeType(const std::string& str) {
    if (str == "Box") return FeatureNodeType::Box;
    if (str == "Sketch") return FeatureNodeType::Sketch;
    if (str == "Extrude") return FeatureNodeType::Extrude;
    if (str == "Revolve") return FeatureNodeType::Revolve;
    if (str == "Fillet") return FeatureNodeType::Fillet;
    if (str == "Chamfer") return FeatureNodeType::Chamfer;
    if (str == "Hole") return FeatureNodeType::Hole;
    if (str == "Shell") return FeatureNodeType::Shell;
    if (str == "Combine") return FeatureNodeType::Combine;
    return FeatureNodeType::Box;
}

double ProjectIO::ComputeVolume(const TopoDS_Shape& shape) {
    if (shape.IsNull()) return 0.0;
    try {
        GProp_GProps gprops;
        BRepGProp::VolumeProperties(shape, gprops);
        return gprops.Mass();
    } catch (...) {
        return 0.0;
    }
}

// -------------------------------------------------------------
// STEP Import / Export
// -------------------------------------------------------------

IOResult ProjectIO::ExportSTEP(const TopoDS_Shape& shape,
                              const std::string& filePath,
                              const std::string& schema) {
    IOResult result;
    result.filePath = filePath;

    if (shape.IsNull()) {
        result.success = false;
        result.errorMessage = "ExportSTEP failed: Input shape is null.";
        return result;
    }

    try {
        STEPControl_Writer writer;

        if (schema == "AP242") {
            Interface_Static::SetCVal("write.step.schema", "AP242DIS");
        } else {
            Interface_Static::SetCVal("write.step.schema", "AP214");
        }
        Interface_Static::SetCVal("write.step.unit", "MM");

        IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);
        if (status != IFSelect_RetDone) {
            result.success = false;
            result.errorMessage = "ExportSTEP failed during shape transfer (status: " + std::to_string(status) + ").";
            return result;
        }

        status = writer.Write(filePath.c_str());
        if (status != IFSelect_RetDone) {
            result.success = false;
            result.errorMessage = "ExportSTEP failed writing to file '" + filePath + "' (status: " + std::to_string(status) + ").";
            return result;
        }

        result.success = true;
        result.volume = ComputeVolume(shape);
        return result;
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = std::string("OCCT Standard_Failure in ExportSTEP: ") + e.GetMessageString();
        return result;
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = std::string("std::exception in ExportSTEP: ") + e.what();
        return result;
    }
}

IOResult ProjectIO::ImportSTEP(const std::string& filePath, TopoDS_Shape& outShape) {
    IOResult result;
    result.filePath = filePath;

    // Check if file exists
    std::ifstream checkFile(filePath);
    if (!checkFile.good()) {
        result.success = false;
        result.errorMessage = "ImportSTEP failed: File '" + filePath + "' cannot be opened or does not exist.";
        return result;
    }
    checkFile.close();

    try {
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
        if (status != IFSelect_RetDone) {
            result.success = false;
            result.errorMessage = "ImportSTEP failed: ReadFile returned error status " + std::to_string(status);
            return result;
        }

        reader.TransferRoots();
        outShape = reader.OneShape();

        if (outShape.IsNull()) {
            result.success = false;
            result.errorMessage = "ImportSTEP failed: Imported shape is null or empty.";
            return result;
        }

        result.success = true;
        result.volume = ComputeVolume(outShape);
        return result;
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = std::string("OCCT Standard_Failure in ImportSTEP: ") + e.GetMessageString();
        return result;
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = std::string("std::exception in ImportSTEP: ") + e.what();
        return result;
    }
}

// -------------------------------------------------------------
// STL Import / Export
// -------------------------------------------------------------

IOResult ProjectIO::ExportSTL(const TopoDS_Shape& shape,
                             const std::string& filePath,
                             double linearDeflection,
                             double angularDeflection,
                             bool binary) {
    IOResult result;
    result.filePath = filePath;

    if (shape.IsNull()) {
        result.success = false;
        result.errorMessage = "ExportSTL failed: Input shape is null.";
        return result;
    }

    try {
        // Ensure tessellation mesh exists on shape
        BRepMesh_IncrementalMesh meshGen(shape, linearDeflection, Standard_False, angularDeflection);
        meshGen.Perform();

        StlAPI_Writer writer;
        writer.ASCIIMode() = binary ? Standard_False : Standard_True;

        Standard_Boolean ok = writer.Write(shape, filePath.c_str());
        if (!ok) {
            result.success = false;
            result.errorMessage = "ExportSTL failed writing to file '" + filePath + "'.";
            return result;
        }

        result.success = true;
        result.volume = ComputeVolume(shape);
        return result;
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = std::string("OCCT Standard_Failure in ExportSTL: ") + e.GetMessageString();
        return result;
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = std::string("std::exception in ExportSTL: ") + e.what();
        return result;
    }
}

IOResult ProjectIO::ImportSTL(const std::string& filePath, TopoDS_Shape& outShape) {
    IOResult result;
    result.filePath = filePath;

    std::ifstream checkFile(filePath);
    if (!checkFile.good()) {
        result.success = false;
        result.errorMessage = "ImportSTL failed: File '" + filePath + "' cannot be opened or does not exist.";
        return result;
    }
    checkFile.close();

    try {
        StlAPI_Reader reader;
        Standard_Boolean ok = reader.Read(outShape, filePath.c_str());
        if (!ok || outShape.IsNull()) {
            result.success = false;
            result.errorMessage = "ImportSTL failed: Could not read mesh from '" + filePath + "'.";
            return result;
        }

        result.success = true;
        result.volume = ComputeVolume(outShape);
        return result;
    } catch (const Standard_Failure& e) {
        result.success = false;
        result.errorMessage = std::string("OCCT Standard_Failure in ImportSTL: ") + e.GetMessageString();
        return result;
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = std::string("std::exception in ImportSTL: ") + e.what();
        return result;
    }
}

// -------------------------------------------------------------
// Native Project File (.androcad JSON)
// -------------------------------------------------------------

IOResult ProjectIO::SaveProject(const TimelineDAG& dag, const std::string& filePath) {
    IOResult result;
    result.filePath = filePath;

    std::ofstream out(filePath);
    if (!out.is_open()) {
        result.success = false;
        result.errorMessage = "Failed to open file for writing: " + filePath;
        return result;
    }

    ProjectMetadata meta;

    out << "{\n";
    out << "  \"metadata\": {\n";
    out << "    \"format_version\": " << meta.formatVersion << ",\n";
    out << "    \"app_name\": \"" << JsonEscape(meta.appName) << "\",\n";
    out << "    \"app_version\": \"" << JsonEscape(meta.appVersion) << "\",\n";
    out << "    \"units\": \"" << JsonEscape(meta.units) << "\",\n";
    out << "    \"schema\": \"" << JsonEscape(meta.schema) << "\"\n";
    out << "  },\n";

    // User Parameters
    out << "  \"parameters\": {\n";
    const auto& params = dag.GetAllParameters();
    size_t paramCount = 0;
    for (const auto& kv : params) {
        out << "    \"" << JsonEscape(kv.first) << "\": \"" << JsonEscape(kv.second) << "\"";
        if (++paramCount < params.size()) out << ",";
        out << "\n";
    }
    out << "  },\n";

    // Rollback Marker
    out << "  \"rollback_index\": " << dag.GetRollbackIndex() << ",\n";

    // Feature Nodes
    out << "  \"nodes\": [\n";
    const auto& nodes = dag.GetNodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        out << "    {\n";
        out << "      \"id\": " << node.id << ",\n";
        out << "      \"name\": \"" << JsonEscape(node.name) << "\",\n";
        out << "      \"type\": \"" << NodeTypeToString(node.type) << "\",\n";

        // Param Expressions
        out << "      \"expressions\": {\n";
        size_t expCount = 0;
        for (const auto& kv : node.paramExpressions) {
            out << "        \"" << JsonEscape(kv.first) << "\": \"" << JsonEscape(kv.second) << "\"";
            if (++expCount < node.paramExpressions.size()) out << ",";
            out << "\n";
        }
        out << "      },\n";

        // References
        out << "      \"references\": {\n";
        size_t refCount = 0;
        for (const auto& kv : node.references) {
            const auto& r = kv.second;
            out << "        \"" << JsonEscape(kv.first) << "\": {\n";
            out << "          \"semanticTag\": \"" << JsonEscape(r.semanticTag) << "\",\n";
            out << "          \"lastKnownLocalId\": " << r.lastKnownLocalId << ",\n";
            out << "          \"fp_type\": " << static_cast<int>(r.fingerprint.type) << ",\n";
            out << "          \"fp_geomType\": " << r.fingerprint.geomType << ",\n";
            out << "          \"fp_cx\": " << r.fingerprint.center[0] << ",\n";
            out << "          \"fp_cy\": " << r.fingerprint.center[1] << ",\n";
            out << "          \"fp_cz\": " << r.fingerprint.center[2] << ",\n";
            out << "          \"fp_nx\": " << r.fingerprint.normal[0] << ",\n";
            out << "          \"fp_ny\": " << r.fingerprint.normal[1] << ",\n";
            out << "          \"fp_nz\": " << r.fingerprint.normal[2] << ",\n";
            out << "          \"fp_size\": " << r.fingerprint.size << ",\n";
            out << "          \"fp_subCount\": " << r.fingerprint.subEntityCount << "\n";
            out << "        }";
            if (++refCount < node.references.size()) out << ",";
            out << "\n";
        }
        out << "      }\n";
        out << "    }";
        if (i + 1 < nodes.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";

    out.close();
    result.success = true;
    result.entitiesProcessed = static_cast<int>(nodes.size());
    return result;
}

// -------------------------------------------------------------
// Lightweight JSON Parser for .androcad Project Files
// -------------------------------------------------------------

static void SkipWhitespace(const std::string& s, size_t& pos) {
    while (pos < s.size() && (std::isspace(s[pos]) || s[pos] == ',' || s[pos] == ':')) {
        pos++;
    }
}

static std::string ParseJsonString(const std::string& s, size_t& pos) {
    SkipWhitespace(s, pos);
    if (pos >= s.size() || s[pos] != '"') return "";
    pos++; // skip opening quote
    std::string res;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\' && pos + 1 < s.size()) {
            pos++;
            switch (s[pos]) {
                case '"': res += '"'; break;
                case '\\': res += '\\'; break;
                case 'n': res += '\n'; break;
                case 'r': res += '\r'; break;
                case 't': res += '\t'; break;
                default: res += s[pos]; break;
            }
        } else {
            res += s[pos];
        }
        pos++;
    }
    if (pos < s.size() && s[pos] == '"') pos++; // skip closing quote
    return res;
}

static double ParseJsonNumber(const std::string& s, size_t& pos) {
    SkipWhitespace(s, pos);
    size_t start = pos;
    if (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) pos++;
    while (pos < s.size() && (std::isdigit(s[pos]) || s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E' || s[pos] == '+' || s[pos] == '-')) {
        pos++;
    }
    std::string numStr = s.substr(start, pos - start);
    try {
        return std::stod(numStr);
    } catch (...) {
        return 0.0;
    }
}

IOResult ProjectIO::LoadProject(TimelineDAG& dag, const std::string& filePath) {
    IOResult result;
    result.filePath = filePath;

    std::ifstream in(filePath);
    if (!in.is_open()) {
        result.success = false;
        result.errorMessage = "Failed to open project file: " + filePath;
        return result;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string json = buffer.str();
    in.close();

    if (json.empty()) {
        result.success = false;
        result.errorMessage = "Project file is empty: " + filePath;
        return result;
    }

    dag.Clear();

    // Parse parameters
    size_t pParams = json.find("\"parameters\"");
    if (pParams != std::string::npos) {
        size_t braceOpen = json.find('{', pParams);
        size_t braceClose = json.find('}', braceOpen);
        if (braceOpen != std::string::npos && braceClose != std::string::npos) {
            size_t cursor = braceOpen + 1;
            while (cursor < braceClose) {
                std::string key = ParseJsonString(json, cursor);
                if (key.empty()) break;
                std::string val = ParseJsonString(json, cursor);
                dag.SetParameter(key, val);
            }
        }
    }

    // Parse rollback_index
    size_t pRollback = json.find("\"rollback_index\"");
    if (pRollback != std::string::npos) {
        size_t colon = json.find(':', pRollback);
        if (colon != std::string::npos) {
            size_t cursor = colon + 1;
            int rIdx = static_cast<int>(ParseJsonNumber(json, cursor));
            dag.SetRollbackIndex(rIdx);
        }
    }

    // Parse nodes array
    size_t pNodes = json.find("\"nodes\"");
    if (pNodes != std::string::npos) {
        size_t arrayOpen = json.find('[', pNodes);
        size_t arrayClose = json.rfind(']');
        if (arrayOpen != std::string::npos && arrayClose != std::string::npos) {
            size_t cursor = arrayOpen + 1;
            while (cursor < arrayClose) {
                size_t nodeStart = json.find('{', cursor);
                if (nodeStart == std::string::npos || nodeStart >= arrayClose) break;

                // Find matching closing brace for this node
                size_t nodeEnd = nodeStart + 1;
                int braceDepth = 1;
                while (nodeEnd < arrayClose && braceDepth > 0) {
                    if (json[nodeEnd] == '{') braceDepth++;
                    else if (json[nodeEnd] == '}') braceDepth--;
                    nodeEnd++;
                }

                std::string nodeJson = json.substr(nodeStart, nodeEnd - nodeStart);
                cursor = nodeEnd;

                // Extract node fields
                FeatureNode node;
                size_t cur = 0;

                size_t pId = nodeJson.find("\"id\"");
                if (pId != std::string::npos) {
                    size_t c = nodeJson.find(':', pId) + 1;
                    node.id = static_cast<uint32_t>(ParseJsonNumber(nodeJson, c));
                }

                size_t pName = nodeJson.find("\"name\"");
                if (pName != std::string::npos) {
                    size_t c = nodeJson.find(':', pName) + 1;
                    node.name = ParseJsonString(nodeJson, c);
                }

                size_t pType = nodeJson.find("\"type\"");
                if (pType != std::string::npos) {
                    size_t c = nodeJson.find(':', pType) + 1;
                    std::string typeStr = ParseJsonString(nodeJson, c);
                    node.type = StringToNodeType(typeStr);
                }

                // Extract expressions
                size_t pExp = nodeJson.find("\"expressions\"");
                if (pExp != std::string::npos) {
                    size_t expOpen = nodeJson.find('{', pExp);
                    size_t expClose = nodeJson.find('}', expOpen);
                    if (expOpen != std::string::npos && expClose != std::string::npos) {
                        size_t c = expOpen + 1;
                        while (c < expClose) {
                            std::string eKey = ParseJsonString(nodeJson, c);
                            if (eKey.empty()) break;
                            std::string eVal = ParseJsonString(nodeJson, c);
                            node.paramExpressions[eKey] = eVal;
                        }
                    }
                }

                // Extract references
                size_t pRef = nodeJson.find("\"references\"");
                if (pRef != std::string::npos) {
                    size_t refOpen = nodeJson.find('{', pRef);
                    size_t refClose = nodeJson.rfind('}');
                    if (refOpen != std::string::npos && refClose != std::string::npos && refClose > refOpen) {
                        size_t c = refOpen + 1;
                        while (c < refClose) {
                            std::string refKey = ParseJsonString(nodeJson, c);
                            if (refKey.empty()) break;

                            size_t objOpen = nodeJson.find('{', c);
                            size_t objClose = nodeJson.find('}', objOpen);
                            if (objOpen == std::string::npos || objClose == std::string::npos) break;

                            std::string refObj = nodeJson.substr(objOpen, objClose - objOpen + 1);
                            c = objClose + 1;

                            TopologicalReference tr;
                            size_t sTag = refObj.find("\"semanticTag\"");
                            if (sTag != std::string::npos) {
                                size_t sc = refObj.find(':', sTag) + 1;
                                tr.semanticTag = ParseJsonString(refObj, sc);
                            }
                            size_t sId = refObj.find("\"lastKnownLocalId\"");
                            if (sId != std::string::npos) {
                                size_t sc = refObj.find(':', sId) + 1;
                                tr.lastKnownLocalId = static_cast<uint32_t>(ParseJsonNumber(refObj, sc));
                            }
                            size_t sType = refObj.find("\"fp_type\"");
                            if (sType != std::string::npos) {
                                size_t sc = refObj.find(':', sType) + 1;
                                tr.fingerprint.type = static_cast<SubShapeType>(static_cast<int>(ParseJsonNumber(refObj, sc)));
                            }
                            size_t sGeom = refObj.find("\"fp_geomType\"");
                            if (sGeom != std::string::npos) {
                                size_t sc = refObj.find(':', sGeom) + 1;
                                tr.fingerprint.geomType = static_cast<int>(ParseJsonNumber(refObj, sc));
                            }
                            size_t sCx = refObj.find("\"fp_cx\"");
                            if (sCx != std::string::npos) {
                                size_t sc = refObj.find(':', sCx) + 1;
                                tr.fingerprint.center[0] = ParseJsonNumber(refObj, sc);
                            }
                            size_t sCy = refObj.find("\"fp_cy\"");
                            if (sCy != std::string::npos) {
                                size_t sc = refObj.find(':', sCy) + 1;
                                tr.fingerprint.center[1] = ParseJsonNumber(refObj, sc);
                            }
                            size_t sCz = refObj.find("\"fp_cz\"");
                            if (sCz != std::string::npos) {
                                size_t sc = refObj.find(':', sCz) + 1;
                                tr.fingerprint.center[2] = ParseJsonNumber(refObj, sc);
                            }
                            size_t sNx = refObj.find("\"fp_nx\"");
                            if (sNx != std::string::npos) {
                                size_t sc = refObj.find(':', sNx) + 1;
                                tr.fingerprint.normal[0] = ParseJsonNumber(refObj, sc);
                            }
                            size_t sNy = refObj.find("\"fp_ny\"");
                            if (sNy != std::string::npos) {
                                size_t sc = refObj.find(':', sNy) + 1;
                                tr.fingerprint.normal[1] = ParseJsonNumber(refObj, sc);
                            }
                            size_t sNz = refObj.find("\"fp_nz\"");
                            if (sNz != std::string::npos) {
                                size_t sc = refObj.find(':', sNz) + 1;
                                tr.fingerprint.normal[2] = ParseJsonNumber(refObj, sc);
                            }
                            size_t sSize = refObj.find("\"fp_size\"");
                            if (sSize != std::string::npos) {
                                size_t sc = refObj.find(':', sSize) + 1;
                                tr.fingerprint.size = ParseJsonNumber(refObj, sc);
                            }
                            size_t sSub = refObj.find("\"fp_subCount\"");
                            if (sSub != std::string::npos) {
                                size_t sc = refObj.find(':', sSub) + 1;
                                tr.fingerprint.subEntityCount = static_cast<int>(ParseJsonNumber(refObj, sc));
                            }

                            node.references[refKey] = tr;
                        }
                    }
                }

                dag.AddNode(node);
            }
        }
    }

    bool recomputed = dag.Rebuild();
    result.success = recomputed;
    if (!recomputed) {
        result.errorMessage = "Project loaded but DAG recomputation failed: " + dag.GetRebuildLog();
    }
    result.entitiesProcessed = static_cast<int>(dag.GetNodes().size());
    result.volume = ComputeVolume(dag.GetActiveShape());
    return result;
}

} // namespace androcad
