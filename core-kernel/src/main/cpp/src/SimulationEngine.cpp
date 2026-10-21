// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "SimulationEngine.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <TopAbs_State.hxx>
#include <gp_Pnt.hxx>

#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

namespace androcad {

namespace {

// Invert a 4x4 matrix using cofactors
bool Invert4x4(const double m[16], double inv[16]) {
    double invOut[16];

    invOut[0] = m[5]  * m[10] * m[15] - 
                m[5]  * m[11] * m[14] - 
                m[9]  * m[6]  * m[15] + 
                m[9]  * m[7]  * m[14] +
                m[13] * m[6]  * m[11] - 
                m[13] * m[7]  * m[10];

    invOut[4] = -m[4]  * m[10] * m[15] + 
                 m[4]  * m[11] * m[14] + 
                 m[8]  * m[6]  * m[15] - 
                 m[8]  * m[7]  * m[14] - 
                 m[12] * m[6]  * m[11] + 
                 m[12] * m[7]  * m[10];

    invOut[8] = m[4]  * m[9] * m[15] - 
                m[4]  * m[11] * m[13] - 
                m[8]  * m[5] * m[15] + 
                m[8]  * m[7] * m[13] + 
                m[12] * m[5] * m[11] - 
                m[12] * m[7] * m[9];

    invOut[12] = -m[4]  * m[9] * m[14] + 
                  m[4]  * m[10] * m[13] +
                  m[8]  * m[5] * m[14] - 
                  m[8]  * m[6] * m[13] - 
                  m[12] * m[5] * m[10] + 
                  m[12] * m[6] * m[9];

    invOut[1] = -m[1]  * m[10] * m[15] + 
                 m[1]  * m[11] * m[14] + 
                 m[9]  * m[2] * m[15] - 
                 m[9]  * m[3] * m[14] - 
                 m[13] * m[2] * m[11] + 
                 m[13] * m[3] * m[10];

    invOut[5] = m[0]  * m[10] * m[15] - 
                m[0]  * m[11] * m[14] - 
                m[8]  * m[2] * m[15] + 
                m[8]  * m[3] * m[14] + 
                m[12] * m[2] * m[11] - 
                m[12] * m[3] * m[10];

    invOut[9] = -m[0]  * m[9] * m[15] + 
                 m[0]  * m[11] * m[13] + 
                 m[8]  * m[1] * m[15] - 
                 m[8]  * m[3] * m[13] - 
                 m[12] * m[1] * m[11] + 
                 m[12] * m[3] * m[9];

    invOut[13] = m[0]  * m[9] * m[14] - 
                 m[0]  * m[10] * m[13] - 
                 m[8]  * m[1] * m[14] + 
                 m[8]  * m[2] * m[13] + 
                 m[12] * m[1] * m[10] - 
                 m[12] * m[2] * m[9];

    invOut[2] = m[1]  * m[6] * m[15] - 
                m[1]  * m[7] * m[14] - 
                m[5]  * m[2] * m[15] + 
                m[5]  * m[3] * m[14] + 
                m[13] * m[2] * m[7] - 
                m[13] * m[3] * m[6];

    invOut[6] = -m[0]  * m[6] * m[15] + 
                 m[0]  * m[7] * m[14] + 
                 m[4]  * m[2] * m[15] - 
                 m[4]  * m[3] * m[14] - 
                 m[12] * m[2] * m[7] + 
                 m[12] * m[3] * m[6];

    invOut[10] = m[0]  * m[5] * m[15] - 
                 m[0]  * m[7] * m[13] - 
                 m[4]  * m[1] * m[15] + 
                 m[4]  * m[3] * m[13] + 
                 m[12] * m[1] * m[7] - 
                 m[12] * m[3] * m[5];

    invOut[14] = -m[0]  * m[5] * m[14] + 
                  m[0]  * m[6] * m[13] + 
                  m[4]  * m[1] * m[14] - 
                  m[4]  * m[2] * m[13] - 
                  m[12] * m[1] * m[6] + 
                  m[12] * m[2] * m[5];

    invOut[3] = -m[1] * m[6] * m[11] + 
                 m[1] * m[7] * m[10] + 
                 m[5] * m[2] * m[11] - 
                 m[5] * m[3] * m[10] - 
                 m[9] * m[2] * m[7] + 
                 m[9] * m[3] * m[6];

    invOut[7] = m[0] * m[6] * m[11] - 
                m[0] * m[7] * m[10] - 
                m[4] * m[2] * m[11] + 
                m[4] * m[3] * m[10] + 
                m[8] * m[2] * m[7] - 
                m[8] * m[3] * m[6];

    invOut[11] = -m[0] * m[5] * m[11] + 
                  m[0] * m[7] * m[9] + 
                  m[4] * m[1] * m[11] - 
                  m[4] * m[3] * m[9] - 
                  m[8] * m[1] * m[7] + 
                  m[8] * m[3] * m[5];

    invOut[15] = m[0] * m[5] * m[10] - 
                 m[0] * m[6] * m[9] - 
                 m[4] * m[1] * m[10] + 
                 m[4] * m[2] * m[9] + 
                 m[8] * m[1] * m[6] - 
                 m[8] * m[2] * m[5];

    double det = m[0] * invOut[0] + m[1] * invOut[4] + m[2] * invOut[8] + m[3] * invOut[12];

    if (std::abs(det) < 1e-25) {
        return false;
    }

    double invDet = 1.0 / det;
    for (int i = 0; i < 16; i++) {
        inv[i] = invOut[i] * invDet;
    }
    return true;
}

// Convert normalized stress t in [0, 1] to Turbo/Rainbow RGB
void GetStressColor(double t, float& r, float& g, float& b) {
    t = std::max(0.0, std::min(1.0, t));
    if (t < 0.25) {
        double f = t / 0.25;
        r = 0.0f;
        g = static_cast<float>(f);
        b = 1.0f;
    } else if (t < 0.50) {
        double f = (t - 0.25) / 0.25;
        r = 0.0f;
        g = 1.0f;
        b = static_cast<float>(1.0 - f);
    } else if (t < 0.75) {
        double f = (t - 0.50) / 0.25;
        r = static_cast<float>(f);
        g = 1.0f;
        b = 0.0f;
    } else {
        double f = (t - 0.75) / 0.25;
        r = 1.0f;
        g = static_cast<float>(1.0 - f);
        b = 0.0f;
    }
}

struct TriangleKey {
    uint32_t v[3];
    bool operator==(const TriangleKey& o) const {
        return v[0] == o.v[0] && v[1] == o.v[1] && v[2] == o.v[2];
    }
};

struct TriangleKeyHash {
    std::size_t operator()(const TriangleKey& k) const {
        std::size_t h1 = std::hash<uint32_t>()(k.v[0]);
        std::size_t h2 = std::hash<uint32_t>()(k.v[1]);
        std::size_t h3 = std::hash<uint32_t>()(k.v[2]);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

TriangleKey MakeKey(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t arr[3] = {a, b, c};
    std::sort(arr, arr + 3);
    return TriangleKey{{arr[0], arr[1], arr[2]}};
}

} // namespace

SimulationEngine::SimulationEngine() = default;

void SimulationEngine::Clear() {
    nodes_.clear();
    elements_.clear();
    constraints_.clear();
    loads_.clear();
}

void SimulationEngine::SetMaterial(const Material& material) {
    youngModulusPa_ = material.youngModulusGPa * 1.0e9;
    poissonRatio_ = material.poissonRatio;
    yieldStrengthPa_ = material.yieldStrengthMPa * 1.0e6;
}

void SimulationEngine::SetMaterialProperties(double youngModulusGPa, double poissonRatio, double yieldStrengthMPa) {
    youngModulusPa_ = youngModulusGPa * 1.0e9;
    poissonRatio_ = poissonRatio;
    yieldStrengthPa_ = yieldStrengthMPa * 1.0e6;
}

void SimulationEngine::AddConstraint(const BoundaryConstraint& constraint) {
    constraints_.push_back(constraint);
}

void SimulationEngine::AddLoad(const StructuralLoad& load) {
    loads_.push_back(load);
}

void SimulationEngine::GenerateBeamMesh(double lengthMm, double widthMm, double heightMm,
                                       int nx, int ny, int nz) {
    Clear();

    double dx = (lengthMm * 1e-3) / nx;
    double dy = (widthMm * 1e-3) / ny;
    double dz = (heightMm * 1e-3) / nz;

    double y0 = -(widthMm * 1e-3) * 0.5;
    double z0 = -(heightMm * 1e-3) * 0.5;

    nodes_.reserve((nx + 1) * (ny + 1) * (nz + 1));

    auto nodeIndex = [ny, nz](int i, int j, int k) -> uint32_t {
        return static_cast<uint32_t>(i * (ny + 1) * (nz + 1) + j * (nz + 1) + k);
    };

    for (int i = 0; i <= nx; ++i) {
        double x = i * dx;
        for (int j = 0; j <= ny; ++j) {
            double y = y0 + j * dy;
            for (int k = 0; k <= nz; ++k) {
                double z = z0 + k * dz;
                FEANode node;
                node.id = static_cast<uint32_t>(nodes_.size());
                node.x = x;
                node.y = y;
                node.z = z;
                nodes_.push_back(node);
            }
        }
    }

    elements_.reserve(nx * ny * nz * 5);

    uint32_t elemId = 0;
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            for (int k = 0; k < nz; ++k) {
                uint32_t v0 = nodeIndex(i,   j,   k);
                uint32_t v1 = nodeIndex(i+1, j,   k);
                uint32_t v2 = nodeIndex(i+1, j+1, k);
                uint32_t v3 = nodeIndex(i,   j+1, k);
                uint32_t v4 = nodeIndex(i,   j,   k+1);
                uint32_t v5 = nodeIndex(i+1, j,   k+1);
                uint32_t v6 = nodeIndex(i+1, j+1, k+1);
                uint32_t v7 = nodeIndex(i,   j+1, k+1);

                // Alternating Kuhn/Freudenthal 5-tetrahedron decomposition
                if ((i + j + k) % 2 == 0) {
                    uint32_t tets[5][4] = {
                        {v0, v1, v3, v4},
                        {v1, v2, v3, v6},
                        {v1, v4, v5, v6},
                        {v3, v4, v6, v7},
                        {v1, v3, v4, v6}
                    };
                    for (int t = 0; t < 5; ++t) {
                        FEATet4 el;
                        el.id = elemId++;
                        el.nodes[0] = tets[t][0];
                        el.nodes[1] = tets[t][1];
                        el.nodes[2] = tets[t][2];
                        el.nodes[3] = tets[t][3];
                        elements_.push_back(el);
                    }
                } else {
                    uint32_t tets[5][4] = {
                        {v1, v2, v0, v5},
                        {v0, v3, v2, v7},
                        {v0, v4, v5, v7},
                        {v2, v5, v6, v7},
                        {v0, v2, v5, v7}
                    };
                    for (int t = 0; t < 5; ++t) {
                        FEATet4 el;
                        el.id = elemId++;
                        el.nodes[0] = tets[t][0];
                        el.nodes[1] = tets[t][1];
                        el.nodes[2] = tets[t][2];
                        el.nodes[3] = tets[t][3];
                        elements_.push_back(el);
                    }
                }
            }
        }
    }
}

void SimulationEngine::DiscretizeShape(const TopoDS_Shape& shape, double targetElementSizeMm) {
    Clear();
    if (shape.IsNull()) return;

    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    box.SetGap(1e-4);

    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    double lx = xmax - xmin;
    double ly = ymax - ymin;
    double lz = zmax - zmin;

    int nx = std::max(2, static_cast<int>(std::ceil(lx / targetElementSizeMm)));
    int ny = std::max(2, static_cast<int>(std::ceil(ly / targetElementSizeMm)));
    int nz = std::max(2, static_cast<int>(std::ceil(lz / targetElementSizeMm)));

    double dx = (lx * 1e-3) / nx;
    double dy = (ly * 1e-3) / ny;
    double dz = (lz * 1e-3) / nz;

    double x0 = xmin * 1e-3;
    double y0 = ymin * 1e-3;
    double z0 = zmin * 1e-3;

    BRepClass3d_SolidClassifier classifier(shape);

    // Map grid point (i,j,k) to node id
    std::unordered_map<uint64_t, uint32_t> gridToNode;
    auto makeGridKey = [](int i, int j, int k) -> uint64_t {
        return (static_cast<uint64_t>(i) & 0x1FFFFF) |
               ((static_cast<uint64_t>(j) & 0x1FFFFF) << 21) |
               ((static_cast<uint64_t>(k) & 0x1FFFFF) << 42);
    };

    auto getNode = [&](int i, int j, int k) -> uint32_t {
        uint64_t key = makeGridKey(i, j, k);
        auto it = gridToNode.find(key);
        if (it != gridToNode.end()) return it->second;

        FEANode node;
        node.id = static_cast<uint32_t>(nodes_.size());
        node.x = x0 + i * dx;
        node.y = y0 + j * dy;
        node.z = z0 + k * dz;
        nodes_.push_back(node);
        gridToNode[key] = node.id;
        return node.id;
    };

    uint32_t elemId = 0;
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            for (int k = 0; k < nz; ++k) {
                // Check cell center
                gp_Pnt centerMm((x0 + (i + 0.5) * dx) * 1e3,
                                (y0 + (j + 0.5) * dy) * 1e3,
                                (z0 + (k + 0.5) * dz) * 1e3);

                classifier.Perform(centerMm, 1e-4);
                TopAbs_State state = classifier.State();
                if (state != TopAbs_IN && state != TopAbs_ON) continue;

                uint32_t v0 = getNode(i,   j,   k);
                uint32_t v1 = getNode(i+1, j,   k);
                uint32_t v2 = getNode(i+1, j+1, k);
                uint32_t v3 = getNode(i,   j+1, k);
                uint32_t v4 = getNode(i,   j,   k+1);
                uint32_t v5 = getNode(i+1, j,   k+1);
                uint32_t v6 = getNode(i+1, j+1, k+1);
                uint32_t v7 = getNode(i,   j+1, k+1);

                if ((i + j + k) % 2 == 0) {
                    uint32_t tets[5][4] = {
                        {v0, v1, v3, v4},
                        {v1, v2, v3, v6},
                        {v1, v4, v5, v6},
                        {v3, v4, v6, v7},
                        {v1, v3, v4, v6}
                    };
                    for (int t = 0; t < 5; ++t) {
                        FEATet4 el;
                        el.id = elemId++;
                        el.nodes[0] = tets[t][0];
                        el.nodes[1] = tets[t][1];
                        el.nodes[2] = tets[t][2];
                        el.nodes[3] = tets[t][3];
                        elements_.push_back(el);
                    }
                } else {
                    uint32_t tets[5][4] = {
                        {v1, v2, v0, v5},
                        {v0, v3, v2, v7},
                        {v0, v4, v5, v7},
                        {v2, v5, v6, v7},
                        {v0, v2, v5, v7}
                    };
                    for (int t = 0; t < 5; ++t) {
                        FEATet4 el;
                        el.id = elemId++;
                        el.nodes[0] = tets[t][0];
                        el.nodes[1] = tets[t][1];
                        el.nodes[2] = tets[t][2];
                        el.nodes[3] = tets[t][3];
                        elements_.push_back(el);
                    }
                }
            }
        }
    }
}

