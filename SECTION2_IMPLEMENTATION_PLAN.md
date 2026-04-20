# Section 2 — Algorithm Visualizer: Complete Implementation Plan

> **Developer:** Parth  
> **Tech Stack:** C++17, OpenGL (Legacy + Modern Pipeline), GLFW 3, Dear ImGui, GLM  
> **Workspace:** `src/Visualizer/` · `include/Visualizer/`

---

## 1. What Is Already Built

### ✅ `IAlgorithm.h` — Abstract Base Interface
Defines the pure-virtual interface all 15 algorithms implement:
`init()`, `step()`, `stepK()`, `runToCompletion()`, `isFinished()`, `getHighlightedPixels()`  
`Pixel` struct with `int x, y`

> **Gap:** `init()` takes 4 ints — fine for lines, needs generalization for circles (center+radius), polygons (vertex list). Fix: add overloaded variants or an `AlgoParams` union.

### ✅ `DDALine` — State Machine (Complete)
Correctly implements: `calculateIncrements()`, `step()`, `stepK()`, `runToCompletion()`, all getters for ImGui display.

> **Gap:** `path` vector grows unbounded — cap at 1000 steps for safety.

### ✅ `VisualizerEngine` — Rendering + UI Shell
Working: ImGui control panel, dynamic `glOrtho` grid, `GL_QUADS` pixel rendering with zoom.

> **Critical gaps:**
> 1. Hardcoded `DDALine ddaAlgorithm` → must become `std::unique_ptr<IAlgorithm>`
> 2. No Theory panel — only live values shown
> 3. No algorithm selector dropdown
> 4. Single red color for all pixels — needs current-step highlight (yellow)

### ✅ `main.cpp` — GLFW + ImGui Loop
Correct. No tab bar yet — needed for teammate integration.

### 🔲 Empty Stubs: `FillAlgos.h/.cpp`, `ClipAlgos.h/.cpp`

---

## 2. Architecture — State Machine Pattern

```
init() ──► [READY] ──► step()/stepK()/runToCompletion() ──► [RUNNING] ──► [FINISHED]
                            │
              getHighlightedPixels() → grid render
              getCurrentState()      → ImGui panel
```

### Extended IAlgorithm Interface

```cpp
struct AlgoState {
    std::string stepDescription;        // "Plotting pixel (5, 3) — rounding 4.6 → 5"
    std::vector<std::string> variables; // {"dx = 8", "dy = 6", "p = 10"}
    std::string formula;                // "p_new = p + 2*dy - 2*dx"
};

class IAlgorithm {
public:
    // Existing
    virtual void init(int x1, int y1, int x2, int y2) = 0;
    virtual void step() = 0;
    virtual void stepK(int k) = 0;
    virtual void runToCompletion() = 0;
    virtual bool isFinished() const = 0;
    virtual std::vector<Pixel> getHighlightedPixels() const = 0;
    // New
    virtual AlgoState getCurrentState() const = 0;
    virtual std::string getTheory() const = 0;
    virtual std::string getName() const = 0;
    virtual void reset() = 0;
};
```

### VisualizerEngine Refactor

```cpp
class VisualizerEngine {
private:
    std::unique_ptr<IAlgorithm> currentAlgorithm;  // replaces DDALine
    int selectedAlgoIndex = 0;
    int inputX1=0, inputY1=0, inputX2=10, inputY2=8;
    int kSteps = 3;
    void drawGrid(int w, int h, float gridMax);
    void drawPixels(const std::vector<Pixel>& pixels);
    void drawTheoryPanel();
    void drawControlPanel();
    void drawStatePanel();
    std::unique_ptr<IAlgorithm> createAlgorithm(int index);
public:
    VisualizerEngine();
    void renderUI();
    void renderGrid(int w, int h);
};
```

---

## 3. All 15 Algorithms — Build Plan

### Phase 1 — Foundation Fixes (Do First)

| File | Change |
|------|--------|
| `include/Visualizer/IAlgorithm.h` | Add `AlgoState` struct + 4 new virtual methods |
| `include/Visualizer/LineAlgos.h` | Add `getCurrentState()`, `getTheory()`, `getName()`, `reset()` to DDALine |
| `src/Visualizer/LineAlgos.cpp` | Implement those 4 methods for DDALine |
| `include/Visualizer/VisualizerEngine.h` | Replace `DDALine` member with `unique_ptr<IAlgorithm>` |
| `src/Visualizer/VisualizerEngine.cpp` | Add dropdown, Theory tab, yellow current-step pixel |

