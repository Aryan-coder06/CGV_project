#include "../../include/Visualizer/TransformAlgos.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

// ---- Formatting helpers ----------------------------------------
static std::string tf2(float v) {
  std::ostringstream o;
  o << std::fixed << std::setprecision(2) << v;
  return o.str();
}
static std::string ti(int v) { return std::to_string(v); }

static const char* PHASE_NAMES[4] = {
  "Original", "After Translation", "After Rotation", "After Scaling"
};

// ---- Constructor -----------------------------------------------
Transform2D::Transform2D()
    : oCX(0), oCY(0), oSize(8), oSides(5),
      tx(3.0f), ty(2.0f),
      rotAngleDeg(45.0f), pivotX(0.0f), pivotY(0.0f),
      sx(1.5f), sy(1.5f),
      currentPhase(0), finished(false) {}

// ---- Build regular N-gon ----------------------------------------
std::vector<Transform2D::Vertex>
Transform2D::buildPolygon(int cx, int cy, int size, int sides) const {
  std::vector<Vertex> poly;
  const float pi2 = 2.0f * 3.14159265f;
  float startAngle = -3.14159265f / 2.0f;
  for (int i = 0; i < sides; ++i) {
    float a = startAngle + (float)i * pi2 / (float)sides;
    poly.push_back({ (float)cx + (float)size * cosf(a),
                     (float)cy + (float)size * sinf(a) });
  }
  return poly;
}

// ---- Rasterise helpers -----------------------------------------
void Transform2D::rasteriseLine(float ax, float ay, float bx, float by,
                                 std::vector<Pixel>& out, int pixType) const {
  int x0 = (int)roundf(ax), y0 = (int)roundf(ay);
  int x1 = (int)roundf(bx), y1 = (int)roundf(by);
  int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
  int sx2 = (x0 < x1) ? 1 : -1, sy2 = (y0 < y1) ? 1 : -1;
  int err = dx - dy;
  for (int g = 0; g < 2000; ++g) {
    out.push_back({x0, y0, 1.0f, pixType});
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += sx2; }
    if (e2 <  dx) { err += dx; y0 += sy2; }
  }
}

void Transform2D::rasterisePoly(const std::vector<Vertex>& poly,
                                 std::vector<Pixel>& out, int pixType) const {
  if (poly.size() < 2) return;
  for (int i = 0; i < (int)poly.size(); ++i) {
    const Vertex& a = poly[i];
    const Vertex& b = poly[(i + 1) % (int)poly.size()];
    rasteriseLine(a.x, a.y, b.x, b.y, out, pixType);
  }
}

void Transform2D::buildDisplay() {
  displayPixels.clear();
  rasterisePoly(phases[0], displayPixels, 1);            // original: orange
  if (currentPhase > 0)
    rasterisePoly(phases[currentPhase], displayPixels, 2); // current: green
}

// ---- init -------------------------------------------------------
void Transform2D::init(int cx, int cy, int size, int sides) {
  oCX = cx; oCY = cy; oSize = size; oSides = sides;
  pivotX = (float)cx; pivotY = (float)cy; // pivot = centroid by default
  currentPhase = 0;
  finished     = false;
  for (auto& p : phases) p.clear();
  phases[0] = buildPolygon(cx, cy, size, sides);
  buildDisplay();

  lastState = AlgoState();
  lastState.totalSteps     = 3;
  lastState.currentStep    = 0;
  lastState.hasCalculation = false;
}

// ---- Translation step ------------------------------------------
void Transform2D::doTranslation() {
  const auto& inp = phases[0];
  auto& out = phases[1];
  out.clear();
  lastState.calcLines.clear();
  lastState.calcLines.push_back("--- Step 1: Translation ---");
  lastState.calcLines.push_back("  Matrix form:");
  lastState.calcLines.push_back("  | 1  0  tx |   | x |   | x + tx |");
  lastState.calcLines.push_back("  | 0  1  ty | * | y | = | y + ty |");
  lastState.calcLines.push_back("  | 0  0   1 |   | 1 |   |   1    |");
  lastState.calcLines.push_back("");
  lastState.calcLines.push_back("  tx = " + tf2(tx) + "   ty = " + tf2(ty));
  lastState.calcLines.push_back("");

  int shown = 0;
  for (const auto& v : inp) {
    float nx = v.x + tx, ny = v.y + ty;
    out.push_back({nx, ny});
    if (++shown <= 4) {
      lastState.calcLines.push_back("  (" + tf2(v.x) + ", " + tf2(v.y) + ")");
      lastState.calcLines.push_back("  => (" + tf2(v.x) + "+" + tf2(tx) +
                                    ", " + tf2(v.y) + "+" + tf2(ty) + ")");
      lastState.calcLines.push_back("  => (" + tf2(nx) + ", " + tf2(ny) + ")");
      if (shown < (int)inp.size() && shown < 4) lastState.calcLines.push_back("");
    }
  }
  if ((int)inp.size() > 4) lastState.calcLines.push_back("  ...");
  lastState.currentPixelInfo = ">> Translated by (" + tf2(tx) + ", " + tf2(ty) + ")";
}

