# CGV Project — Computer Graphics & Visualization Suite

<div align="center">

```
 ██████╗ ██████╗ ██╗   ██╗
██╔════╝██╔════╝ ██║   ██║
██║     ██║  ███╗██║   ██║
██║     ██║   ██║╚██╗ ██╔╝
╚██████╗╚██████╔╝ ╚████╔╝
 ╚═════╝ ╚═════╝   ╚═══╝
```

**Computer Graphics & Visualization Suite**

*A hands-on, interactive platform that shows how every pixel gets drawn — algorithm by algorithm, step by step.*

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=c%2B%2B)
![OpenGL](https://img.shields.io/badge/OpenGL-3.0%2B-5586A4?style=flat-square&logo=opengl)
![GLFW](https://img.shields.io/badge/GLFW-3-white?style=flat-square)
![Dear ImGui](https://img.shields.io/badge/Dear%20ImGui-docking-purple?style=flat-square)
![CMake](https://img.shields.io/badge/CMake-3.10%2B-064F8C?style=flat-square&logo=cmake)
![License](https://img.shields.io/badge/License-Academic-green?style=flat-square)

</div>

---

## Table of Contents

- [Overview](#-overview)
- [Sections at a Glance](#-sections-at-a-glance)
  - [Section 1 — Paint Application](#section-1--paint-application)
  - [Section 2 — Algorithm Visualizer](#section-2--algorithm-visualizer)
  - [Section 3 — Graph Algorithm Visualizer](#section-3--graph-algorithm-visualizer)
- [Technology Stack](#-technology-stack)
- [Folder Structure](#-folder-structure)
- [Build Instructions](#-build-instructions)
- [Connecting All Three Sections](#-connecting-all-three-sections)

---

## 🎯 Overview

This project is a self-contained **Computer Graphics learning tool** that demonstrates — in a hands-on, interactive way — how the fundamental algorithms powering every graphics engine actually work: pixel by pixel, step by step.

It is split into **three independent sections**, each self-contained in its own source and header directories, but all rendered inside the **same GLFW window** using a tabbed Dear ImGui navigation bar. Each section can be developed and tested independently, then merged through `main.cpp`.

> **Executable name:** `final_visualize` (Linux/Mac) · `final_visualize.exe` (Windows)

---

## 🗂 Sections at a Glance

### Section 1 — Paint Application

> `src/PaintApp/` · `include/PaintApp/`

A classic raster-based paint tool where every drawing primitive — lines, circles, ellipses, polygons — is rendered using the **same foundational CGV algorithms** (Bresenham, DDA, Midpoint) instead of OpenGL's built-in primitives. The Paint App is a live demonstration of those algorithms in production use.

**Key Features:**
- Freehand drawing with pixel-perfect Bresenham lines under the hood
- Shape tools: Line, Circle, Ellipse, Rectangle, Polygon
- Flood Fill and Boundary Fill tools
- Color picker, canvas clear, undo/redo stack
- All rendering via `GL_POINTS` / `GL_QUADS` on a pixel grid — **zero OpenGL shape primitives**

---

### Section 2 — Algorithm Visualizer

> `src/Visualizer/` · `include/Visualizer/`

An **interactive, step-by-step educational visualizer** for 15 classic Computer Graphics algorithms. Each algorithm is modelled as a **state machine** — users can execute it one step at a time, jump K steps ahead, or run it to completion. A real-time panel displays the exact internal variables (decision parameters, increments, error terms, matrices), and the 2D pixel grid shows which pixels are being plotted and why.

**Algorithms Implemented:**

| # | Category | Algorithm | Status |
|---|----------|-----------|--------|
| 1 | Line Drawing | DDA (Digital Differential Analyzer) | ✅ |
| 2 | Line Drawing | Bresenham's Line Algorithm | ✅ |
| 3 | Line Drawing | Xiaolin Wu's Anti-Aliased Line | ✅ |
| 4 | Circle & Curve | Midpoint Circle Algorithm | ✅ |
| 5 | Circle & Curve | Bresenham's Circle Algorithm | ✅ |
| 6 | Circle & Curve | Midpoint Ellipse Algorithm | ✅ |
| 7 | Filling | Flood Fill (4-way & 8-way) | ✅ |
| 8 | Filling | Boundary Fill | ✅ |
| 9 | Filling | Scan-line Polygon Fill | ✅ |
| 10 | Clipping | Cohen-Sutherland Line Clipping | ✅ |
| 11 | Clipping | Sutherland-Hodgman Polygon Clipping | ✅ |
| 12 | Transformations | 2D Transformations (Translation, Rotation, Scaling) | ✅ |
| 13 | Transformations | Window-to-Viewport Transformation | ✅ |
| 14 | 3D Fundamentals | Painter's Algorithm (Depth Sorting) | ✅ |
| 15 | 3D Fundamentals | Z-Buffer / Depth Buffer | ✅ |

📄 Detailed plan: [`SECTION2_IMPLEMENTATION_PLAN.md`](./SECTION2_IMPLEMENTATION_PLAN.md)

---

### Section 3 — Graph Algorithm Visualizer

> `src/GraphApp/` · `include/GraphApp/`

An interactive **pathfinding and graph algorithm visualizer** on a 2D grid. Two points are placed on the grid (randomly or by user click), and the selected algorithm finds the path between them — animating every frontier expansion, visited node, and final path in real time.

**Algorithms Planned:**

| # | Algorithm |
|---|-----------|
| 1 | Breadth-First Search (BFS) |
| 2 | Depth-First Search (DFS) |
| 3 | Dijkstra's Algorithm |
| 4 | A* (A-Star) Search |
| 5 | Greedy Best-First Search |
| 6 | Bellman-Ford |
| 7 | Floyd-Warshall |
| 8 | Bidirectional BFS |
| 9 | Jump Point Search (JPS) |
| 10 | IDA* (Iterative Deepening A*) |

---

## 🛠 Technology Stack

| Technology | Version | Role |
|------------|---------|------|
| **C++17** | — | Core language |
| **OpenGL Legacy** | 1.x | `GL_QUADS` pixel-grid rendering for the visualizer |
| **OpenGL Modern** | 3.x | VAO/VBO shader-based rendering for advanced sections |
| **GLFW** | 3 | Window creation and input handling |
| **Dear ImGui** | docking branch | All UI panels, buttons, sliders, and text |
| **GLM** | — | Matrix math for transformation algorithms |
| **glad** | — | OpenGL function loader |
| **CMake** | 3.10+ | Cross-platform build system |

---

## 📁 Folder Structure

```
CGV_Project/
│
├── CMakeLists.txt                   # Single build config for all 3 sections
├── README.md                        # This file
├── SECTION2_IMPLEMENTATION_PLAN.md  # Detailed plan for the Algorithm Visualizer
│
├── assets/
│   ├── shaders/                     # GLSL vertex & fragment shaders
│   │   ├── grid.vert
│   │   ├── grid.frag
│   │   ├── pixel.vert
│   │   └── pixel.frag
│   └── fonts/                       # Fonts for Dear ImGui (e.g., JetBrainsMono.ttf)
│
├── third_party/                     # External libraries — DO NOT MODIFY
│   ├── glfw/
│   ├── glad/
│   ├── glm/
│   └── imgui/
│
├── include/
│   ├── Core/
│   │   ├── Window.h
│   │   └── Shader.h
│   ├── PaintApp/                    # Section 1 headers
│   ├── GraphApp/                    # Section 3 headers
│   └── Visualizer/                  # Section 2 headers
│       ├── IAlgorithm.h             # Abstract base for all algorithms
│       ├── VisualizerEngine.h       # Manages UI + grid rendering
│       ├── LineAlgos.h              # DDA, Bresenham, Xiaolin Wu
│       ├── CircleAlgos.h            # Midpoint Circle, Bresenham Circle, Ellipse
│       ├── FillAlgos.h              # Flood Fill, Boundary Fill, Scanline
│       ├── ClipAlgos.h              # Cohen-Sutherland, Sutherland-Hodgman
│       └── TransformAlgos.h         # 2D Transforms, Viewport, Painter's, Z-Buffer
│
└── src/
    ├── main.cpp                     # Entry point; tab-switches between sections
    ├── Core/
    │   ├── Window.cpp
    │   └── Shader.cpp
    ├── PaintApp/                    # Section 1
    │   ├── Canvas.cpp
    │   └── PaintTools.cpp
    ├── GraphApp/                    # Section 3
    │   ├── GraphGrid.cpp
    │   └── Pathfinding.cpp
    └── Visualizer/                  # Section 2
        ├── VisualizerEngine.cpp
        ├── LineAlgos.cpp
        ├── CircleAlgos.cpp
        ├── FillAlgos.cpp
        ├── ClipAlgos.cpp
        └── TransformAlgos.cpp
```

---

## 🔨 Build Instructions

### Prerequisites

- CMake >= 3.10
- A C++17 compiler — GCC 9+, Clang 10+, or MSVC 2019+
- GLFW3 installed system-wide **or** placed as source in `third_party/glfw/`
- OpenGL drivers installed (any GPU from the last decade will do)

---

### Fresh Build (Linux / macOS)

> ⚠️ **Always build from inside the `build/` directory.** Running `cmake ..` from the project root will cause CMake to look for `CMakeLists.txt` one level up and fail.

```bash
# 1. Clone the repository
git clone https://github.com/<your-org>/CGV_Project.git
cd CGV_Project

# 2. Create the build directory and move into it
mkdir build && cd build

# 3. Configure with CMake
#    ".." correctly points CMake to CGV_Project/CMakeLists.txt
cmake ..

# 4. Compile
cmake --build . --config Release

# 5. Run
./final_visualize
```

---

### Rebuilding from Scratch (Recommended after major changes)

If you run into stale cache issues, CMake errors, or linker weirdness — **delete the build directory entirely** and start fresh. This is the safest fix.

```bash
# From CGV_Project/ (the project root — NOT inside build/)
rm -rf build

# Recreate and configure
mkdir build && cd build
cmake ..

# Rebuild
cmake --build . --config Release

# Run
./final_visualize
```

> 💡 **Tip:** Get into the habit of doing a clean rebuild whenever you pull changes that touch `CMakeLists.txt` or `third_party/`.

---

### Windows (Visual Studio)

```bash
# From the project root
mkdir build && cd build

cmake .. -G "Visual Studio 17 2022" -A x64

# Open CGV_Project.sln in Visual Studio and build in Release mode,
# OR build from the command line:
cmake --build . --config Release

# Run
.\Release\final_visualize.exe
```

> **Rebuilding cleanly on Windows:** Delete the `build\` folder in File Explorer (or `rmdir /s /q build` in cmd), then re-run the steps above.

---

## 🔗 Connecting All Three Sections

All three sections are wired together in `src/main.cpp` using **Dear ImGui's tab bar**. Each section exposes a simple two-method interface that `main.cpp` calls every frame:

```cpp
void renderUI();                        // Draws the ImGui control panel
void renderGrid(int width, int height); // Draws the OpenGL canvas/grid
```

### Integration Pattern in `main.cpp`

```cpp
// --- SECTION OBJECTS ---
VisualizerEngine visualizer;    // Section 2 — already wired
// PaintApp        paintApp;    // Section 1
// GraphApp        graphApp;    // Section 3

// --- MAIN LOOP ---
while (!glfwWindowShouldClose(window)) {
    // ... ImGui new frame setup ...

    if (ImGui::BeginTabBar("MainTabs")) {

        if (ImGui::BeginTabItem("Paint App")) {
            // paintApp.renderUI();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Algorithm Visualizer")) {
            visualizer.renderUI();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Graph Visualizer")) {
            // graphApp.renderUI();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // activeSection->renderGrid(width, height);

    // ... ImGui render + glfwSwapBuffers ...
}
```

---

### Development Rules

1. **Never modify `main.cpp` logic** beyond adding your section object and calling `renderUI()` / `renderGrid()`.
2. **Never touch `third_party/`** — it is shared and read-only.
3. **Work only in your section's directories** (`src/[Section]/` and `include/[Section]/`).
4. **Use `IAlgorithm.h`** conventions if you need algorithm state machines — the interface is already defined.
5. **Target OpenGL 3.0** as the minimum — do not use extension-only features.
6. **All UI must go through Dear ImGui** — do not create separate OS windows.

---

### PaintApp — Integration Checklist

- [ ] Create `include/PaintApp/PaintApp.h`
  ```cpp
  class PaintApp {
  public:
      void renderUI();
      void renderCanvas(int w, int h);
  };
  ```
- [ ] Implement in `src/PaintApp/`
- [ ] In `main.cpp`: `#include "../include/PaintApp/PaintApp.h"` and add `PaintApp paintApp;`
- [ ] Call `paintApp.renderUI()` inside the `"Paint App"` tab item
- [ ] Call `paintApp.renderCanvas(width, height)` in the render section

### GraphApp — Integration Checklist

- [ ] Create `include/GraphApp/GraphApp.h`
  ```cpp
  class GraphApp {
  public:
      void renderUI();
      void renderGrid(int w, int h);
  };
  ```
- [ ] Implement in `src/GraphApp/`
- [ ] In `main.cpp`: `#include "../include/GraphApp/GraphApp.h"` and add `GraphApp graphApp;`
- [ ] Call `graphApp.renderUI()` inside the `"Graph Visualizer"` tab item
- [ ] Call `graphApp.renderGrid(width, height)` in the render section

---

<div align="center">

*Submitted as an academic course project for Computer Graphics & Visualization (CGV).*  
*All code is original unless explicitly noted in source comments.*  
*Redistribution requires author permission.*

</div>