---

### Phase 2 — Line Algorithms (3 total)

**File:** `include/Visualizer/LineAlgos.h` + `src/Visualizer/LineAlgos.cpp`

#### Algorithm 1: DDA Line ✅ Already done

#### Algorithm 2: Bresenham's Line

```
Init:  dx=|x2-x1|, dy=|y2-y1|, p = 2*dy - dx
Step:  if p < 0:  x++,      p += 2*dy
       else:      x++, y++, p += 2*dy - 2*dx
State: "p = 6 ≥ 0 → Y increments"  |  "p = -2 < 0 → Y stays"
```

Key getters to add: `getDecisionParam() const { return p; }`

#### Algorithm 3: Xiaolin Wu's Line (Anti-aliasing)

```
Uses fractpart(v) = v - floor(v), rfrac(v) = 1 - fractpart(v)
Each step plots 2 side-by-side pixels with intensities: fractpart(y) and rfrac(y)
Grid rendering needs alpha: glColor4f(r, g, b, pixel.intensity)
```

New struct: `struct IntensityPixel { int x, y; float intensity; };`  
New method: `std::vector<IntensityPixel> getIntensityPixels() const;`

---

### Phase 3 — Circle & Curve Algorithms (3 total)

**New files:** `include/Visualizer/CircleAlgos.h` + `src/Visualizer/CircleAlgos.cpp`

#### Algorithm 4: Midpoint Circle

```
Init:  p = 1 - r,  x = 0,  y = r
Step:  plot8Points(cx+x, cy+y) and all 7 symmetric variants
       if p < 0:  p += 2*x + 3
       else:      y--,  p += 2*(x-y) + 5
       x++
State: "p = -3 < 0 → y stays. Plotting 8 symmetric pixels."
```

#### Algorithm 5: Bresenham's Circle

Similar to midpoint with different error term update formula.

#### Algorithm 6: Midpoint Ellipse

```
Region 1 (while 2*b²*x < 2*a²*y):
  p1 init = b² - a²*b + a²/4
  if p1 < 0: p1 += 2*b²*x + b²    (x++ only)
  else:      p1 += 2*b²*x - 2*a²*y + b²  (x++, y--)

Region 2 (until y < 0):
  p2 init = b²*(x+0.5)² + a²*(y-1)² - a²*b²
  if p2 > 0: p2 += a² - 2*a²*y    (y-- only)
  else:      p2 += 2*b²*x - 2*a²*y + a²  (x++, y--)

State: "Region 1 — slope < 1" / "Region 2 — slope > 1"
```

---

### Phase 4 — Filling Algorithms (3 total)

**Files:** `include/Visualizer/FillAlgos.h` + `src/Visualizer/FillAlgos.cpp`

#### Algorithm 7: Flood Fill (4-way)

```cpp
class FloodFill4 : public IAlgorithm {
    std::queue<Pixel> frontier;
    std::vector<std::vector<bool>> visited;
    std::vector<Pixel> filledPixels;
    int gridW, gridH;
public:
    void init(int seedX, int seedY, int w, int h);
    // step(): pop one pixel, check 4 neighbors (up/down/left/right),
    //         push unvisited valid ones to queue
    // State: "Queue: 12 pixels | Filled: 47 pixels"
};
```

Pixel colors: frontier = orange, filled = green.

#### Algorithm 8: Boundary Fill

Same pattern as FloodFill4 but uses boundary color check instead of visited array.

#### Algorithm 9: Scanline Polygon Fill

```cpp
class ScanlineFill : public IAlgorithm {
    struct Edge { float xMin, xMax; int yMax; };
    std::vector<Edge> activeEdgeTable;
    int scanY;
    std::vector<std::pair<int,int>> polygon;
public:
    void initPolygon(std::vector<std::pair<int,int>> vertices);
    // step(): scanY++, update AET, sort intersections, fill spans
    // State: "Scan Y = 15 | AET entries: 4 | Filling from x=3 to x=11"
    int getCurrentScanLine() const;
    std::vector<Edge> getActiveEdgeTable() const;
};
```

