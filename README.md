# CGV Project — Computer Graphics & Visualization Suite

> **A comprehensive, interactive Computer Graphics & Visualization platform built with C++17, OpenGL (Legacy + Modern), GLFW, Dear ImGui, and GLM.**  
> This project was developed as a semester project for the **Computer Graphics & Visualization (CGV)** course, and is divided into three fully independent, modular sections that share a single build system and window context.

---

## Table of Contents

- [Project Overview](#project-overview)
- [Sections at a Glance](#sections-at-a-glance)
  - [Section 1 — Paint Application](#section-1--paint-application)
  - [Section 2 — Algorithm Visualizer](#section-2--algorithm-visualizer)
  - [Section 3 — Graph Algorithm Visualizer](#section-3--graph-algorithm-visualizer)
- [Technology Stack](#technology-stack)
- [Folder Structure](#folder-structure)
- [Build Instructions](#build-instructions)
- [How to Connect All Three Sections](#how-to-connect-all-three-sections)
- [Team & Ownership](#team--ownership)
- [Contributing Guidelines](#contributing-guidelines)
- [License](#license)

---

## Project Overview

This project is a self-contained **Computer Graphics learning tool** that demonstrates, in a hands-on and interactive way, how the fundamental algorithms that power every graphics engine actually work — pixel by pixel, step by step.

It is split into **three independent sections**, each self-contained in its own source and header directories, but all rendered inside the same GLFW window using a **tabbed Dear ImGui navigation bar**. Each section can be developed independently and merged through `main.cpp`.

---

## Sections at a Glance

### Section 1 — Paint Application

> **Owner: Teammate 1 (PaintApp)**

A classic raster-based paint tool where every drawing primitive — lines, circles, ellipses, polygons — is rendered using the **same foundational CGV algorithms** (Bresenham, DDA, Midpoint) instead of OpenGL's built-in primitives. This makes the Paint App a live demonstration of the algorithms in production use.

**Key Features:**
- Freehand drawing with pixel-perfect Bresenham lines under the hood
- Shape tools: Line, Circle, Ellipse, Rectangle, Polygon
- Flood Fill and Boundary Fill tools
- Color picker, canvas clear, undo/redo stack
- All rendering done via `GL_POINTS` / `GL_QUADS` on a pixel grid — no OpenGL shape primitives

**Source Workspace:** `src/PaintApp/` and `include/PaintApp/`

---

### Section 2 — Algorithm Visualizer

> **Owner: Parth (Visualizer)**

An **interactive, step-by-step educational visualizer** for 15 classic Computer Graphics algorithms. Each algorithm is represented as a **state machine** — users can execute it one step at a time, jump K steps ahead, or run it to completion. At every frame, a real-time panel displays the exact internal variables (decision parameters, increments, error terms, matrices) and the 2D pixel grid shows which pixels are being plotted and why.

**Algorithms Implemented:**

| # | Category | Algorithm |
|---|----------|-----------|
| 1 | Line Drawing | DDA (Digital Differential Analyzer) ✅ |
| 2 | Line Drawing | Bresenham's Line Algorithm |
| 3 | Line Drawing | Xiaolin Wu's Anti-Aliased Line |
| 4 | Circle & Curve | Midpoint Circle Algorithm |
| 5 | Circle & Curve | Bresenham's Circle Algorithm |
| 6 | Circle & Curve | Midpoint Ellipse Algorithm |
| 7 | Filling | Flood Fill (4-way & 8-way) |
| 8 | Filling | Boundary Fill |
| 9 | Filling | Scan-line Polygon Fill |
| 10 | Clipping | Cohen-Sutherland Line Clipping |
| 11 | Clipping | Sutherland-Hodgman Polygon Clipping |
| 12 | Transformations | 2D Transformations (Translation, Rotation, Scaling) |
| 13 | Transformations | Window-to-Viewport Transformation |
| 14 | 3D Fundamentals | Painter's Algorithm (Depth Sorting) |
| 15 | 3D Fundamentals | Z-Buffer / Depth Buffer |

**Source Workspace:** `src/Visualizer/` and `include/Visualizer/`  
**Detailed plan:** See [`SECTION2_IMPLEMENTATION_PLAN.md`](./SECTION2_IMPLEMENTATION_PLAN.md)

---

### Section 3 — Graph Algorithm Visualizer

> **Owner: Teammate 2 (GraphApp)**

An interactive **pathfinding and graph algorithm visualizer** on a 2D grid. Two points are placed on the grid (randomly or by user click), and the selected algorithm finds the path between them — animating every frontier expansion, visited node, and final path in real time.

**Algorithms Planned (8–10):**

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

**Source Workspace:** `src/GraphApp/` and `include/GraphApp/`

---

## Technology Stack

| Technology | Role |
|------------|------|
| **C++17** | Core language |
| **OpenGL (Legacy 1.x / GL_QUADS)** | Pixel-grid rendering for the visualizer |
| **OpenGL (Modern 3.x / VAO/VBO)** | Shader-based rendering for advanced sections |
| **GLFW 3** | Window creation and input handling |
| **Dear ImGui** | All UI panels, buttons, sliders, and text |
| **GLM** | Matrix math for transformation algorithms |
| **glad** | OpenGL function loader |
| **CMake 3.10+** | Cross-platform build system |

---

## Folder Structure

```
CGV_Project/
│
├── CMakeLists.txt                  # Single build configuration for all 3 sections
├── README.md                       # This file
├── SECTION2_IMPLEMENTATION_PLAN.md # Detailed plan for the Algorithm Visualizer
│
├── assets/
│   ├── shaders/                    # GLSL vertex & fragment shaders
│   │   ├── grid.vert
│   │   ├── grid.frag
│   │   ├── pixel.vert
│   │   └── pixel.frag
│   └── fonts/                      # Fonts for Dear ImGui (e.g., JetBrainsMono.ttf)
│
├── third_party/                    # External libraries — DO NOT MODIFY
│   ├── glfw/
│   ├── glad/
│   ├── glm/
│   └── imgui/
│
├── include/
│   ├── Core/
│   │   ├── Window.h
│   │   └── Shader.h
│   ├── PaintApp/                   # Section 1 headers (Teammate 1)
│   ├── GraphApp/                   # Section 3 headers (Teammate 2)
│   └── Visualizer/                 # Section 2 headers (Parth)
│       ├── IAlgorithm.h            # Abstract base for all algorithms
│       ├── VisualizerEngine.h      # Manages UI + grid rendering
│       ├── LineAlgos.h             # DDA, Bresenham, Xiaolin Wu
│       ├── CircleAlgos.h           # Midpoint Circle, Bresenham Circle, Ellipse
│       ├── FillAlgos.h             # Flood Fill, Boundary Fill, Scanline
│       ├── ClipAlgos.h             # Cohen-Sutherland, Sutherland-Hodgman
│       └── TransformAlgos.h        # 2D Transforms, Viewport, Painter's, Z-Buffer
│
└── src/
    ├── main.cpp                    # Entry point; tab-switches between sections
    ├── Core/
    │   ├── Window.cpp
    │   └── Shader.cpp
    ├── PaintApp/                   # Section 1 (Teammate 1)
    │   ├── Canvas.cpp
    │   └── PaintTools.cpp
    ├── GraphApp/                   # Section 3 (Teammate 2)
    │   ├── GraphGrid.cpp
    │   └── Pathfinding.cpp
    └── Visualizer/                 # Section 2 (Parth)
        ├── VisualizerEngine.cpp
        ├── LineAlgos.cpp
        ├── CircleAlgos.cpp
        ├── FillAlgos.cpp
        ├── ClipAlgos.cpp
        └── TransformAlgos.cpp
```

---

## Build Instructions

### Prerequisites
- CMake >= 3.10
- A C++17 compiler (GCC 9+, Clang 10+, or MSVC 2019+)
- GLFW3 installed (or place source in `third_party/glfw/`)
- OpenGL drivers installed

### Steps

```bash
# 1. Clone the repository
git clone https://github.com/<your-org>/CGV_Project.git
cd CGV_Project

# 2. Create build directory
mkdir build && cd build

# 3. Configure with CMake
cmake ..

# 4. Build
cmake --build . --config Release

# 5. Run
./CGV_Project       # Linux/Mac
CGV_Project.exe     # Windows
```

### Windows (Visual Studio)
```bash
cmake .. -G "Visual Studio 17 2022" -A x64
# Open CGV_Project.sln and build in Release mode
```

---

## Three.js 3D Pixel-Paint Demo (Integrated)

A Three.js-based 3D grid paint visualizer is now integrated at:

`web/threejs-3d-paint/`

### Run it locally

```bash
cd web/threejs-3d-paint
python3 -m http.server 8080
```

Then open:

`http://localhost:8080`

This demo helps visualize pixel/block coloring with raycasting in a 3D grid and can be used as a reference for future 3D CGV features.

---

## CGV Web App (Desktop-to-Web Conversion)

A new web app version of the current CGV flow is available at:

`web/cgv-webapp/`

It includes:
- Home page with 3 module choices
- Algorithm Visualizer UI (Theory + Visualize)
- Live DDA step controls
- 3D grid visualization in the visualize viewport (Three.js/WebGL)

### Run the web app

```bash
cd web/cgv-webapp
python3 -m http.server 8090
```

Open: `http://localhost:8090`

---

## How to Connect All Three Sections

> See [`SECTION2_IMPLEMENTATION_PLAN.md`](./SECTION2_IMPLEMENTATION_PLAN.md) for a detailed integration guide for Section 2.

The three sections are connected through `src/main.cpp` using **Dear ImGui's tab bar**. Each section exposes a simple two-function interface that `main.cpp` calls every frame:

```cpp
// The interface every section must implement
void renderUI();                              // Draws the ImGui panel
void renderGrid(int width, int height);       // Draws the OpenGL grid/canvas
```

### Integration Pattern in `main.cpp`

```cpp
// --- SECTION OBJECTS ---
VisualizerEngine visualizer;    // Section 2 — Parth already added this
// PaintApp        paintApp;    // Section 1 — Teammate 1 adds this
// GraphApp        graphApp;    // Section 3 — Teammate 2 adds this

// --- MAIN LOOP ---
while (!glfwWindowShouldClose(window)) {
    // ... ImGui new frame setup ...

    // Tab bar to switch sections
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

    // Render whichever section is active
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // activeSection->renderGrid(width, height);

    // ... ImGui render + swap buffers ...
}
```

### Rules for All Teammates

1. **Never touch `main.cpp` logic** beyond adding your object and calling `renderUI()` / `renderGrid()`.
2. **Never touch `third_party/`** — it is shared and read-only.
3. **Work only in your designated directories** (`src/[YourSection]/` and `include/[YourSection]/`).
4. **Share `IAlgorithm.h`** conventions if you need algorithm state machines — the interface is already defined.
5. **Use `GL_VERSION 3.0`** as the minimum — do not rely on extension-only features.
6. **All UI must go through Dear ImGui** — do not create separate OS windows.

### Teammate 1 (PaintApp) — Connection Checklist
- [ ] Create `include/PaintApp/PaintApp.h` with `class PaintApp { void renderUI(); void renderCanvas(int w, int h); };`
- [ ] Implement in `src/PaintApp/`
- [ ] In `main.cpp`, add `#include "../include/PaintApp/PaintApp.h"` and instantiate `PaintApp paintApp;`
- [ ] Call `paintApp.renderUI()` inside the "Paint App" tab item
- [ ] Call `paintApp.renderCanvas(width, height)` in the render section

### Teammate 2 (GraphApp) — Connection Checklist
- [ ] Create `include/GraphApp/GraphApp.h` with `class GraphApp { void renderUI(); void renderGrid(int w, int h); };`
- [ ] Implement in `src/GraphApp/`
- [ ] In `main.cpp`, add `#include "../include/GraphApp/GraphApp.h"` and instantiate `GraphApp graphApp;`
- [ ] Call `graphApp.renderUI()` inside the "Graph Visualizer" tab item
- [ ] Call `graphApp.renderGrid(width, height)` in the render section

---

## Team & Ownership

| Section | Developer | Directory |
|---------|-----------|-----------|
| Section 1 — Paint Application | Teammate 1 | `src/PaintApp/`, `include/PaintApp/` |
| Section 2 — Algorithm Visualizer | **Parth** | `src/Visualizer/`, `include/Visualizer/` |
| Section 3 — Graph Algorithm Visualizer | Teammate 2 | `src/GraphApp/`, `include/GraphApp/` |
| Core / Build | All | `src/Core/`, `CMakeLists.txt`, `main.cpp` |

---

## Contributing Guidelines

- Use `git branch <section>/<feature>` naming (e.g., `visualizer/bresenham-line`, `paintapp/flood-fill`)
- Never push directly to `main` — use pull requests
- Keep all `#include` paths relative to the project root
- Do not add new `third_party` libraries without team consensus
- Comment every algorithm implementation with the mathematical formula/step it corresponds to

---

## License

This project is submitted as an academic course project. All code is original unless explicitly noted in comments. Redistribution requires author permission.
