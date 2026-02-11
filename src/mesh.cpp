#include "mesh.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

bool Mesh::loadOBJ(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.push_back({ glm::vec3(x, y, z) });
        } else if (type == "f") {
            std::string s;
            int vIdx[3];
            for(int i = 0; i < 3; ++i) {
                ss >> s;
                // Robust parsing: extract vertex index before any '/'
                vIdx[i] = std::stoi(s.substr(0, s.find('/'))) - 1;
            }
            faces.push_back({ vIdx[0], vIdx[1], vIdx[2] });
        }
    }
    buildEdges();
    return true;
}

void Mesh::buildEdges() {
    vertexNeighbors.assign(vertices.size(), std::set<int>());
    edges.clear();
    std::set<std::pair<int, int>> edgeSet;

    for (auto& f : faces) {
        if (f.removed) continue;
        int v[3] = {f.v0, f.v1, f.v2};
        for (int i = 0; i < 3; ++i) {
            int a = v[i], b = v[(i + 1) % 3];
            vertexNeighbors[a].insert(b);
            vertexNeighbors[b].insert(a);
            
            int mn = std::min(a, b), mx = std::max(a, b);
            if (edgeSet.find({mn, mx}) == edgeSet.end()) {
                edgeSet.insert({mn, mx});
                edges.push_back({mn, mx});
            }
        }
    }
}

void Mesh::computeVertexQuadrics() {
    for (auto& v : vertices) v.Q = glm::mat4(0.0f);

    for (auto& f : faces) {
        if (f.removed) continue;
        glm::vec3 p0 = vertices[f.v0].position, p1 = vertices[f.v1].position, p2 = vertices[f.v2].position;
        glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
        if (glm::length(n) < 1e-9f) continue; // Skip degenerate input faces
        
        n = glm::normalize(n);
        glm::vec4 plane(n.x, n.y, n.z, -glm::dot(n, p0));
        glm::mat4 Kp = glm::outerProduct(plane, plane);

        vertices[f.v0].Q += Kp; vertices[f.v1].Q += Kp; vertices[f.v2].Q += Kp;
    }
}


EdgeCollapseResult Mesh::computeEdgeCollapse(int v0, int v1) {
    glm::mat4 Q_edge = vertices[v0].Q + vertices[v1].Q;
    glm::mat4 Q_dash = Q_edge;

    // Set the last row to [0, 0, 0, 1] to solve for v
    Q_dash[0][3] = 0.0f; Q_dash[1][3] = 0.0f; Q_dash[2][3] = 0.0f; Q_dash[3][3] = 1.0f;

    EdgeCollapseResult result;
    float det = glm::determinant(Q_dash);

    // Only invert if the matrix is stable (determinant not near zero)
    if (std::abs(det) > 1e-4) { 
        glm::vec4 vBarH = glm::inverse(Q_dash) * glm::vec4(0, 0, 0, 1);
        result.vBar = glm::vec3(vBarH);
        result.valid = true;
    } else {
        // FALLBACK: If matrix is unstable, use the midpoint or one of the endpoints
        // This prevents the "exploding lines" in your screenshot
        result.vBar = (vertices[v0].position + vertices[v1].position) * 0.5f;
        result.valid = false;
    }

    glm::vec4 v4(result.vBar, 1.0f);
    result.cost = glm::dot(v4, Q_edge * v4);
    return result;
}

bool Mesh::isValidPair(int vA, int vB, glm::vec3 newPos) {
    if (vA >= vertices.size() || vB >= vertices.size()) return false;
    if (vertices[vA].removed || vertices[vB].removed) return false;

    // 1. Manifold Check: Ensure we aren't merging across a "gap" 
    // that would create non-manifold geometry.
    std::vector<int> common;
    for (int n : vertexNeighbors[vA]) {
        if (vertexNeighbors[vB].count(n)) common.push_back(n);
    }
    // In a standard manifold mesh, two vertices share exactly 2 neighbors (the edge)
    if (common.size() > 2) return false; 

    // 2. Face Flip Check with Tolerance
    for (auto& f : faces) {
        if (f.removed) continue;
        if (f.v0 != vA && f.v1 != vA && f.v2 != vA && f.v0 != vB && f.v1 != vB && f.v2 != vB) continue;

        glm::vec3 p0 = vertices[f.v0].position;
        glm::vec3 p1 = vertices[f.v1].position;
        glm::vec3 p2 = vertices[f.v2].position;
        
        // Use non-normalized cross product to check area
        glm::vec3 oldN = glm::cross(p1 - p0, p2 - p0);
        
        // Preview the new face shape
        glm::vec3 q0 = (f.v0 == vA || f.v0 == vB) ? newPos : p0;
        glm::vec3 q1 = (f.v1 == vA || f.v1 == vB) ? newPos : p1;
        glm::vec3 q2 = (f.v2 == vA || f.v2 == vB) ? newPos : p2;
        glm::vec3 newN = glm::cross(q1 - q0, q2 - q0);

        // If the triangle becomes practically a point, skip it
        if (glm::length(newN) < 1e-12f) continue; 

        // CRITICAL: Normal Flip Check. 
        // Instead of 0.0f, use a small negative epsilon to allow 
        // very slight rotations in nearly-flat areas.
        if (glm::dot(oldN, newN) < -0.001f) return false;
    }
    return true;
}