void SimulationEngine::ApplyBoundaryConditions(std::vector<double>& F,
                                              std::vector<std::vector<std::pair<int, double>>>& K_rows) {
    size_t numNodes = nodes_.size();

    // 1. Mark fixed DOFs
    for (auto& node : nodes_) {
        double xm = node.x * 1e3;
        double ym = node.y * 1e3;
        double zm = node.z * 1e3;

        for (const auto& bc : constraints_) {
            if (xm >= bc.minX && xm <= bc.maxX &&
                ym >= bc.minY && ym <= bc.maxY &&
                zm >= bc.minZ && zm <= bc.maxZ) {
                switch (bc.type) {
                    case ConstraintType::FIXED:
                    case ConstraintType::PIN:
                        node.isFixedX = true;
                        node.isFixedY = true;
                        node.isFixedZ = true;
                        break;
                    case ConstraintType::ROLLER_X:
                        node.isFixedX = true;
                        break;
                    case ConstraintType::ROLLER_Y:
                        node.isFixedY = true;
                        break;
                    case ConstraintType::ROLLER_Z:
                        node.isFixedZ = true;
                        break;
                }
            }
        }
    }

    // 2. Assemble external load vector
    for (const auto& load : loads_) {
        std::vector<uint32_t> matchedNodes;
        for (const auto& node : nodes_) {
            double xm = node.x * 1e3;
            double ym = node.y * 1e3;
            double zm = node.z * 1e3;

            if (xm >= load.minX && xm <= load.maxX &&
                ym >= load.minY && ym <= load.maxY &&
                zm >= load.minZ && zm <= load.maxZ) {
                matchedNodes.push_back(node.id);
            }
        }

        if (!matchedNodes.empty()) {
            double fxPerNode = load.forceX / matchedNodes.size();
            double fyPerNode = load.forceY / matchedNodes.size();
            double fzPerNode = load.forceZ / matchedNodes.size();

            for (uint32_t nid : matchedNodes) {
                F[3 * nid + 0] += fxPerNode;
                F[3 * nid + 1] += fyPerNode;
                F[3 * nid + 2] += fzPerNode;
            }
        }
    }

    // 3. Apply Dirichlet BCs to stiffness matrix and force vector
    for (size_t nid = 0; nid < numNodes; ++nid) {
        const auto& node = nodes_[nid];
        bool fixed[3] = {node.isFixedX, node.isFixedY, node.isFixedZ};

        for (int d = 0; d < 3; ++d) {
            if (fixed[d]) {
                int dof = static_cast<int>(3 * nid + d);
                // Clear row dof
                K_rows[dof].clear();
                K_rows[dof].emplace_back(dof, 1.0);
                F[dof] = 0.0;
            }
        }
    }

    // Zero out symmetric column entries for fixed DOFs
    for (size_t row = 0; row < K_rows.size(); ++row) {
        auto& rowEntries = K_rows[row];
        for (auto& entry : rowEntries) {
            int col = entry.first;
            if (col != static_cast<int>(row)) {
                uint32_t colNode = static_cast<uint32_t>(col / 3);
                int colDof = col % 3;
                bool isFixed = (colDof == 0 && nodes_[colNode].isFixedX) ||
                               (colDof == 1 && nodes_[colNode].isFixedY) ||
                               (colDof == 2 && nodes_[colNode].isFixedZ);
                if (isFixed) {
                    entry.second = 0.0;
                }
            }
        }
    }
}

