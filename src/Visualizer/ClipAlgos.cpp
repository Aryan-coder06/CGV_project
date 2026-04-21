#include "../../include/Visualizer/ClipAlgos.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

// ----------------------------------------------------------------
// Helper: format float to 2 decimal places
// ----------------------------------------------------------------
static std::string cf2(float v) {
  std::ostringstream o;
  o << std::fixed << std::setprecision(2) << v;
  return o.str();
}
static std::string ci(int v) { return std::to_string(v); }
static std::string cf0(float v) { return std::to_string((int)std::round(v)); }

// ----------------------------------------------------------------
// Outcode bits: which sides of the clip window a point is outside
// ----------------------------------------------------------------
int CohenSutherland::computeOutcode(float x, float y) const {
  int code = INSIDE;
  if (x < xMin)      code |= LEFT;
  else if (x > xMax) code |= RIGHT;
  if (y < yMin)      code |= BOTTOM;
  else if (y > yMax) code |= TOP;
  return code;
}

// ----------------------------------------------------------------
// Rasterise a floating-point segment into pixels using Bresenham
// ----------------------------------------------------------------
void CohenSutherland::rasteriseLine(float ax, float ay, float bx, float by,
                                    std::vector<Pixel>& out, int pixType) const {
  int x0 = (int)std::round(ax), y0 = (int)std::round(ay);
  int x1 = (int)std::round(bx), y1 = (int)std::round(by);
  int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;
  while (true) {
    out.push_back({x0, y0, 1.0f, pixType});
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += sx; }
    if (e2 <  dx) { err += dx; y0 += sy; }
  }
}

// ----------------------------------------------------------------
// Merge original + clipped pixels for display
// ----------------------------------------------------------------
void CohenSutherland::buildDisplay() {
  displayPixels.clear();
  // Draw original line in orange (type 1) first
  for (auto& p : originalPixels) displayPixels.push_back(p);
  // Overwrite accepted region in green (type 2)
  for (auto& p : clippedPixels)  displayPixels.push_back(p);
}

// ----------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------
CohenSutherland::CohenSutherland()
    : ox1(0), oy1(0), ox2(14), oy2(10),
      x1(0), y1(0), x2(14), y2(10),
      xMin(-6), xMax(6), yMin(-5), yMax(5),
      outcode1(0), outcode2(0),
      accepted(false), rejected(false), finished(false), currentStep(0) {}

// ----------------------------------------------------------------
// init — called when user presses "Apply & Reset"
// ----------------------------------------------------------------
void CohenSutherland::init(int lx1, int ly1, int lx2, int ly2) {
  ox1 = (float)lx1; oy1 = (float)ly1;
  ox2 = (float)lx2; oy2 = (float)ly2;
  x1  = ox1; y1 = oy1;
  x2  = ox2; y2 = oy2;

  accepted = false;
  rejected = false;
  finished = false;
  currentStep = 0;

  clippedPixels.clear();
  originalPixels.clear();
  displayPixels.clear();
  rasteriseLine(ox1, oy1, ox2, oy2, originalPixels, 1); // orange

  outcode1 = computeOutcode(x1, y1);
  outcode2 = computeOutcode(x2, y2);

  lastState = AlgoState();
  // total steps: we can't know exactly, but max is 4 iterations → cap at 6
  lastState.totalSteps = 6;
  lastState.hasCalculation = false;

  buildDisplay();
}

