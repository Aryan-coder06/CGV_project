#include "../../include/Visualizer/WindowViewportAlgo.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

static std::string wf2(float v) {
  std::ostringstream o;
  o << std::fixed << std::setprecision(2) << v;
  return o.str();
}
static std::string wi(int v) { return std::to_string(v); }

// ---- Constructor -----------------------------------------------
WindowViewport::WindowViewport()
    : wxMin(-10.f), wxMax(10.f), wyMin(-10.f), wyMax(10.f),
      vxMin(20.f),  vxMax(40.f),  vyMin(5.f),  vyMax(25.f),
      oCX(0), oCY(0), oSize(6), oSides(5),
      sx(1.f), sy(1.f),
      currentPhase(0), finished(false) {}

// ---- Build regular polygon in world coords ---------------------
std::vector<WindowViewport::Vertex>
WindowViewport::buildPolygon(int cx, int cy, int size, int sides) const {
  std::vector<Vertex> poly;
  const float pi2 = 2.f * 3.14159265f;
  float sa = -3.14159265f / 2.f;
  for (int i = 0; i < sides; ++i) {
    float a = sa + (float)i * pi2 / (float)sides;
    poly.push_back({ (float)cx + (float)size * cosf(a),
                     (float)cy + (float)size * sinf(a) });
  }
  return poly;
}

// ---- Bresenham rasteriser --------------------------------------
void WindowViewport::rasteriseLine(float ax, float ay, float bx, float by,
                                    std::vector<Pixel>& out, int pixType) const {
  int x0 = (int)roundf(ax), y0 = (int)roundf(ay);
  int x1 = (int)roundf(bx), y1 = (int)roundf(by);
  int dx = std::abs(x1-x0), dy = std::abs(y1-y0);
  int sx2 = x0<x1?1:-1, sy2 = y0<y1?1:-1;
  int err = dx - dy;
  for (int g = 0; g < 3000; ++g) {
    out.push_back({x0, y0, 1.f, pixType});
    if (x0==x1 && y0==y1) break;
    int e2 = 2*err;
    if (e2 > -dy) { err -= dy; x0 += sx2; }
    if (e2 <  dx) { err += dx; y0 += sy2; }
  }
}

void WindowViewport::rasterisePoly(const std::vector<Vertex>& poly,
                                    std::vector<Pixel>& out, int pixType) const {
  if (poly.size() < 2) return;
  for (int i = 0; i < (int)poly.size(); ++i) {
    const Vertex& a = poly[i];
    const Vertex& b = poly[(i+1)%(int)poly.size()];
    rasteriseLine(a.x, a.y, b.x, b.y, out, pixType);
  }
}

void WindowViewport::buildDisplay() {
  displayPixels.clear();
  rasterisePoly(worldPoly, displayPixels, 1);   // world polygon: orange
  if (currentPhase >= 2 && !viewPoly.empty())
    rasterisePoly(viewPoly, displayPixels, 2);   // viewport polygon: green
}

// ---- init ------------------------------------------------------
void WindowViewport::init(int cx, int cy, int size, int sides) {
  oCX = cx; oCY = cy; oSize = size; oSides = sides;
  currentPhase = 0;
  finished     = false;
  worldPoly    = buildPolygon(cx, cy, size, sides);
  viewPoly.clear();
  sx = 1.f; sy = 1.f;
  buildDisplay();

  lastState = AlgoState();
  lastState.totalSteps     = 2;
  lastState.currentStep    = 0;
  lastState.hasCalculation = false;
}

// ---- Step 1: Compute scale factors -----------------------------
static void stepScaleFactors(WindowViewport& self,
                              float wxMin, float wxMax, float wyMin, float wyMax,
                              float vxMin, float vxMax, float vyMin, float vyMax,
                              float& sx, float& sy, AlgoState& st) {
  sx = (vxMax - vxMin) / (wxMax - wxMin);
  sy = (vyMax - vyMin) / (wyMax - wyMin);

  st.calcLines.clear();
  st.calcLines.push_back("--- Step 1: Compute Scale Factors ---");
  st.calcLines.push_back("");
  st.calcLines.push_back("  sx = (Vxmax - Vxmin) / (Wxmax - Wxmin)");
  st.calcLines.push_back("     = (" + wf2(vxMax) + " - " + wf2(vxMin) + ") / (" +
                          wf2(wxMax) + " - " + wf2(wxMin) + ")");
  st.calcLines.push_back("     = " + wf2(vxMax-vxMin) + " / " + wf2(wxMax-wxMin));
  st.calcLines.push_back("     = " + wf2(sx));
  st.calcLines.push_back("");
  st.calcLines.push_back("  sy = (Vymax - Vymin) / (Wymax - Wymin)");
  st.calcLines.push_back("     = (" + wf2(vyMax) + " - " + wf2(vyMin) + ") / (" +
                          wf2(wyMax) + " - " + wf2(wyMin) + ")");
  st.calcLines.push_back("     = " + wf2(vyMax-vyMin) + " / " + wf2(wyMax-wyMin));
  st.calcLines.push_back("     = " + wf2(sy));
  st.calcLines.push_back("");
  st.calcLines.push_back("  Aspect ratio: " +
    wf2((vxMax-vxMin)/(vyMax-vyMin)) + " : 1 (viewport)");
  st.currentPixelInfo = ">> sx=" + wf2(sx) + "  sy=" + wf2(sy);
  st.hasCalculation   = true;
  st.currentStep      = 1;
  st.totalSteps       = 2;
}