// ---- Rotation step ---------------------------------------------
void Transform2D::doRotation() {
  const auto& inp = phases[1];
  auto& out = phases[2];
  out.clear();
  float rad = rotAngleDeg * 3.14159265f / 180.0f;
  float cosA = cosf(rad), sinA = sinf(rad);

  lastState.calcLines.clear();
  lastState.calcLines.push_back("--- Step 2: Rotation about Pivot ---");
  lastState.calcLines.push_back("  Pivot: (" + tf2(pivotX) + ", " + tf2(pivotY) + ")");
  lastState.calcLines.push_back("  Angle: " + tf2(rotAngleDeg) + " deg");
  lastState.calcLines.push_back("  cos(" + tf2(rotAngleDeg) + ") = " + tf2(cosA));
  lastState.calcLines.push_back("  sin(" + tf2(rotAngleDeg) + ") = " + tf2(sinA));
  lastState.calcLines.push_back("");
  lastState.calcLines.push_back("  Matrix form (around origin):");
  lastState.calcLines.push_back("  | cos  -sin  0 |");
  lastState.calcLines.push_back("  | sin   cos  0 |");
  lastState.calcLines.push_back("  |  0     0   1 |");
  lastState.calcLines.push_back("");
  lastState.calcLines.push_back("  x' = px + (x-px)*cos - (y-py)*sin");
  lastState.calcLines.push_back("  y' = py + (x-px)*sin + (y-py)*cos");
  lastState.calcLines.push_back("");

  int shown = 0;
  for (const auto& v : inp) {
    float dx2 = v.x - pivotX, dy2 = v.y - pivotY;
    float nx = pivotX + dx2 * cosA - dy2 * sinA;
    float ny = pivotY + dx2 * sinA + dy2 * cosA;
    out.push_back({nx, ny});
    if (++shown <= 3) {
      lastState.calcLines.push_back("  P=(" + tf2(v.x) + "," + tf2(v.y) + ")");
      lastState.calcLines.push_back("  dx=" + tf2(dx2) + " dy=" + tf2(dy2));
      lastState.calcLines.push_back("  => (" + tf2(nx) + ", " + tf2(ny) + ")");
      if (shown < (int)inp.size() && shown < 3) lastState.calcLines.push_back("");
    }
  }
  if ((int)inp.size() > 3) lastState.calcLines.push_back("  ...");
  lastState.currentPixelInfo = ">> Rotated " + tf2(rotAngleDeg) + " deg around (" +
                               tf2(pivotX) + "," + tf2(pivotY) + ")";
}

// ---- Scaling step ----------------------------------------------
void Transform2D::doScaling() {
  const auto& inp = phases[2];
  auto& out = phases[3];
  out.clear();

  lastState.calcLines.clear();
  lastState.calcLines.push_back("--- Step 3: Scaling about Pivot ---");
  lastState.calcLines.push_back("  Pivot: (" + tf2(pivotX) + ", " + tf2(pivotY) + ")");
  lastState.calcLines.push_back("  sx = " + tf2(sx) + "   sy = " + tf2(sy));
  lastState.calcLines.push_back("");
  lastState.calcLines.push_back("  Matrix form:");
  lastState.calcLines.push_back("  | sx  0   px*(1-sx) |");
  lastState.calcLines.push_back("  |  0  sy  py*(1-sy) |");
  lastState.calcLines.push_back("  |  0   0      1     |");
  lastState.calcLines.push_back("");
  lastState.calcLines.push_back("  x' = px + (x - px) * sx");
  lastState.calcLines.push_back("  y' = py + (y - py) * sy");
  lastState.calcLines.push_back("");

  int shown = 0;
  for (const auto& v : inp) {
    float nx = pivotX + (v.x - pivotX) * sx;
    float ny = pivotY + (v.y - pivotY) * sy;
    out.push_back({nx, ny});
    if (++shown <= 3) {
      lastState.calcLines.push_back("  P=(" + tf2(v.x) + "," + tf2(v.y) + ")");
      lastState.calcLines.push_back("  => (" + tf2(pivotX) + " + (" + tf2(v.x) + "-" +
                                    tf2(pivotX) + ")*" + tf2(sx) + ",");
      lastState.calcLines.push_back("      " + tf2(pivotY) + " + (" + tf2(v.y) + "-" +
                                    tf2(pivotY) + ")*" + tf2(sy) + ")");
      lastState.calcLines.push_back("  => (" + tf2(nx) + ", " + tf2(ny) + ")");
      if (shown < (int)inp.size() && shown < 3) lastState.calcLines.push_back("");
    }
  }
  if ((int)inp.size() > 3) lastState.calcLines.push_back("  ...");
  lastState.currentPixelInfo = ">> Scaled by (" + tf2(sx) + ", " + tf2(sy) +
                               ") from pivot (" + tf2(pivotX) + "," + tf2(pivotY) + ")";
}

// ---- Public interface ------------------------------------------
void Transform2D::step() {
  if (finished) return;
  currentPhase++;
  switch (currentPhase) {
    case 1: doTranslation(); break;
    case 2: doRotation();    break;
    case 3: doScaling();     break;
  }
  buildDisplay();
  lastState.hasCalculation = true;
  lastState.currentStep    = currentPhase;
  lastState.totalSteps     = 3;
  if (currentPhase == 3) finished = true;
}

