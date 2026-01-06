# Project Atlas
- A voxel engine using OpenGL 4.6 Core, C++17. For use with Windows.

<h3>
Features:
</h3>

- SSAO
- Frustrum Culling
- Terrain generation using libnoise
- Placement and deletion of blocks
- World data persistence:
    - Auto-saving world state
    - Manual save system

<h2>
Preview
</h2>

<p align="center">
  <video src="milestones/demo/demo1.mp4" controls width="100%"></video>
</p>

<h2>
Milestones
</h2>

| Terrain Generation + Skybox |
|---------|
| *Inital terrain generation using a simple heightmap.* |
| ![Alt Text 1](milestones/1_terraingen_cubemap.png)|

| Terrain Generation w/libnoise |
|---------|
| *Terrain generation using libnoise for more interesting views, trees are WIP.* |
| ![Alt Text 1](milestones/2_betterterrain_blocksplace.png)|

| Proper Tree Generation |
|---------|
| *Updated tree generation to randomly construct canopy.* |
| ![Alt Text 1](milestones/3_propertrees.png)|

| G-Buffer Normal | G-Buffer Depth |
|----------------------------|--------------------------------|
| *Working on implementing SSAO. Implemented G-Buffer with debug view for surface normals.* | *Working on implementing SSAO. Implemented G-Buffer with debug view for depth.* |
| ![](milestones/4a_normals.png) | ![](milestones/4b_depth.png) |

| SSAO (Off) | SSAO (On) |
|----------------------------|--------------------------------|
| *Previous version of engine before implementation of SSAO.* | *SSAO significantly improves scene depth by enhancing contact shadows at the intersections where blocks meet. This helps improve the detail of the geometry.* |
| ![](milestones/5a_SSAO_OFF.png) | ![](milestones/5b_SSAO_ON.png) |

| Frustrum Culling (Off) | Frustrum Culling (On) |
|----------------------------|--------------------------------|
| *Previous version of engine without frustrum culling.* | *Frustrum culling is an optimization used to render ONLY the blocks found inside the camera view frustrum. This cuts down on the number of chunks/blocks being rendered considerably.* |
| ![](milestones/6a2_FC_OFF.png) | ![](milestones/6b2_FC_ON.png) |
|  | ![Alt Text 1](milestones/VisualCameraFrustum.png) | 

<h2>
Requirements
</h2>

> - [Download](https://visualstudio.microsoft.com/vs/community/) Visual Studio 2022 Community Edition.
> -- Install workloads: *Desktop development with C++*.
> - [Download](https://cmake.org/download/) and install CMake (>= v3.31.0).

<h2>
Build
</h2>

- Clone repo:
```
git clone https://github.com/RobRob7/ProjectAtlas.git
```
- Then run commands:
```
cd ProjectAtlas
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
<h2>
Run
</h2>

- For Command Prompt:

```
cd Release
Atlas.exe
```
- For Git Bash:
```
cd Release
./Atlas.exe
```


<h2>
Dependencies
</h2>

Libraries already provided, the following are used:
|Library|Usage|Version|
|-------|-------|-----|
|[FreeType](https://freetype.org/download.html)|Font rendering|v2.10.0|
|[Glad](https://glad.dav1d.de/)|OpenGL loader generator|N/A|
|[GLFW](https://www.glfw.org/download.html)|API for creating windows, contexts and surfaces, receiving input and events|v3.4|
|[GLM](https://github.com/g-truc/glm/releases/tag/1.0.1)|Header only C++ mathematics library|v1.01|
|[ImGui](https://github.com/ocornut/imgui/releases/tag/v1.92.5)|Bloat-free Graphical User interface for C++|v1.92.5|
|[libnoise](https://libnoise.sourceforge.net/)|A portable, open-source, coherent noise-generating library for C++|v1.0.0|

<h2>
Project Structure
</h2>

Project layout:
- **include/**
  - all my header files
- **src/**
    - main.cpp → main driver
    - **chunk/**
        - chunkdata.cpp → chunk data
        - chunkmanager.cpp → management of chunk meshes
        - chunkmesh.cpp → chunk mesh
    - **core/**
        - application.cpp → main application
        - scene.cpp → object setup + renderer call
    - **player/**
        - crosshair.cpp → crosshair UI icon
    - **renderer/**
        - debugpass.cpp → gBuffer normal + depth view
        - gbufferpass.cpp → gBuffer pass
        - renderer.cpp → render pipeline
        - ssaopass.cpp → SSAO pass
    - **save/**
        - save.cpp → world state saving
    - **system/**
        - camera.cpp → camera system
    - **utility/**
        - cubemap.cpp → setup + render cubemap
        - shader.cpp → shader helper class
        - texture.cpp → setup texture
- **res/**
  - **shader/** → Shaders
  - **texture/** → Textures
- **deps/** → Dependency files
- **papers/** → Papers implemented