---

### Phase 5 — Clipping Algorithms (2 total)

**Files:** `include/Visualizer/ClipAlgos.h` + `src/Visualizer/ClipAlgos.cpp`

#### Algorithm 10: Cohen-Sutherland Line Clipping

```cpp
// Outcodes: INSIDE=0, LEFT=1, RIGHT=2, BOTTOM=4, TOP=8
struct ClipWindow { int xMin, yMin, xMax, yMax; };

class CohenSutherland : public IAlgorithm {
    float lx1, ly1, lx2, ly2;
    ClipWindow window;
    int code1, code2;
    enum Phase { COMPUTE_CODES, TRIVIAL_ACCEPT, TRIVIAL_REJECT,
                 CLIP_EDGE, DONE } phase;
public:
    void initLine(float x1,float y1,float x2,float y2, ClipWindow w);
    std::string getOutcodeString(int code); // 0101 → "BOTTOM-LEFT"
};
// State: "code1=0101(BOTTOM-LEFT) | code2=0000(INSIDE)"
//        "AND = 0 → Not trivially rejected → Clip LEFT edge"
```

#### Algorithm 11: Sutherland-Hodgman Polygon Clipping

```cpp
class SutherlandHodgman : public IAlgorithm {
    std::vector<std::pair<float,float>> polygon, clipped;
    int currentEdge;  // 0=left, 1=right, 2=bottom, 3=top
public:
    void initPolygon(std::vector<std::pair<float,float>> verts, ClipWindow w);
    // step(): process polygon against currentEdge, generate new vertices
    // State: "Clipping against LEFT edge (2 of 4)"
};
```

---

### Phase 6 — Transformations & 3D (4 total)

**New files:** `include/Visualizer/TransformAlgos.h` + `src/Visualizer/TransformAlgos.cpp`

#### Algorithm 12: 2D Transformations

```cpp
#include <glm/glm.hpp>

class Transform2D : public IAlgorithm {
public:
    enum TransformType { TRANSLATE, ROTATE, SCALE, COMPOSITE };
private:
    std::vector<glm::vec2> originalShape, currentShape;
    glm::mat3 currentMatrix;
    std::vector<std::pair<TransformType, glm::vec3>> transformQueue;
    int currentTransformIdx;
public:
    void initTransform(std::vector<glm::vec2> shape, ...);
    glm::mat3 getCurrentMatrix() const;
    // State: Shows 3x3 matrix updating, formula: [cos θ  -sin θ  0]
    //                                               [sin θ   cos θ  0]
    //                                               [  0       0    1]
};
```

#### Algorithm 13: Window-to-Viewport

```
Formula displayed:
  vx = vxMin + (x - wxMin) * (vxMax - vxMin) / (wxMax - wxMin)
  vy = vyMin + (y - wyMin) * (vyMax - wyMin) / (wyMax - wyMin)
Split view: LEFT = world window, RIGHT = viewport result
```

#### Algorithm 14: Painter's Algorithm

```cpp
class PaintersAlgorithm : public IAlgorithm {
    struct Polygon3D { std::vector<glm::vec3> verts; glm::vec3 color; float avgZ; };
    std::vector<Polygon3D> polygons;  // sorted by Z descending (far → near)
    int currentDrawIdx;
public:
    void initScene(std::vector<Polygon3D> scene);
    // step(): draw polygons[currentDrawIdx++]
    // State: "Drawing polygon 3/5 (Z = 2.4) | Back-to-front order"
};
```

#### Algorithm 15: Z-Buffer (Depth Buffer)

```cpp
class ZBuffer : public IAlgorithm {
    std::vector<std::vector<float>> depthBuffer;           // initialized to +∞
    std::vector<std::vector<glm::vec3>> colorBuffer;
    int currentPixelX, currentPixelY;
    std::vector</* triangle */> triangles;
public:
    std::vector<std::vector<float>> getDepthBuffer() const;    // grayscale viz
    std::vector<std::vector<glm::vec3>> getColorBuffer() const;
    // step(): test one pixel: if zNew < depthBuffer[x][y] → update both buffers
    // Side-by-side: color frame LEFT, depth map RIGHT
};
```

---

## 4. UI Layout Design

### Two-Tab Layout (Every Algorithm)