// ----------------------------------------------------------------
// One Cohen-Sutherland iteration (step)
// ----------------------------------------------------------------
void CohenSutherland::doClipStep() {
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";

  // Both inside?
  if (!(outcode1 | outcode2)) {
    accepted = true; finished = true;
    clippedPixels.clear();
    rasteriseLine(x1, y1, x2, y2, clippedPixels, 2); // green
    buildDisplay();
    lastState.calcLines.push_back("--- Trivial Accept ---");
    lastState.calcLines.push_back("  OC1=" + ci(outcode1) + "  OC2=" + ci(outcode2));
    lastState.calcLines.push_back("  Both endpoints INSIDE the clip window.");
    lastState.calcLines.push_back(">> Line fully accepted: (" +
        cf2(x1) + "," + cf2(y1) + ") -> (" + cf2(x2) + "," + cf2(y2) + ")");
    lastState.currentPixelInfo = ">> ACCEPTED  (green pixels shown)";
    lastState.hasCalculation = true;
    lastState.totalSteps = currentStep;
    return;
  }

  // Both outside same region?
  if (outcode1 & outcode2) {
    rejected = true; finished = true;
    clippedPixels.clear();
    buildDisplay();
    lastState.calcLines.push_back("--- Trivial Reject ---");
    lastState.calcLines.push_back("  OC1=" + ci(outcode1) + "  OC2=" + ci(outcode2));
    lastState.calcLines.push_back("  OC1 & OC2 = " + ci(outcode1 & outcode2) + "  ≠ 0");
    lastState.calcLines.push_back("  Both points on the same side — line invisible.");
    lastState.currentPixelInfo = ">> REJECTED  (no pixels to show)";
    lastState.hasCalculation = true;
    lastState.totalSteps = currentStep;
    return;
  }

  // Pick the endpoint outside the clip window
  int outcodeOut = (outcode1 != INSIDE) ? outcode1 : outcode2;
  float xi = x1, yi = y1; // will be overwritten

  lastState.calcLines.push_back("--- Iteration " + ci(currentStep) + " ---");
  lastState.calcLines.push_back("  OC1=" + ci(outcode1) + "  OC2=" + ci(outcode2));

  std::string side;
  if (outcodeOut & TOP) {
    side = "TOP";
    xi = x1 + (x2 - x1) * (yMax - y1) / (y2 - y1);
    yi = yMax;
    lastState.calcLines.push_back("  Clipping to TOP  (y = " + cf2(yMax) + ")");
    lastState.calcLines.push_back("  x = x1 + (x2-x1)*(yMax-y1)/(y2-y1)");
    lastState.calcLines.push_back("  x = " + cf2(x1) + " + (" + cf2(x2) + "-" +
        cf2(x1) + ")*(" + cf2(yMax) + "-" + cf2(y1) + ")/(" + cf2(y2) + "-" + cf2(y1) + ")");
    lastState.calcLines.push_back("  x = " + cf2(xi) + ",  y = " + cf2(yi));
  } else if (outcodeOut & BOTTOM) {
    side = "BOTTOM";
    xi = x1 + (x2 - x1) * (yMin - y1) / (y2 - y1);
    yi = yMin;
    lastState.calcLines.push_back("  Clipping to BOTTOM  (y = " + cf2(yMin) + ")");
    lastState.calcLines.push_back("  x = x1 + (x2-x1)*(yMin-y1)/(y2-y1)");
    lastState.calcLines.push_back("  x = " + cf2(x1) + " + (" + cf2(x2) + "-" +
        cf2(x1) + ")*(" + cf2(yMin) + "-" + cf2(y1) + ")/(" + cf2(y2) + "-" + cf2(y1) + ")");
    lastState.calcLines.push_back("  x = " + cf2(xi) + ",  y = " + cf2(yi));
  } else if (outcodeOut & RIGHT) {
    side = "RIGHT";
    yi = y1 + (y2 - y1) * (xMax - x1) / (x2 - x1);
    xi = xMax;
    lastState.calcLines.push_back("  Clipping to RIGHT  (x = " + cf2(xMax) + ")");
    lastState.calcLines.push_back("  y = y1 + (y2-y1)*(xMax-x1)/(x2-x1)");
    lastState.calcLines.push_back("  y = " + cf2(y1) + " + (" + cf2(y2) + "-" +
        cf2(y1) + ")*(" + cf2(xMax) + "-" + cf2(x1) + ")/(" + cf2(x2) + "-" + cf2(x1) + ")");
    lastState.calcLines.push_back("  x = " + cf2(xi) + ",  y = " + cf2(yi));
  } else { // LEFT
    side = "LEFT";
    yi = y1 + (y2 - y1) * (xMin - x1) / (x2 - x1);
    xi = xMin;
    lastState.calcLines.push_back("  Clipping to LEFT  (x = " + cf2(xMin) + ")");
    lastState.calcLines.push_back("  y = y1 + (y2-y1)*(xMin-x1)/(x2-x1)");
    lastState.calcLines.push_back("  y = " + cf2(y1) + " + (" + cf2(y2) + "-" +
        cf2(y1) + ")*(" + cf2(xMin) + "-" + cf2(x1) + ")/(" + cf2(x2) + "-" + cf2(x1) + ")");
    lastState.calcLines.push_back("  x = " + cf2(xi) + ",  y = " + cf2(yi));
  }

  // Replace the outside endpoint
  if (outcodeOut == outcode1) {
    x1 = xi; y1 = yi;
    outcode1 = computeOutcode(x1, y1);
    lastState.calcLines.push_back("  >> P1 clipped to " + side + ": (" +
        cf2(x1) + ", " + cf2(y1) + ")");
    lastState.calcLines.push_back("  >> New OC1 = " + ci(outcode1));
  } else {
    x2 = xi; y2 = yi;
    outcode2 = computeOutcode(x2, y2);
    lastState.calcLines.push_back("  >> P2 clipped to " + side + ": (" +
        cf2(x2) + ", " + cf2(y2) + ")");
    lastState.calcLines.push_back("  >> New OC2 = " + ci(outcode2));
  }

  lastState.currentPixelInfo =
      ">> Working line: (" + cf2(x1) + "," + cf2(y1) + ") -> (" + cf2(x2) + "," + cf2(y2) + ")";
  lastState.hasCalculation = true;
}

