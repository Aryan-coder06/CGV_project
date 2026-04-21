#include "../../include/Visualizer/ClipAlgos.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

// Shared formatting helpers (also in ClipAlgos.cpp — anonymous linkage keeps them separate)
static std::string sh_f2(float v) {
  std::ostringstream o;
  o << std::fixed << std::setprecision(2) << v;
  return o.str();
}
static std::string sh_ci(int v) { return std::to_string(v); }

// ================================================================
//  SutherlandHodgman — Polygon Clipping
// ================================================================

static const char* SH_EDGE_NAMES[4] = { "LEFT", "RIGHT", "BOTTOM", "TOP" };

// ---- Constructor -----------------------------------------------
SutherlandHodgman::SutherlandHodgman()
    : oCX(0), oCY(0), oSize(7), oSides(5),
      xMin(-6.0f), xMax(6.0f), yMin(-5.0f), yMax(5.0f),
      currentPhase(0), finished(false) {}

// ---- Build regular N-gon ---------------------------------------
std::vector<SutherlandHodgman::Vertex>
SutherlandHodgman::buildRegularPolygon(int cx, int cy, int size, int sides) const {
  std::vector<Vertex> poly;
  const float pi2 = 2.0f * 3.14159265f;
  float startAngle = -3.14159265f / 2.0f;
  for (int i = 0; i < sides; ++i) {
    float a = startAngle + (float)i * pi2 / (float)sides;
    poly.push_back({ (float)cx + 0.5f + (float)size * cosf(a),
                     (float)cy + 0.5f + (float)size * sinf(a) });
  }
  return poly;
}

// ---- Inside test per edge --------------------------------------
bool SutherlandHodgman::isInside(const Vertex& p, int edge) const {
  switch (edge) {
    case CLIP_LEFT:   return p.x >= xMin;
    case CLIP_RIGHT:  return p.x <= xMax;
    case CLIP_BOTTOM: return p.y >= yMin;
    case CLIP_TOP:    return p.y <= yMax;
  }
  return false;
}

// ---- Segment-edge intersection ---------------------------------
SutherlandHodgman::Vertex
SutherlandHodgman::intersect(const Vertex& a, const Vertex& b, int edge) const {
  float dx = b.x - a.x, dy = b.y - a.y;
  switch (edge) {
    case CLIP_LEFT: {
      float t = (xMin - a.x) / (std::abs(dx) < 1e-6f ? 1e-6f : dx);
      return { xMin, a.y + t * dy };
    }
    case CLIP_RIGHT: {
      float t = (xMax - a.x) / (std::abs(dx) < 1e-6f ? 1e-6f : dx);
      return { xMax, a.y + t * dy };
    }
    case CLIP_BOTTOM: {
      float t = (yMin - a.y) / (std::abs(dy) < 1e-6f ? 1e-6f : dy);
      return { a.x + t * dx, yMin };
    }
    case CLIP_TOP: {
      float t = (yMax - a.y) / (std::abs(dy) < 1e-6f ? 1e-6f : dy);
      return { a.x + t * dx, yMax };
    }
  }
  return a;
}

// ---- Sutherland-Hodgman core -----------------------------------
std::vector<SutherlandHodgman::Vertex>
SutherlandHodgman::clipAgainstEdge(const std::vector<Vertex>& poly, int edge) const {
  std::vector<Vertex> output;
  if (poly.empty()) return output;
  int n = (int)poly.size();
  for (int i = 0; i < n; ++i) {
    const Vertex& cur  = poly[i];
    const Vertex& prev = poly[(i + n - 1) % n];
    bool curIn  = isInside(cur,  edge);
    bool prevIn = isInside(prev, edge);
    if (curIn) {
      if (!prevIn) output.push_back(intersect(prev, cur, edge));
      output.push_back(cur);
    } else if (prevIn) {
      output.push_back(intersect(prev, cur, edge));
    }
  }
  return output;
}

// ---- Rasterise helper ------------------------------------------
void SutherlandHodgman::rasteriseLine(float ax, float ay, float bx, float by,
                                      std::vector<Pixel>& out, int pixType) const {
  int x0 = (int)roundf(ax), y0 = (int)roundf(ay);
  int x1 = (int)roundf(bx), y1 = (int)roundf(by);
  int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;
  for (int guard = 0; guard < 2000; ++guard) {
    out.push_back({x0, y0, 1.0f, pixType});
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += sx; }
    if (e2 <  dx) { err += dx; y0 += sy; }
  }
}

