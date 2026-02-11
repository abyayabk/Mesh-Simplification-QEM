#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <queue>
#include <set>  

struct Vertex {
    glm::vec3 position;
    bool removed = false;
    glm::mat4 Q = glm::mat4(0.0f);
};

struct Face {
    int v0, v1, v2;          // indices into vertices
    glm::vec3 faceNormal;     // normal of this face
    float a, b, c, d;                  // plane equation: ax + by + cz + d = 0
    bool removed = false;
    // std::vector<int> adjacentFaces; // indices of neighboring faces (optional)
};

struct Edge {
    int v0, v1;       // vertex indices
    float cost = 0.0f; // quadric error cost
    glm::vec3 optimalPos;
};

struct EdgeCollapseResult {
    glm::vec3 vBar;   // optimal collapse position
    float cost;       // error at vBar
    bool valid;       // if matrix was invertible
};

struct EdgeCompare {
    bool operator()(const Edge& a, const Edge& b) {
        return a.cost > b.cost; // min-heap
    }
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Edge> edges;           // all edges
    std::vector<glm::vec3> normalLines;
    std::vector<Edge> collapsibleEdges;       // edges that can safely collapse
    std::priority_queue<Edge, std::vector<Edge>, EdgeCompare> edgeHeap;

    std::vector<std::set<int>> vertexNeighbors; // adjacency: vertex -> neighboring vertices
    std::vector<Edge> getNeighborEdges(int v);  // return edges connected to vertex v

    bool loadOBJ(const char* path);  // load vertex + face data

    void buildEdges();                // compute edges from faces
    bool isValidPair(int vA, int vB, glm::vec3 newPos); //vA and vB are the two vertices of edge we collapse. After computng the mimimum Q, get the vector with lowest error as newPos
    void computeVertexQuadrics(); // Find Q for each vertex, remember Q for each vertex is the sum of squared of all of its plane eqn as matrix(p) -> p(transpose) * p
    EdgeCollapseResult computeEdgeCollapse(int v0, int v1);
    void collapseEdge(const Edge& e);
    void reindexVertices();

    void simplifyMesh(int targetFaceCount);

    void saveOBJ(const char* path); //For exporting an obj file
};
