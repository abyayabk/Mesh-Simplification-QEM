Mesh Simplification Using Quadric Error Metrics (QEM)

Example of mesh simplification applied to a watertight 3D model.

Overview

This project implements mesh simplification using the Quadric Error Metrics (QEM) algorithm, a foundational technique in computer graphics for reducing polygon counts while preserving geometric fidelity. The goal is to take high-resolution 3D meshes and produce lower-resolution approximations, suitable for real-time rendering, interactive applications, and 3D visualization.

This project demonstrates:

- Understanding of geometry, linear algebra, and 3D topology.

- Practical application of vertex quadrics and plane fitting.

- Edge collapse heuristics with validity checks for manifold preservation.

- A custom OBJ loader and writer for full mesh I/O.


Features

- Load and save OBJ meshes.

- Build vertex adjacency and edge sets for simplification.

- Compute per-vertex quadrics using plane equations of incident faces.

- Evaluate edge collapse costs and compute optimal positions for vertex removal.

- Maintain mesh integrity by preventing face flips and non-manifold edges.

- Fully adjustable target face count for mesh reduction.

- Optional visualization-ready vertex and face normals.


How It Works

* Load Mesh

- Reads .OBJ files with vertices and triangular faces.

- Stores vertex positions, adjacency, and face connectivity.


* Compute Quadrics

- For each face, compute plane equation (a,b,c,d) and generate a 4x4 quadric matrix Kp = pp^T.

- Sum the quadrics for each incident vertex to form Qv.


* Edge Collapse

- For every edge (v0,v1):

    Compute optimal collapse position using Q0 + Q1.

    Check validity to prevent inverted faces or non-manifold geometry.

- Collapse edges iteratively until the target number of faces is reached.


* Output Mesh

- Reindex vertices to remove deleted ones.

- Export simplified mesh as .OBJ.


**References**

Garland, M., & Heckbert, P. S. (1997). Surface Simplification Using Quadric Error Metrics.

Stanford 3D Scanning Repository – Bunny, Dragon, Armadillo

GLM: OpenGL Mathematics