void SutherlandHodgman::rasterisePoly(const std::vector<Vertex>& poly,
                                      std::vector<Pixel>& out, int pixType) const {
  if (poly.size() < 2) return;
  for (int i = 0; i < (int)poly.size(); ++i) {
    const Vertex& a = poly[i];
    const Vertex& b = poly[(i + 1) % (int)poly.size()];
    rasteriseLine(a.x, a.y, b.x, b.y, out, pixType);
  }
}

void SutherlandHodgman::buildDisplay() {
  displayPixels.clear();
  rasterisePoly(phases[0], displayPixels, 1); // original: orange
  int p = currentPhase;
  if (p > 0 && !phases[p].empty())
    rasterisePoly(phases[p], displayPixels, 2); // clipped: green
}

// ---- init ------------------------------------------------------
void SutherlandHodgman::init(int cx, int cy, int size, int sides) {
  oCX = cx; oCY = cy; oSize = size; oSides = sides;
  currentPhase = 0;
  finished     = false;
  for (auto& ph : phases) ph.clear();
  phases[0] = buildRegularPolygon(cx, cy, size, sides);
  buildDisplay();

  lastState = AlgoState();
  lastState.totalSteps     = 4;
  lastState.currentStep    = 0;
  lastState.hasCalculation = false;
}

// ---- step ------------------------------------------------------
void SutherlandHodgman::step() {
  if (finished) return;
  int edge = currentPhase;
  std::vector<Vertex> input = phases[currentPhase];
  currentPhase++;
  phases[currentPhase] = clipAgainstEdge(input, edge);
  buildDisplay();

  lastState.calcLines.clear();
  std::string eName = SH_EDGE_NAMES[edge];
  lastState.calcLines.push_back("--- Clip against " + eName + " edge ---");
  switch (edge) {
    case CLIP_LEFT:   lastState.calcLines.push_back("  Inside rule: x >= " + sh_f2(xMin)); break;
    case CLIP_RIGHT:  lastState.calcLines.push_back("  Inside rule: x <= " + sh_f2(xMax)); break;
    case CLIP_BOTTOM: lastState.calcLines.push_back("  Inside rule: y >= " + sh_f2(yMin)); break;
    case CLIP_TOP:    lastState.calcLines.push_back("  Inside rule: y <= " + sh_f2(yMax)); break;
  }
  lastState.calcLines.push_back("  Input  verts: " + sh_ci((int)input.size()));
  lastState.calcLines.push_back("  Output verts: " + sh_ci((int)phases[currentPhase].size()));
  lastState.calcLines.push_back("");
  lastState.calcLines.push_back("  Cases (S=prev, P=curr):");
  lastState.calcLines.push_back("    S in,  P in   -> output P");
  lastState.calcLines.push_back("    S out, P in   -> output I, P");
  lastState.calcLines.push_back("    S in,  P out  -> output I");
  lastState.calcLines.push_back("    S out, P out  -> nothing");
  lastState.calcLines.push_back("");

  int shown = 0;
  for (auto& v : phases[currentPhase]) {
    if (++shown > 8) { lastState.calcLines.push_back("  ..."); break; }
    lastState.calcLines.push_back("  >> (" + sh_f2(v.x) + ", " + sh_f2(v.y) + ")");
  }

  if (phases[currentPhase].empty())
    lastState.currentPixelInfo = ">> Polygon fully clipped — nothing remains!";
  else
    lastState.currentPixelInfo = ">> " + sh_ci((int)phases[currentPhase].size()) +
                                 " vertices after " + eName + " clip";

  lastState.hasCalculation = true;
  lastState.currentStep    = currentPhase;
  lastState.totalSteps     = 4;
  if (currentPhase == 4) finished = true;
}

// ---- stepK, runToCompletion, reset -----------------------------
void SutherlandHodgman::stepK(int k) {
  for (int i = 0; i < k && !finished; ++i) step();
  lastState.hasCalculation   = false;
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";
}

void SutherlandHodgman::runToCompletion() {
  while (!finished) step();
  lastState.hasCalculation   = false;
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";
}