```
┌──────────────────────────────────────────────────────┐
│  Algorithm: [DDA Line ▼]                             │
│  [📖 Theory]  [🎮 Visualize]                         │
├──────────────────────────────────────────────────────┤
│  THEORY TAB:                                         │
│  Name, description, formula, complexity, advantages  │
│                                                      │
│  VISUALIZE TAB:                                      │
│  LEFT PANEL           │   RIGHT: PIXEL GRID          │
│  Inputs + Live State  │   pixels: red=plotted        │
│  Step controls        │          yellow=current step │
└──────────────────────────────────────────────────────┘
```

### Pixel Color Scheme

| State | Color |
|-------|-------|
| Previous plotted pixels | Red `(1.0, 0.2, 0.2)` |
| Current step pixel | Yellow `(1.0, 1.0, 0.0)` |
| Xiaolin Wu pixel | Red, alpha = intensity value |
| Flood fill frontier | Orange `(1.0, 0.6, 0.0)` |
| Flood fill filled | Green `(0.2, 0.8, 0.2)` |
| Inside clip region | Green tint |
| Outside clip region | Red tint |

---

## 5. Teammate Integration (How to Connect Sections)

All 3 sections share one GLFW window. The connection point is `src/main.cpp` via a Dear ImGui tab bar. **Do not touch each other's directories.**

### PaintApp (Teammate 1) — Required Interface

```cpp
// include/PaintApp/PaintApp.h
class PaintApp {
public:
    PaintApp();
    void renderUI();
    void renderCanvas(int windowWidth, int windowHeight);
};
```

After implementation, tell Parth to uncomment these lines in `main.cpp`:
```cpp
// PaintApp paintApp;
// in tab bar: paintApp.renderUI()
// in render:  paintApp.renderCanvas(w, h)
```

### GraphApp (Teammate 2) — Required Interface

```cpp
// include/GraphApp/GraphApp.h
class GraphApp {
public:
    GraphApp();
    void renderUI();
    void renderGrid(int windowWidth, int windowHeight);
};
```

### Final `main.cpp` Structure

```cpp
if (ImGui::BeginTabBar("##sections")) {
    if (ImGui::BeginTabItem("🎨 Paint App"))   { activeSection=0; paintApp.renderUI();    ImGui::EndTabItem(); }
    if (ImGui::BeginTabItem("📐 Algorithms"))  { activeSection=1; visualizer.renderUI();  ImGui::EndTabItem(); }
    if (ImGui::BeginTabItem("🗺 Graph Algos")) { activeSection=2; graphApp.renderUI();    ImGui::EndTabItem(); }
    ImGui::EndTabBar();
}
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
if (activeSection == 0) paintApp.renderCanvas(w, h);
if (activeSection == 1) visualizer.renderGrid(w, h);
if (activeSection == 2) graphApp.renderGrid(w, h);
```

---

## 6. File Status Checklist

| File | Status | Notes |
|------|--------|-------|
| `include/Visualizer/IAlgorithm.h` | ✅ Needs extension | Add AlgoState + 4 virtuals |
| `include/Visualizer/VisualizerEngine.h` | ✅ Needs refactor | unique_ptr + dropdown |
| `include/Visualizer/LineAlgos.h` | ✅ Extend | Add Bresenham + WuLine |
| `include/Visualizer/CircleAlgos.h` | 🔲 Create | Midpoint Circle, Bresenham Circle, Ellipse |
| `include/Visualizer/FillAlgos.h` | 🔲 Implement | FloodFill, Boundary, Scanline |
| `include/Visualizer/ClipAlgos.h` | 🔲 Implement | Cohen-Sutherland, Sutherland-Hodgman |
| `include/Visualizer/TransformAlgos.h` | 🔲 Create | 2D Transform, Viewport, Painter's, ZBuffer |
| `src/Visualizer/VisualizerEngine.cpp` | ✅ Needs refactor | |
| `src/Visualizer/LineAlgos.cpp` | ✅ Extend | |
| `src/Visualizer/CircleAlgos.cpp` | 🔲 Create | |
| `src/Visualizer/FillAlgos.cpp` | 🔲 Implement | |
| `src/Visualizer/ClipAlgos.cpp` | 🔲 Implement | |
| `src/Visualizer/TransformAlgos.cpp` | 🔲 Create | |