SimulationResult SimulationEngine::Solve(double deformationScale) {
    SimulationResult result;
    if (nodes_.empty() || elements_.empty()) {
        result.success = false;
        result.errorMessage = "Simulation mesh is empty";
        return result;
    }

    size_t numNodes = nodes_.size();
    size_t numDOFs = numNodes * 3;

    // Lamé elastic constants
    double lambda = (youngModulusPa_ * poissonRatio_) / ((1.0 + poissonRatio_) * (1.0 - 2.0 * poissonRatio_));
    double mu = youngModulusPa_ / (2.0 * (1.0 + poissonRatio_));

    // Prepare sparse matrix row maps
    std::vector<std::unordered_map<int, double>> K_map(numDOFs);

    // Compute element stiffness matrices and assemble
    for (auto& el : elements_) {
        const auto& n0 = nodes_[el.nodes[0]];
        const auto& n1 = nodes_[el.nodes[1]];
        const auto& n2 = nodes_[el.nodes[2]];
        const auto& n3 = nodes_[el.nodes[3]];

        // Build 4x4 coordinate matrix M
        double M[16] = {
            1.0, n0.x, n0.y, n0.z,
            1.0, n1.x, n1.y, n1.z,
            1.0, n2.x, n2.y, n2.z,
            1.0, n3.x, n3.y, n3.z
        };

        // Check signed volume and adjust orientation if negative
        double v1x = n1.x - n0.x, v1y = n1.y - n0.y, v1z = n1.z - n0.z;
        double v2x = n2.x - n0.x, v2y = n2.y - n0.y, v2z = n2.z - n0.z;
        double v3x = n3.x - n0.x, v3y = n3.y - n0.y, v3z = n3.z - n0.z;

        double det = v1x * (v2y * v3z - v2z * v3y) -
                     v1y * (v2x * v3z - v2z * v3x) +
                     v1z * (v2x * v3y - v2y * v3x);

        if (det < 0.0) {
            std::swap(el.nodes[1], el.nodes[2]);
            // Re-swap in M
            M[4] = 1.0; M[5] = n2.x; M[6] = n2.y; M[7] = n2.z;
            M[8] = 1.0; M[9] = n1.x; M[10] = n1.y; M[11] = n1.z;
            det = -det;
        }

        double vol = det / 6.0;
        if (vol < 1e-18) continue;
        el.volumeM3 = vol;

        double invM[16];
        if (!Invert4x4(M, invM)) continue;

        // Shape function derivatives for 4 nodes: b_i = dN_i/dx, c_i = dN_i/dy, d_i = dN_i/dz
        double b[4], c[4], d[4];
        for (int i = 0; i < 4; ++i) {
            b[i] = invM[1 * 4 + i];
            c[i] = invM[2 * 4 + i];
            d[i] = invM[3 * 4 + i];
        }

        // 3x3 block sub-stiffness K_e(i, j)
        for (int i = 0; i < 4; ++i) {
            uint32_t ni = el.nodes[i];
            for (int j = 0; j < 4; ++j) {
                uint32_t nj = el.nodes[j];

                double k00 = vol * ((lambda + 2.0 * mu) * b[i] * b[j] + mu * (c[i] * c[j] + d[i] * d[j]));
                double k01 = vol * (lambda * b[i] * c[j] + mu * c[i] * b[j]);
                double k02 = vol * (lambda * b[i] * d[j] + mu * d[i] * b[j]);

                double k10 = vol * (lambda * c[i] * b[j] + mu * b[i] * c[j]);
                double k11 = vol * ((lambda + 2.0 * mu) * c[i] * c[j] + mu * (b[i] * b[j] + d[i] * d[j]));
                double k12 = vol * (lambda * c[i] * d[j] + mu * d[i] * c[j]);

                double k20 = vol * (lambda * d[i] * b[j] + mu * b[i] * d[j]);
                double k21 = vol * (lambda * d[i] * c[j] + mu * c[i] * d[j]);
                double k22 = vol * ((lambda + 2.0 * mu) * d[i] * d[j] + mu * (b[i] * b[j] + c[i] * c[j]));

                K_map[3 * ni + 0][3 * nj + 0] += k00;
                K_map[3 * ni + 0][3 * nj + 1] += k01;
                K_map[3 * ni + 0][3 * nj + 2] += k02;

                K_map[3 * ni + 1][3 * nj + 0] += k10;
                K_map[3 * ni + 1][3 * nj + 1] += k11;
                K_map[3 * ni + 1][3 * nj + 2] += k12;

                K_map[3 * ni + 2][3 * nj + 0] += k20;
                K_map[3 * ni + 2][3 * nj + 1] += k21;
                K_map[3 * ni + 2][3 * nj + 2] += k22;
            }
        }
    }

    // Convert map to sorted vectors for fast matrix operations
    std::vector<std::vector<std::pair<int, double>>> K_rows(numDOFs);
    for (size_t i = 0; i < numDOFs; ++i) {
        K_rows[i].reserve(K_map[i].size());
        for (const auto& kv : K_map[i]) {
            if (std::abs(kv.second) > 1e-18) {
                K_rows[i].emplace_back(kv.first, kv.second);
            }
        }
    }

    std::vector<double> F(numDOFs, 0.0);
    ApplyBoundaryConditions(F, K_rows);

    // Preconditioned Conjugate Gradient (PCG) with Jacobi diagonal preconditioner
    std::vector<double> U(numDOFs, 0.0);
    std::vector<double> r = F;
    std::vector<double> invDiag(numDOFs, 1.0);

    for (size_t i = 0; i < numDOFs; ++i) {
        double diag = 1.0;
        for (const auto& entry : K_rows[i]) {
            if (entry.first == static_cast<int>(i)) {
                diag = entry.second;
                break;
            }
        }
        if (std::abs(diag) > 1e-12) {
            invDiag[i] = 1.0 / diag;
        }
    }

    std::vector<double> z(numDOFs, 0.0);
    for (size_t i = 0; i < numDOFs; ++i) {
        z[i] = r[i] * invDiag[i];
    }

    std::vector<double> p = z;
    double rho = 0.0;
    for (size_t i = 0; i < numDOFs; ++i) rho += r[i] * z[i];

    double normF = 0.0;
    for (size_t i = 0; i < numDOFs; ++i) normF += F[i] * F[i];
    normF = std::sqrt(normF);
    if (normF < 1e-12) normF = 1.0;

    const int maxIterations = static_cast<int>(std::max(500UL, numDOFs * 2));
    const double tolerance = 1e-6;

    for (int iter = 0; iter < maxIterations; ++iter) {
        // q = K * p
        std::vector<double> q(numDOFs, 0.0);
        for (size_t i = 0; i < numDOFs; ++i) {
            double sum = 0.0;
            for (const auto& entry : K_rows[i]) {
                sum += entry.second * p[entry.first];
            }
            q[i] = sum;
        }

        double pDotQ = 0.0;
        for (size_t i = 0; i < numDOFs; ++i) pDotQ += p[i] * q[i];

        if (std::abs(pDotQ) < 1e-25) break;

        double alpha = rho / pDotQ;
        for (size_t i = 0; i < numDOFs; ++i) {
            U[i] += alpha * p[i];
            r[i] -= alpha * q[i];
        }

        double normR = 0.0;
        for (size_t i = 0; i < numDOFs; ++i) normR += r[i] * r[i];
        normR = std::sqrt(normR);

        if (normR / normF < tolerance) {
            break;
        }

        for (size_t i = 0; i < numDOFs; ++i) {
            z[i] = r[i] * invDiag[i];
        }

        double rhoNew = 0.0;
        for (size_t i = 0; i < numDOFs; ++i) rhoNew += r[i] * z[i];

        double beta = rhoNew / rho;
        for (size_t i = 0; i < numDOFs; ++i) {
            p[i] = z[i] + beta * p[i];
        }
        rho = rhoNew;
    }

    // Store nodal displacements
    double maxDispM = 0.0;
    for (size_t i = 0; i < numNodes; ++i) {
        nodes_[i].uX = U[3 * i + 0];
        nodes_[i].uY = U[3 * i + 1];
        nodes_[i].uZ = U[3 * i + 2];
        double d = std::sqrt(nodes_[i].uX * nodes_[i].uX +
                             nodes_[i].uY * nodes_[i].uY +
                             nodes_[i].uZ * nodes_[i].uZ);
        if (d > maxDispM) maxDispM = d;
    }

    // Element stress calculation
    std::vector<double> nodeStressSum(numNodes, 0.0);
    std::vector<double> nodeVolSum(numNodes, 0.0);

    double maxStressPa = 0.0;
    double minStressPa = 1e20;

    for (auto& el : elements_) {
        const auto& n0 = nodes_[el.nodes[0]];
        const auto& n1 = nodes_[el.nodes[1]];
        const auto& n2 = nodes_[el.nodes[2]];
        const auto& n3 = nodes_[el.nodes[3]];

        double M[16] = {
            1.0, n0.x, n0.y, n0.z,
            1.0, n1.x, n1.y, n1.z,
            1.0, n2.x, n2.y, n2.z,
            1.0, n3.x, n3.y, n3.z
        };

        double invM[16];
        if (!Invert4x4(M, invM)) continue;

        double b[4], c[4], d[4];
        for (int i = 0; i < 4; ++i) {
            b[i] = invM[1 * 4 + i];
            c[i] = invM[2 * 4 + i];
            d[i] = invM[3 * 4 + i];
        }

        // Strain tensor
        double epsXX = 0.0, epsYY = 0.0, epsZZ = 0.0;
        double gamXY = 0.0, gamYZ = 0.0, gamZX = 0.0;

        for (int i = 0; i < 4; ++i) {
            const auto& nd = nodes_[el.nodes[i]];
            epsXX += b[i] * nd.uX;
            epsYY += c[i] * nd.uY;
            epsZZ += d[i] * nd.uZ;
            gamXY += c[i] * nd.uX + b[i] * nd.uY;
            gamYZ += d[i] * nd.uY + c[i] * nd.uZ;
            gamZX += d[i] * nd.uX + b[i] * nd.uZ;
        }

        // Stress tensor
        el.sigmaXX = (lambda + 2.0 * mu) * epsXX + lambda * (epsYY + epsZZ);
        el.sigmaYY = (lambda + 2.0 * mu) * epsYY + lambda * (epsXX + epsZZ);
        el.sigmaZZ = (lambda + 2.0 * mu) * epsZZ + lambda * (epsXX + epsYY);
        el.tauXY = mu * gamXY;
        el.tauYZ = mu * gamYZ;
        el.tauZX = mu * gamZX;

        // Von Mises Stress
        double sxx_syy = el.sigmaXX - el.sigmaYY;
        double syy_szz = el.sigmaYY - el.sigmaZZ;
        double szz_sxx = el.sigmaZZ - el.sigmaXX;
        double vm = std::sqrt(0.5 * (sxx_syy * sxx_syy + syy_szz * syy_szz + szz_sxx * szz_sxx +
                                     6.0 * (el.tauXY * el.tauXY + el.tauYZ * el.tauYZ + el.tauZX * el.tauZX)));
        el.vonMisesStressPa = vm;

        for (int i = 0; i < 4; ++i) {
            uint32_t nid = el.nodes[i];
            nodeStressSum[nid] += vm * el.volumeM3;
            nodeVolSum[nid] += el.volumeM3;
        }
    }

    for (size_t i = 0; i < numNodes; ++i) {
        if (nodeVolSum[i] > 1e-18) {
            nodes_[i].vonMisesStressPa = nodeStressSum[i] / nodeVolSum[i];
        } else {
            nodes_[i].vonMisesStressPa = 0.0;
        }
        if (nodes_[i].vonMisesStressPa > maxStressPa) maxStressPa = nodes_[i].vonMisesStressPa;
        if (nodes_[i].vonMisesStressPa < minStressPa) minStressPa = nodes_[i].vonMisesStressPa;
    }

    if (minStressPa > maxStressPa) minStressPa = 0.0;

    result.success = true;
    result.nodeCount = static_cast<uint32_t>(numNodes);
    result.elementCount = static_cast<uint32_t>(elements_.size());
    result.maxVonMisesStressMPa = maxStressPa * 1e-6;
    result.minVonMisesStressMPa = minStressPa * 1e-6;
    result.maxDisplacementMm = maxDispM * 1e3;
    result.minSafetyFactor = (maxStressPa > 1e-3) ? (yieldStrengthPa_ / maxStressPa) : 999.0;

    result.visualMesh = BuildVisualSurfaceMesh(deformationScale, result.minVonMisesStressMPa, result.maxVonMisesStressMPa);

    return result;
}