void Transform2D::stepK(int k) {
  for (int i = 0; i < k && !finished; ++i) step();
  lastState.hasCalculation   = false;
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";
}

void Transform2D::runToCompletion() {
  while (!finished) step();
  lastState.hasCalculation   = false;
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";
}

void Transform2D::reset() { init(oCX, oCY, oSize, oSides); }
bool Transform2D::isFinished() const { return finished; }

std::vector<Pixel> Transform2D::getHighlightedPixels() const { return displayPixels; }
AlgoState          Transform2D::getCurrentState()      const { return lastState; }

std::vector<std::string> Transform2D::getInitInfo() const {
  return {
    "Polygon: " + ti(oSides) + "-gon  centre=(" + ti(oCX) + "," + ti(oCY) + ")  size=" + ti(oSize),
    "Translation: tx=" + tf2(tx) + "  ty=" + tf2(ty),
    "Rotation:    " + tf2(rotAngleDeg) + " deg  pivot=(" + tf2(pivotX) + "," + tf2(pivotY) + ")",
    "Scaling:     sx=" + tf2(sx) + "  sy=" + tf2(sy),
    "Steps: 3  (1=Translate, 2=Rotate, 3=Scale)",
  };
}

std::vector<std::string> Transform2D::getCurrentVars() const {
  int p = std::min(currentPhase, 3);
  return {
    "Phase    : " + std::string(PHASE_NAMES[p]),
    "Vertices : " + ti((int)phases[p].size()),
    "tx,ty    : (" + tf2(tx) + ", " + tf2(ty) + ")",
    "Angle    : " + tf2(rotAngleDeg) + " deg",
    "sx,sy    : (" + tf2(sx) + ", " + tf2(sy) + ")",
    "Done     : " + std::string(finished ? "YES" : "NO"),
  };
}

std::string Transform2D::getName() const {
  return "2D Transformations (Translation, Rotation, Scaling)";
}

std::string Transform2D::getTheory() const {
  return
    "2D Geometric Transformations\n"
    "==============================\n\n"
    "OVERVIEW:\n"
    "  In Computer Graphics, objects are moved, rotated and resized\n"
    "  using 2D transformation matrices. Using homogeneous coordinates\n"
    "  (x, y, 1), all three operations become simple matrix multiplications.\n\n"
    "1. TRANSLATION:\n"
    "  Moves a point by (tx, ty).\n"
    "  Matrix:\n"
    "    | 1  0  tx |   | x |   | x + tx |\n"
    "    | 0  1  ty | * | y | = | y + ty |\n"
    "    | 0  0   1 |   | 1 |   |   1    |\n"
    "  Result: x' = x + tx,   y' = y + ty\n\n"
    "2. ROTATION (about pivot point px, py):\n"
    "  Steps:\n"
    "    a. Translate pivot to origin\n"
    "    b. Rotate by angle theta\n"
    "    c. Translate back\n"
    "  Combined matrix:\n"
    "    | cos  -sin  px(1-cos)+py*sin |\n"
    "    | sin   cos  py(1-cos)-px*sin |\n"
    "    |  0     0           1        |\n"
    "  Formulas:\n"
    "    dx = x - px,  dy = y - py\n"
    "    x' = px + dx*cos(theta) - dy*sin(theta)\n"
    "    y' = py + dx*sin(theta) + dy*cos(theta)\n\n"
    "3. SCALING (about pivot point px, py):\n"
    "  Scales by (sx, sy) relative to pivot:\n"
    "    x' = px + (x - px) * sx\n"
    "    y' = py + (y - py) * sy\n"
    "  Matrix:\n"
    "    | sx   0   px*(1-sx) |\n"
    "    |  0   sy  py*(1-sy) |\n"
    "    |  0    0      1     |\n\n"
    "COMPOSITE TRANSFORMATIONS:\n"
    "  All three can be combined into a single matrix:\n"
    "    M = S * R * T\n"
    "  Applied as  P' = M * P\n"
    "  Order matters! T then R then S is the standard convention.\n\n"
    "HOMOGENEOUS COORDINATES:\n"
    "  Adding a 3rd coordinate '1' allows all transforms\n"
    "  (including translation) to be represented as matrix multiply.\n"
    "  This unifies the math and enables composite transforms.\n\n"
    "VISUALIZATION (this tool):\n"
    "  Step 1: Translation  — polygon shifts by (tx, ty)\n"
    "  Step 2: Rotation     — polygon rotates about pivot\n"
    "  Step 3: Scaling      — polygon grows/shrinks about pivot\n\n"
    "ADVANTAGES OF MATRIX FORM:\n"
    "+   Composite transforms in O(1)\n"
    "+   GPU hardware accelerated with matrix pipelines\n"
    "+   Same math works in 3D (4x4 matrices)\n\n"
    "COMPLEXITY:\n"
    "  Time:  O(n) per transform where n = vertex count\n"
    "  Space: O(n)\n";
}
