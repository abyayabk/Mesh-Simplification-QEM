# Mesh Simplification Using Quadric Error Metrics (QEM)

![Mesh Comparison 1](<Mesh Comparison 1.png>)
![Mesh Comparison 2](<Mesh Comparison 2.png>)
![Mesh Comparison 3](<Mesh Comparison 3.png>)

---

## Overview

This project implements **Mesh Simplification** using **Quadric Error Metrics (QEM)**, a classical algorithm in computer graphics for reducing mesh complexity while preserving geometric fidelity. The main goal is to simplify high-resolution 3D meshes into lower-resolution representations, suitable for **real-time rendering, interactive applications, and 3D visualization**.  

I developed this project from scratch, including **OBJ loading/saving, adjacency computation, quadric calculations, and edge collapse with a priority heap**, fully inspired by Garland & Heckbert's 1997 paper. This project demonstrates a combination of **geometry, linear algebra, and computational geometry**, showing not only coding skills but also deep theoretical understanding — a key part of my preparation for graduate studies in Computer Science.

---

## Key Features

- **OBJ Loader & Writer**: Supports standard triangular meshes.
- **Vertex Quadrics Computation**: Uses plane equations from incident faces to compute per-vertex quadrics.
- **Edge Collapse using Priority Heap**: Collapses edges based on **minimal quadric error**, with a heap to always select the most optimal edge.
- **Manifold & Face-Flip Safety Checks**: Prevents collapsing edges that would invert faces or break mesh topology.
- **Target Face Count Simplification**: Allows reducing meshes to a specified number of faces.
- **Reindexing of Vertices**: Cleans up deleted vertices and updates face indices for correct OBJ export.

---

## Algorithm Workflow

1. **Load OBJ Mesh**
2. **Build Vertex Adjacency & Edges**
3. **Compute Per-Face Plane Equations**
4. **Compute Vertex Quadrics**
5. **Build Edge Heap**
   - Each edge gets a **collapse cost** based on the quadric metric.
   - Heap always selects the **edge with minimal cost**.
6. **Iterative Edge Collapse**
   - Pop edge from heap.
   - Validate edge collapse (manifold & face flip).
   - Collapse edge and update:
     - Vertex positions & quadrics
     - Faces (remove degenerate faces)
     - Vertex adjacency
     - Heap entries for affected edges
7. **Reindex Vertices**
8. **Export Simplified OBJ**

This approach ensures **efficient simplification while preserving mesh topology**, closely following the theory of Garland & Heckbert.

---

## Usage

```bash
# Build project
mkdir build && cd build
cmake ..
cmake --build .

# Run simplification
./hello_world


References

Garland, M., & Heckbert, P. S. (1997). Surface Simplification Using Quadric Error Metrics.

Stanford 3D Scanning Repository – Bunny, Dragon, Armadillo

GLM: OpenGL Mathematics

LearnOpenGL - https://learnopengl.com/