SimulationMeshPayload SimulationEngine::BuildVisualSurfaceMesh(double deformationScale,
                                                              double minStressMPa,
                                                              double maxStressMPa) {
    SimulationMeshPayload payload;

    // 1. Identify external boundary triangles (faces belonging to only 1 tet)
    std::unordered_map<TriangleKey, std::pair<uint32_t, int>, TriangleKeyHash> faceMap;

    for (const auto& el : elements_) {
        uint32_t faces[4][3] = {
            {el.nodes[0], el.nodes[1], el.nodes[2]},
            {el.nodes[0], el.nodes[2], el.nodes[3]},
            {el.nodes[0], el.nodes[3], el.nodes[1]},
            {el.nodes[1], el.nodes[3], el.nodes[2]}
        };

        for (int f = 0; f < 4; ++f) {
            TriangleKey key = MakeKey(faces[f][0], faces[f][1], faces[f][2]);
            auto it = faceMap.find(key);
            if (it == faceMap.end()) {
                faceMap[key] = {el.id, 1};
            } else {
                it->second.second++;
            }
        }
    }

    // Remap node indices to visual vertex indices
    std::unordered_map<uint32_t, uint32_t> nodeToVisual;

    auto addVisualVertex = [&](uint32_t nid) -> uint32_t {
        auto it = nodeToVisual.find(nid);
        if (it != nodeToVisual.end()) return it->second;

        const auto& n = nodes_[nid];
        uint32_t vidx = static_cast<uint32_t>(payload.positions.size() / 3);

        // Deformed position in mm: (x + scale * Ux) * 1e3
        double px = (n.x + deformationScale * n.uX) * 1e3;
        double py = (n.y + deformationScale * n.uY) * 1e3;
        double pz = (n.z + deformationScale * n.uZ) * 1e3;

        payload.positions.push_back(static_cast<float>(px));
        payload.positions.push_back(static_cast<float>(py));
        payload.positions.push_back(static_cast<float>(pz));

        double stressMPa = n.vonMisesStressPa * 1e-6;
        payload.stressesMPa.push_back(static_cast<float>(stressMPa));

        double dispMm = std::sqrt(n.uX * n.uX + n.uY * n.uY + n.uZ * n.uZ) * 1e3;
        payload.displacementsMm.push_back(static_cast<float>(dispMm));

        double range = maxStressMPa - minStressMPa;
        double t = (range > 1e-6) ? ((stressMPa - minStressMPa) / range) : 0.0;
        float r, g, b;
        GetStressColor(t, r, g, b);
        payload.colors.push_back(r);
        payload.colors.push_back(g);
        payload.colors.push_back(b);

        nodeToVisual[nid] = vidx;
        return vidx;
    };

    for (const auto& el : elements_) {
        uint32_t faces[4][3] = {
            {el.nodes[0], el.nodes[1], el.nodes[2]},
            {el.nodes[0], el.nodes[2], el.nodes[3]},
            {el.nodes[0], el.nodes[3], el.nodes[1]},
            {el.nodes[1], el.nodes[3], el.nodes[2]}
        };

        for (int f = 0; f < 4; ++f) {
            TriangleKey key = MakeKey(faces[f][0], faces[f][1], faces[f][2]);
            if (faceMap[key].second == 1) {
                // External face!
                uint32_t v0 = addVisualVertex(faces[f][0]);
                uint32_t v1 = addVisualVertex(faces[f][1]);
                uint32_t v2 = addVisualVertex(faces[f][2]);
                payload.indices.push_back(v0);
                payload.indices.push_back(v1);
                payload.indices.push_back(v2);
            }
        }
    }

    return payload;
}