void Mesh::collapseEdge(const Edge& e) {
    int vA = e.v0, vB = e.v1;
    vertices[vA].position = e.optimalPos;
    vertices[vA].Q += vertices[vB].Q;
    vertices[vB].removed = true;

    for (auto& f : faces) {
        if (f.removed) continue;
        bool changed = false;
        if (f.v0 == vB) { f.v0 = vA; changed = true; }
        if (f.v1 == vB) { f.v1 = vA; changed = true; }
        if (f.v2 == vB) { f.v2 = vA; changed = true; }

        if (changed && (f.v0 == f.v1 || f.v1 == f.v2 || f.v2 == f.v0)) {
            f.removed = true;
        }
    }

    // Topological migration
    for (int n : vertexNeighbors[vB]) {
        if (n == vA) continue;
        vertexNeighbors[n].erase(vB);
        vertexNeighbors[n].insert(vA);
        vertexNeighbors[vA].insert(n);
    }
    vertexNeighbors[vA].erase(vB);
    vertexNeighbors[vB].clear();

    // Re-evaluate neighbors
    for (int n : vertexNeighbors[vA]) {
        EdgeCollapseResult res = computeEdgeCollapse(vA, n);
        edgeHeap.push({vA, n, res.cost, res.vBar});
    }
}

void Mesh::simplifyMesh(int targetFaceCount) {
    std::cout << "Starting simplification..." << std::endl;
    computeVertexQuadrics();
    
    while(!edgeHeap.empty()) edgeHeap.pop();

    for (auto& e : edges) {
        EdgeCollapseResult res = computeEdgeCollapse(e.v0, e.v1);
        edgeHeap.push({e.v0, e.v1, res.cost, res.vBar});
    }

    // Initial count
    int activeFaces = 0;
    for (auto& f : faces) if (!f.removed) activeFaces++;
    
    int initialFaces = activeFaces;
    int collapseCount = 0;

    while (activeFaces > targetFaceCount && !edgeHeap.empty()) {
        Edge e = edgeHeap.top();
        edgeHeap.pop();

        if (vertices[e.v0].removed || vertices[e.v1].removed) continue;
        if (!isValidPair(e.v0, e.v1, e.optimalPos)) continue;

        collapseEdge(e);
        collapseCount++;

        // Recalculate face count
        activeFaces = 0;
        for (auto& f : faces) if (!f.removed) activeFaces++;

        // Print progress every 100 collapses
        if (collapseCount % 100 == 0) {
            std::cout << "Faces: " << activeFaces << " / " << initialFaces 
                      << " (Target: " << targetFaceCount << ")" << std::endl;
        }
    }

    reindexVertices();
    std::cout << "\nSimplification Complete!" << std::endl;
    std::cout << "Final Face Count: " << vertices.size() << " vertices, " 
              << activeFaces << " faces." << std::endl;
}

void Mesh::reindexVertices() {
    std::vector<Vertex> newVerts;
    std::vector<int> oldToNew(vertices.size(), -1);
    for (int i = 0; i < vertices.size(); i++) {
        if (!vertices[i].removed) {
            oldToNew[i] = newVerts.size();
            newVerts.push_back(vertices[i]);
        }
    }
    for (auto& f : faces) {
        if (!f.removed) {
            f.v0 = oldToNew[f.v0]; f.v1 = oldToNew[f.v1]; f.v2 = oldToNew[f.v2];
        }
    }
    vertices = newVerts;
}

void Mesh::saveOBJ(const char* path) {
    std::ofstream file(path);
    if (!file.is_open()) return;
    for (auto& v : vertices) 
        file << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
    for (auto& f : faces) 
        if (!f.removed) file << "f " << f.v0 + 1 << " " << f.v1 + 1 << " " << f.v2 + 1 << "\n";
}