void SutherlandHodgman::reset()           { init(oCX, oCY, oSize, oSides); }
bool SutherlandHodgman::isFinished() const { return finished; }

std::vector<Pixel> SutherlandHodgman::getHighlightedPixels() const { return displayPixels; }
AlgoState          SutherlandHodgman::getCurrentState()      const { return lastState; }

std::vector<std::string> SutherlandHodgman::getInitInfo() const {
  return {
    "Polygon: " + sh_ci(oSides) + "-gon  centre=(" + sh_ci(oCX) + "," + sh_ci(oCY) +
        ")  size=" + sh_ci(oSize),
    "Clip Window: x=[" + sh_f2(xMin) + ", " + sh_f2(xMax) +
        "]  y=[" + sh_f2(yMin) + ", " + sh_f2(yMax) + "]",
    "Input vertices: " + sh_ci((int)phases[0].size()),
    "Steps: 4  (LEFT -> RIGHT -> BOTTOM -> TOP)",
  };
}

std::vector<std::string> SutherlandHodgman::getCurrentVars() const {
  static const char* pNames[] = {
    "Initial", "After LEFT", "After RIGHT", "After BOTTOM", "After TOP"
  };
  int p = std::min(currentPhase, 4);
  return {
    "Phase    : " + std::string(pNames[p]),
    "Vertices : " + sh_ci((int)phases[p].size()),
    "Clip x   : [" + sh_f2(xMin) + ", " + sh_f2(xMax) + "]",
    "Clip y   : [" + sh_f2(yMin) + ", " + sh_f2(yMax) + "]",
    "Done     : " + std::string(finished ? "YES" : "NO"),
  };
}

std::string SutherlandHodgman::getName() const {
  return "Sutherland-Hodgman Polygon Clipping";
}

std::string SutherlandHodgman::getTheory() const {
  return
    "Sutherland-Hodgman Polygon Clipping\n"
    "=====================================\n\n"
    "WHAT IS IT?\n"
    "  An algorithm to clip a polygon against a convex clip region.\n"
    "  Unlike Cohen-Sutherland (lines), this clips an entire polygon,\n"
    "  producing a new clipped polygon as output.\n\n"
    "CORE IDEA:\n"
    "  Process the clip window one edge at a time (4 for a rectangle).\n"
    "  At each stage, the current polygon is passed through a clipper\n"
    "  for that single edge, producing a new smaller polygon.\n\n"
    "ALGORITHM  (for each clip boundary):\n"
    "  For each edge (S, P) where S=previous vertex, P=current:\n"
    "  Case 1: S inside,  P inside   -> output P\n"
    "  Case 2: S outside, P inside   -> output Intersection I, then P\n"
    "  Case 3: S inside,  P outside  -> output Intersection I only\n"
    "  Case 4: S outside, P outside  -> output nothing\n\n"
    "INTERSECTION FORMULAS:\n"
    "  Vertical boundary  x = X:\n"
    "    t = (X - Sx) / (Px - Sx)\n"
    "    I = (X,  Sy + t*(Py - Sy))\n"
    "  Horizontal boundary  y = Y:\n"
    "    t = (Y - Sy) / (Py - Sy)\n"
    "    I = (Sx + t*(Px - Sx),  Y)\n\n"
    "ORDER OF EDGES:\n"
    "  Step 1:  LEFT   (x >= xMin)\n"
    "  Step 2:  RIGHT  (x <= xMax)\n"
    "  Step 3:  BOTTOM (y >= yMin)\n"
    "  Step 4:  TOP    (y <= yMax)\n\n"
    "ADVANTAGES:\n"
    "+   Handles polygons of any shape / number of vertices\n"
    "+   O(n) per edge, O(4n) total for rectangle\n"
    "+   Output is always a valid (possibly degenerate) polygon\n"
    "+   Naturally extends to 3D (clip planes)\n\n"
    "DISADVANTAGES:\n"
    "-   Concave polygons may get extra edges along clip boundary\n"
    "-   Use Weiler-Atherton for correct concave polygon clipping\n\n"
    "COMPLEXITY:\n"
    "  Time:  O(n) per clip edge, O(4n) total\n"
    "  Space: O(n) for output polygon\n";
}