SimulationResult SimulationEngine::SolveCantileverBeam(double lengthMm, double widthMm, double heightMm,
                                                      double pointLoadForceN,
                                                      const Material& material,
                                                      double deformationScale,
                                                      int nx, int ny, int nz) {
    SimulationEngine sim;
    sim.SetMaterial(material);
    sim.GenerateBeamMesh(lengthMm, widthMm, heightMm, nx, ny, nz);

    // Fixed constraint at root (x = 0)
    BoundaryConstraint fixedBC;
    fixedBC.type = ConstraintType::FIXED;
    fixedBC.minX = -1.0;
    fixedBC.maxX = 0.01;
    fixedBC.minY = -1e9;
    fixedBC.maxY = 1e9;
    fixedBC.minZ = -1e9;
    fixedBC.maxZ = 1e9;
    sim.AddConstraint(fixedBC);

    // Point / line load at free end (x = lengthMm, downwards along Z)
    StructuralLoad tipLoad;
    tipLoad.type = LoadType::TOTAL_FORCE;
    tipLoad.forceX = 0.0;
    tipLoad.forceY = 0.0;
    tipLoad.forceZ = -pointLoadForceN; // Downwards in Newtons
    tipLoad.minX = lengthMm - 0.01;
    tipLoad.maxX = lengthMm + 1.0;
    tipLoad.minY = -1e9;
    tipLoad.maxY = 1e9;
    tipLoad.minZ = -1e9;
    tipLoad.maxZ = 1e9;
    sim.AddLoad(tipLoad);

    return sim.Solve(deformationScale);
}

} // namespace androcad