// ----------------------------------------------------------------
// Public interface
// ----------------------------------------------------------------
void CohenSutherland::step() {
  if (isFinished()) return;
  currentStep++;
  lastState.currentStep = currentStep;
  doClipStep();
}

void CohenSutherland::stepK(int k) {
  for (int i = 0; i < k && !isFinished(); ++i) {
    currentStep++;
    lastState.currentStep = currentStep;
    doClipStep();
  }
  lastState.hasCalculation = false;
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";
}

void CohenSutherland::runToCompletion() {
  while (!isFinished()) {
    currentStep++;
    lastState.currentStep = currentStep;
    doClipStep();
  }
  lastState.hasCalculation = false;
  lastState.calcLines.clear();
  lastState.currentPixelInfo = "";
}

void CohenSutherland::reset() { init((int)ox1, (int)oy1, (int)ox2, (int)oy2); }

bool CohenSutherland::isFinished() const { return finished; }

std::vector<Pixel> CohenSutherland::getHighlightedPixels() const {
  return displayPixels;
}

AlgoState CohenSutherland::getCurrentState() const { return lastState; }

std::vector<std::string> CohenSutherland::getInitInfo() const {
  auto ocStr = [](int oc) {
    std::string s = "[";
    if (oc == 0) { s += "INSIDE"; }
    else {
      if (oc & 8) s += "TOP ";
      if (oc & 4) s += "BOTTOM ";
      if (oc & 2) s += "RIGHT ";
      if (oc & 1) s += "LEFT ";
    }
    s += "]";
    return s;
  };
  int oc1 = computeOutcode(ox1, oy1);
  int oc2 = computeOutcode(ox2, oy2);
  return {
    "Line:  P1=(" + cf2(ox1) + ", " + cf2(oy1) + ")  P2=(" + cf2(ox2) + ", " + cf2(oy2) + ")",
    "Clip Window: x=[" + cf2(xMin) + ", " + cf2(xMax) + "]  y=[" + cf2(yMin) + ", " + cf2(yMax) + "]",
    "OC(P1) = " + ci(oc1) + "  " + ocStr(oc1),
    "OC(P2) = " + ci(oc2) + "  " + ocStr(oc2),
    "Trivial Accept? " + std::string((oc1 | oc2) == 0 ? "YES" : "NO"),
    "Trivial Reject? " + std::string((oc1 & oc2) != 0 ? "YES" : "NO"),
  };
}