// ---- Step 2: Apply mapping to vertices -------------------------
static void stepApplyMapping(const std::vector<WindowViewport::Vertex>& wPoly,
                              std::vector<WindowViewport::Vertex>& vPoly,
                              float wxMin, float wyMin,
                              float vxMin, float vyMin,
                              float sx,    float sy,
                              AlgoState& st) {
  vPoly.clear();
  st.calcLines.clear();
  st.calcLines.push_back("--- Step 2: Apply Mapping ---");
  st.calcLines.push_back("");
  st.calcLines.push_back("  Formulas:");
  st.calcLines.push_back("    Xv = Vxmin + (Xw - Wxmin) * sx");
  st.calcLines.push_back("    Yv = Vymin + (Yw - Wymin) * sy");
  st.calcLines.push_back("");
  st.calcLines.push_back("  sx=" + wf2(sx) + "  sy=" + wf2(sy));
  st.calcLines.push_back("");

  int shown = 0;
  for (const auto& v : wPoly) {
    float xv = vxMin + (v.x - wxMin) * sx;
    float yv = vyMin + (v.y - wyMin) * sy;
    vPoly.push_back({xv, yv});
    if (++shown <= 5) {
      st.calcLines.push_back("  W(" + wf2(v.x) + ", " + wf2(v.y) + ")");
      st.calcLines.push_back("  Xv = " + wf2(vxMin) + " + (" + wf2(v.x) + "-" +
                              wf2(wxMin) + ")*" + wf2(sx) + " = " + wf2(xv));
      st.calcLines.push_back("  Yv = " + wf2(vyMin) + " + (" + wf2(v.y) + "-" +
                              wf2(wyMin) + ")*" + wf2(sy) + " = " + wf2(yv));
      st.calcLines.push_back("  => V(" + wf2(xv) + ", " + wf2(yv) + ")");
      if (shown < (int)wPoly.size() && shown < 5) st.calcLines.push_back("");
    }
  }
  if ((int)wPoly.size() > 5) st.calcLines.push_back("  ...");

  st.currentPixelInfo = ">> Mapped " + wi((int)vPoly.size()) + " vertices to viewport";
  st.hasCalculation   = true;
  st.currentStep      = 2;
  st.totalSteps       = 2;
}

// ---- Public step/run -------------------------------------------
void WindowViewport::step() {
  if (finished) return;
  currentPhase++;
  if (currentPhase == 1) {
    stepScaleFactors(*this, wxMin, wxMax, wyMin, wyMax,
                     vxMin, vxMax, vyMin, vyMax, sx, sy, lastState);
  } else if (currentPhase == 2) {
    stepApplyMapping(worldPoly, viewPoly, wxMin, wyMin,
                     vxMin, vyMin, sx, sy, lastState);
    buildDisplay();
    finished = true;
  }
}

void WindowViewport::stepK(int k) {
  for (int i = 0; i < k && !finished; ++i) step();
  lastState.hasCalculation   = false;
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";
}

void WindowViewport::runToCompletion() {
  while (!finished) step();
  lastState.hasCalculation   = false;
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";
}

void WindowViewport::reset()            { init(oCX, oCY, oSize, oSides); }
bool WindowViewport::isFinished() const  { return finished; }

std::vector<Pixel> WindowViewport::getHighlightedPixels() const { return displayPixels; }
AlgoState          WindowViewport::getCurrentState()      const { return lastState; }