std::vector<std::string> CohenSutherland::getCurrentVars() const {
  auto ocStr = [](int oc) -> std::string {
    if (oc == 0) return "0 (INSIDE)";
    std::string s = std::to_string(oc) + " ";
    if (oc & 8) s += "T";
    if (oc & 4) s += "B";
    if (oc & 2) s += "R";
    if (oc & 1) s += "L";
    return s;
  };
  return {
    "P1 current : (" + cf2(x1) + ", " + cf2(y1) + ")",
    "P2 current : (" + cf2(x2) + ", " + cf2(y2) + ")",
    "OC1        : " + ocStr(outcode1),
    "OC2        : " + ocStr(outcode2),
    "Step       : " + ci(currentStep),
    "Status     : " + std::string(finished ? (accepted ? "ACCEPTED" : "REJECTED") : "clipping..."),
  };
}

std::string CohenSutherland::getName() const {
  return "Cohen-Sutherland Line Clipping";
}

std::string CohenSutherland::getTheory() const {
  return
    "Cohen-Sutherland Line Clipping\n"
    "================================\n\n"
    "WHAT IS IT?\n"
    "  An efficient algorithm to clip a 2D line segment against\n"
    "  a rectangular window. It quickly accepts or rejects lines\n"
    "  using bit-codes before computing intersection points.\n\n"
    "THE CLIP WINDOW:\n"
    "  A rectangle defined by four edges:\n"
    "    xMin, xMax  (left and right)\n"
    "    yMin, yMax  (bottom and top)\n"
    "  Only the portion of the line inside the rectangle is kept.\n\n"
    "OUTCODE (REGION CODE):\n"
    "  Each endpoint gets a 4-bit outcode based on its position:\n"
    "    Bit 0 (1): LEFT   — x < xMin\n"
    "    Bit 1 (2): RIGHT  — x > xMax\n"
    "    Bit 2 (4): BOTTOM — y < yMin\n"
    "    Bit 3 (8): TOP    — y > yMax\n"
    "  If all bits are 0, the point is INSIDE the window.\n\n"
    "ALGORITHM STEPS:\n"
    "  1. Compute OC1 for P1 and OC2 for P2.\n"
    "  2. Trivial Accept:  if (OC1 | OC2) == 0\n"
    "       Both inside — accept the whole line.\n"
    "  3. Trivial Reject:  if (OC1 & OC2) != 0\n"
    "       Both on the same outside side — discard entirely.\n"
    "  4. Otherwise, clip:\n"
    "       a. Pick an outside endpoint (OC != 0).\n"
    "       b. Find which edge its outcode indicates.\n"
    "       c. Compute the intersection with that edge:\n"
    "          TOP/BOT:  x = x1 + (x2-x1)*(yEdge-y1)/(y2-y1)\n"
    "          LFT/RGT:  y = y1 + (y2-y1)*(xEdge-x1)/(x2-x1)\n"
    "       d. Replace the outside endpoint with intersection.\n"
    "  5. Repeat from step 1 until trivially accepted or rejected.\n\n"
    "WORKED EXAMPLE:\n"
    "  Window: x=[-6,6] y=[-5,5]\n"
    "  Line: P1=(-10, 3)  P2=(10, 8)\n"
    "  OC1 = LEFT(1)               OC2 = RIGHT(2)|TOP(8) = 10\n"
    "  OC1 & OC2 = 0 → not trivially rejected\n"
    "  OC1 | OC2 ≠ 0 → not trivially accepted\n"
    "  Clip P1 to LEFT edge (x=-6):\n"
    "    y = 3 + (8-3)*(-6-(-10))/(10-(-10)) = 3 + 5*4/20 = 4.0\n"
    "    New P1 = (-6, 4)\n"
    "  Clip P2 to TOP edge (y=5):\n"
    "    x = -6 + (10-(-6))*(5-4)/(8-4) = -6 + 16*1/4 = -2.0\n"
    "    New P2 = (-2, 5)  — wait let me just describe the process\n"
    "  Continue until both points are inside.\n\n"
    "ADVANTAGES:\n"
    "+   Very fast — trivial cases handled in O(1) with bitwise ops\n"
    "+   At most 4 iterations needed for a single line\n"
    "+   Simple to implement, works for any convex rectangle\n\n"
    "DISADVANTAGES:\n"
    "-   Limited to rectangular clip windows\n"
    "-   More complex cases need Cyrus-Beck / Liang-Barsky\n\n"
    "COMPLEXITY:\n"
    "  Time : O(1) per line (at most 4 clip iterations)\n"
    "  Space: O(1)\n";
}