std::vector<std::string> WindowViewport::getInitInfo() const {
  return {
    "Polygon: " + wi(oSides) + "-gon  centre=(" + wi(oCX) + "," + wi(oCY) +
        ")  size=" + wi(oSize),
    "World Window: x=[" + wf2(wxMin) + ", " + wf2(wxMax) + "]"
        "  y=[" + wf2(wyMin) + ", " + wf2(wyMax) + "]",
    "Viewport:     x=[" + wf2(vxMin) + ", " + wf2(vxMax) + "]"
        "  y=[" + wf2(vyMin) + ", " + wf2(vyMax) + "]",
    "Steps: 2  (1=Scale factors, 2=Apply mapping)",
  };
}

std::vector<std::string> WindowViewport::getCurrentVars() const {
  return {
    "Phase : " + wi(currentPhase) + " / 2",
    "sx    : " + wf2(sx),
    "sy    : " + wf2(sy),
    "World : [" + wf2(wxMin) + "," + wf2(wxMax) + "] x [" + wf2(wyMin) + "," + wf2(wyMax) + "]",
    "View  : [" + wf2(vxMin) + "," + wf2(vxMax) + "] x [" + wf2(vyMin) + "," + wf2(vyMax) + "]",
    "Done  : " + std::string(finished ? "YES" : "NO"),
  };
}

std::string WindowViewport::getName() const {
  return "Window-to-Viewport Transformation";
}

std::string WindowViewport::getTheory() const {
  return
    "Window-to-Viewport Transformation\n"
    "====================================\n\n"
    "WHAT IS IT?\n"
    "  The process of mapping objects from 'world coordinates'\n"
    "  (the conceptual space where objects are defined) to\n"
    "  'screen/viewport coordinates' (the actual pixel space on screen).\n\n"
    "KEY CONCEPTS:\n"
    "  Window   - A rectangular region in world coordinates that defines\n"
    "             what part of the scene is visible.\n"
    "  Viewport - A rectangular region on screen where the window content\n"
    "             is displayed (measured in screen/device coordinates).\n\n"
    "TRANSFORMATION FORMULAS:\n"
    "  First compute scale factors:\n"
    "    sx = (Vxmax - Vxmin) / (Wxmax - Wxmin)\n"
    "    sy = (Vymax - Vymin) / (Wymax - Wymin)\n\n"
    "  Then map each world point (Xw, Yw) to viewport (Xv, Yv):\n"
    "    Xv = Vxmin + (Xw - Wxmin) * sx\n"
    "    Yv = Vymin + (Yw - Wymin) * sy\n\n"
    "DERIVATION:\n"
    "  We want a linear map f such that:\n"
    "    f(Wxmin) = Vxmin  and  f(Wxmax) = Vxmax\n"
    "  Solving: f(x) = Vxmin + (x - Wxmin) * (Vxmax-Vxmin)/(Wxmax-Wxmin)\n"
    "  Same derivation applies for y.\n\n"
    "WORKED EXAMPLE:\n"
    "  Window: x=[-10,10]  y=[-10,10]\n"
    "  Viewport: x=[0,400]  y=[0,300]\n"
    "  sx = (400-0)/(10-(-10)) = 400/20 = 20\n"
    "  sy = (300-0)/(10-(-10)) = 300/20 = 15\n"
    "  Point (-5, 5) maps to:\n"
    "    Xv = 0 + (-5-(-10)) * 20 = 0 + 5*20 = 100\n"
    "    Yv = 0 + (5-(-10))  * 15 = 0 + 15*15 = 225\n\n"
    "ASPECT RATIO:\n"
    "  If sx != sy, the image is distorted (stretched/squashed).\n"
    "  To preserve aspect ratio, use: sx = sy = min(sx, sy).\n\n"
    "INVERSE TRANSFORMATION (screen→world):\n"
    "    Xw = Wxmin + (Xv - Vxmin) / sx\n"
    "    Yw = Wymin + (Yv - Vymin) / sy\n"
    "  Used for picking/selection: click at (Xv,Yv) → world (Xw,Yw).\n\n"
    "ZOOMING & PANNING:\n"
    "  Zoom in:  shrink the window (smaller world region → fills same viewport)\n"
    "  Zoom out: grow the window\n"
    "  Pan:      translate the window (shift its min/max equally)\n\n"
    "ADVANTAGES:\n"
    "+   Decouples world coordinates from screen coordinates\n"
    "+   Enables zoom, pan, and multi-viewport layouts\n"
    "+   Foundation of every 2D/3D rendering pipeline\n\n"
    "COMPLEXITY:\n"
    "  Time:  O(n) where n = number of vertices\n"
    "  Space: O(n)\n";
}
