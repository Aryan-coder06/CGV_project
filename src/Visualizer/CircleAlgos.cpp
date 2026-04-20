#include "../../include/Visualizer/CircleAlgos.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <string>

// ---------------------------------------------------------------
// Local formatting helpers (translation-unit only)
// ---------------------------------------------------------------
static std::string ci(int v) { return std::to_string(v); }
static std::string cf4(float v) {
    std::ostringstream o; o << std::fixed << std::setprecision(4) << v; return o.str();
}

// ================================================================
//  Constructor
// ================================================================
MidpointCircle::MidpointCircle()
    : cx(0), cy(0), r(8), x_(0), y_(0), p_(0),
      currentStep(0), totalSteps(0) {}

// ================================================================
//  plotOctants()
//  Pushes all 8 symmetric pixels into path and returns a one-line
//  summary like "(3,8) (-3,8) (3,-8) (-3,-8) (8,3) (-8,3) ..."
// ================================================================
std::string MidpointCircle::plotOctants(int x, int y) {
    // 8 symmetric points relative to centre
    struct P { int dx, dy; };
    P pts[8] = {
        { x,  y}, {-x,  y}, { x, -y}, {-x, -y},
        { y,  x}, {-y,  x}, { y, -x}, {-y, -x}
    };

    // Remove exact duplicates (happens at x==0, y==0, or x==y)
    std::vector<P> unique;
    for (auto& p : pts) {
        bool dup = false;
        for (auto& u : unique) if (u.dx==p.dx && u.dy==p.dy) { dup=true; break; }
        if (!dup) unique.push_back(p);
    }

    std::string out = "";
    for (auto& p : unique) {
        path.push_back({ cx + p.dx, cy + p.dy, 1.0f });
        out += "("+ci(cx+p.dx)+","+ci(cy+p.dy)+") ";
    }
    return out;
}

// ================================================================
//  computeTotalSteps() — dry run to count loop iterations
// ================================================================
void MidpointCircle::computeTotalSteps() {
    if (r <= 0) { totalSteps = 0; return; }
    int tx = 0, ty = r, tp = 1 - r;
    totalSteps = 0;
    while (tx < ty) {
        tx++;
        if (tp < 0) { tp += 2*tx + 1; }
        else        { ty--; tp += 2*tx - 2*ty + 1; }
        totalSteps++;
    }
}

// ================================================================
//  init()
// ================================================================
void MidpointCircle::init(int x1, int y1, int x2, int /*y2*/) {
    cx = x1; cy = y1; r = std::abs(x2);
    x_ = 0;  y_ = r;  p_ = 1 - r;
    currentStep = 0;
    path.clear();
    computeTotalSteps();

    // Plot the initial 8 points (step 0 — startup, no calc shown)
    plotOctants(x_, y_);

    lastState = AlgoState();
    lastState.totalSteps    = totalSteps;
    lastState.pixelsPerStep = 8;  // approximate; dedup handled in plotOctants
}

// ================================================================
//  step() — one iteration of the while loop, full calculation
// ================================================================
void MidpointCircle::step() {
    if (isFinished()) return;

    // Capture before-state
    int old_x = x_, old_y = y_, old_p = p_;

    // Advance x (always)
    x_++;

    // Decision
    bool y_decremented = false;
    if (p_ < 0) {
        p_ += 2*x_ + 1;
    } else {
        y_--;
        p_ += 2*x_ - 2*y_ + 1;
        y_decremented = true;
    }

    // Plot 8 octants and get a summary string
    std::string octStr = plotOctants(x_, y_);

    currentStep++;

    // ---- Build calculation record ----
    lastState.hasCalculation  = true;
    lastState.currentStep     = currentStep;
    lastState.totalSteps      = totalSteps;
    lastState.pixelsPerStep   = 8;
    lastState.calcLines.clear();

    lastState.calcLines.push_back("--- Before this step ---");
    lastState.calcLines.push_back("  x = " + ci(old_x) + "  y = " + ci(old_y) + "  p = " + ci(old_p));
    lastState.calcLines.push_back("");

    lastState.calcLines.push_back("--- X increments (always) ---");
    lastState.calcLines.push_back("  x = " + ci(old_x) + " + 1 = " + ci(x_));
    lastState.calcLines.push_back("");

    lastState.calcLines.push_back("--- Decision Parameter ---");
    lastState.calcLines.push_back("  p (before) = " + ci(old_p));
    if (!y_decremented) {
        lastState.calcLines.push_back("  p < 0 ?  YES  (" + ci(old_p) + " < 0)");
        lastState.calcLines.push_back("  Decision: UPPER pixel (Y stays at " + ci(y_) + ")");
        lastState.calcLines.push_back("  p = p + 2*x + 1");
        lastState.calcLines.push_back("  p = " + ci(old_p) + " + 2*" + ci(x_) + " + 1 = " + ci(p_));
    } else {
        lastState.calcLines.push_back("  p >= 0 ?  YES  (" + ci(old_p) + " >= 0)");
        lastState.calcLines.push_back("  Decision: LOWER pixel (Y decrements)");
        lastState.calcLines.push_back("  y = " + ci(old_y) + " - 1 = " + ci(y_));
        lastState.calcLines.push_back("  p = p + 2*x - 2*y + 1");
        lastState.calcLines.push_back("  p = " + ci(old_p) + " + 2*" + ci(x_)
                                    + " - 2*" + ci(y_) + " + 1 = " + ci(p_));
    }
    lastState.calcLines.push_back("");

    lastState.calcLines.push_back("--- 8-Fold Symmetry  (x=" + ci(x_) + ", y=" + ci(y_) + ") ---");
    lastState.calcLines.push_back("  (+x,+y)=(" + ci(cx+x_) + "," + ci(cy+y_) + ")  "
                                + "(-x,+y)=(" + ci(cx-x_) + "," + ci(cy+y_) + ")");
    lastState.calcLines.push_back("  (+x,-y)=(" + ci(cx+x_) + "," + ci(cy-y_) + ")  "
                                + "(-x,-y)=(" + ci(cx-x_) + "," + ci(cy-y_) + ")");
    lastState.calcLines.push_back("  (+y,+x)=(" + ci(cx+y_) + "," + ci(cy+x_) + ")  "
                                + "(-y,+x)=(" + ci(cx-y_) + "," + ci(cy+x_) + ")");
    lastState.calcLines.push_back("  (+y,-x)=(" + ci(cx+y_) + "," + ci(cy-x_) + ")  "
                                + "(-y,-x)=(" + ci(cx-y_) + "," + ci(cy-x_) + ")");

    lastState.currentPixelInfo = ">> New position: (" + ci(x_) + "," + ci(y_) + ")  p=" + ci(p_);
}

// ================================================================
//  Silent helpers
// ================================================================
static void silentStep(int& x_, int& y_, int& p_) {
    x_++;
    if (p_ < 0) { p_ += 2*x_ + 1; }
    else        { y_--; p_ += 2*x_ - 2*y_ + 1; }
}

void MidpointCircle::stepK(int k) {
    for (int i = 0; i < k && !isFinished(); ++i) {
        silentStep(x_, y_, p_);
        plotOctants(x_, y_);
        currentStep++;
    }
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();
    lastState.currentPixelInfo = "";
}

void MidpointCircle::runToCompletion() {
    while (!isFinished()) {
        silentStep(x_, y_, p_);
        plotOctants(x_, y_);
        currentStep++;
    }
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();
}

void MidpointCircle::reset()                        { init(cx, cy, r, 0); }
bool MidpointCircle::isFinished() const             { return currentStep >= totalSteps; }
std::vector<Pixel> MidpointCircle::getHighlightedPixels() const { return path; }
AlgoState MidpointCircle::getCurrentState() const   { return lastState; }
std::string MidpointCircle::getName() const         { return "Midpoint Circle Algorithm"; }

// ================================================================
//  getInitInfo()
// ================================================================
std::vector<std::string> MidpointCircle::getInitInfo() const {
    int p0 = 1 - r;
    return {
        "Centre:  (" + ci(cx) + ", " + ci(cy) + ")",
        "Radius:  r = " + ci(r),
        "Initial: x = 0,  y = r = " + ci(r),
        "Initial decision param: p0 = 1 - r = 1 - " + ci(r) + " = " + ci(p0),
        "Loop: x steps from 0 while x < y",
        "Total octant steps = " + ci(totalSteps),
        "Pixels per step = 8 (via 8-fold symmetry)",
    };
}

// ================================================================
//  getCurrentVars()
// ================================================================
std::vector<std::string> MidpointCircle::getCurrentVars() const {
    // ideal circle eq at midpoint candidate
    float midpointF = (float)(x_+1)*(x_+1) + ((float)y_ - 0.5f)*((float)y_ - 0.5f) - (float)r*r;
    return {
        "Current x  : " + ci(x_),
        "Current y  : " + ci(y_),
        "Decision p : " + ci(p_),
        "r          : " + ci(r),
        "x < y ?    : " + std::string(x_ < y_ ? "YES (still looping)" : "NO  (done)"),
        "F(x+1,y-0.5)= " + cf4(midpointF) + (midpointF<0?" < 0 (inside)":" >= 0 (outside)"),
        "Plotted    : " + ci((int)path.size()) + " pixels",
    };
}

// ================================================================
//  getTheory()
// ================================================================
std::string MidpointCircle::getTheory() const {
    return
        "Midpoint Circle Algorithm\n"
        "=========================\n\n"
        "WHAT IS IT?\n"
        "  An integer-only algorithm that rasterizes a circle\n"
        "  with centre (cx,cy) and radius r.  No trigonometry,\n"
        "  no square roots, no floating-point — only additions.\n"
        "  Due to Jack Bresenham (1977), later simplified by\n"
        "  Pitteway and van Aken.\n\n"
        "8-FOLD SYMMETRY:\n"
        "  A circle is symmetric about both axes and both diagonals.\n"
        "  Only the first octant (0° to 45°, where x: 0→r/√2) is\n"
        "  computed. The other 7 octants are FREE reflections:\n"
        "  For (x, y): plot (+x,+y) (-x,+y) (+x,-y) (-x,-y)\n"
        "                   (+y,+x) (-y,+x) (+y,-x) (-y,-x)\n"
        "  → 8 pixels per step, all shifted by (cx, cy).\n\n"
        "ALGORITHM STEPS:\n"
        "  1. Initialise:  x = 0,  y = r\n"
        "                  p = 1 - r\n"
        "  2. Plot 8 symmetric points for (x, y)\n"
        "  3. While x < y:\n"
        "       x = x + 1             (always step right)\n"
        "       If p < 0:\n"
        "         p = p + 2*x + 1     (midpoint inside → y stays)\n"
        "       Else:\n"
        "         y = y - 1\n"
        "         p = p + 2*x - 2*y + 1  (midpoint outside → y--)\n"
        "       Plot 8 symmetric points for (x, y)\n\n"
        "DECISION PARAMETER p:\n"
        "  Derived from the implicit circle equation:\n"
        "    F(x,y) = x² + y² - r²\n"
        "    F < 0  → inside circle\n"
        "    F = 0  → on circle\n"
        "    F > 0  → outside circle\n\n"
        "  At each step, the midpoint between the two candidate\n"
        "  pixels is (x+1, y-0.5).  Evaluating F at this midpoint\n"
        "  (scaled by 4 to stay integer) gives the decision param.\n\n"
        "  p < 0:  midpoint is INSIDE  → choose upper pixel (y same)\n"
        "  p >= 0: midpoint is OUTSIDE → choose lower pixel (y--)\n\n"
        "UPDATE FORMULAS:\n"
        "  p < 0  → p_new = p + 2*x_new + 1\n"
        "  p >= 0 → p_new = p + 2*x_new - 2*y_new + 1\n"
        "  (x_new and y_new are AFTER incrementing/decrementing)\n\n"
        "WORKED EXAMPLE  centre=(0,0),  r=4:\n"
        "  p0 = 1-4 = -3\n"
        "  Init:  x=0, y=4.  Plot 4 pts: (0,4)(0,-4)(4,0)(-4,0)\n"
        "  Step1: x=1, p=-3<0 → p=0,  y=4.  Plot 8pts at (1,4)\n"
        "  Step2: x=2, p=0>=0 → y=3,  p=-1. Plot 8pts at (2,3)\n"
        "  Step3: x=3, p=-1<0 → p=6,  y=3.  Plot 4pts at (3,3)\n"
        "    x=3, y=3.  x<y? NO → stop.\n\n"
        "WHY NO SQRT AND NO TRIG?\n"
        "  The p update uses only the DIFFERENCE in F between\n"
        "  consecutive steps — computed entirely by addition.\n\n"
        "ADVANTAGES:\n"
        "+   Integer only — extremely fast\n"
        "+   8-fold symmetry: only ~r/√2 steps needed\n"
        "+   Pixel-perfect on any radius\n\n"
        "DISADVANTAGES:\n"
        "-   Only circles — not ellipses (needs separate algo)\n"
        "-   No anti-aliasing\n\n"
        "TIME COMPLEXITY:  O(r)\n"
        "SPACE COMPLEXITY: O(r) — ~π·r/2 pixels plotted\n";
}

// ================================================================
//  BresenhamCircle
// ================================================================
BresenhamCircle::BresenhamCircle()
    : cx(0), cy(0), r(8), x_(0), y_(0), d_(0),
      currentStep(0), totalSteps(0) {}

// ----------------------------------------------------------------
//  plotOctants — same dedup logic as MidpointCircle
// ----------------------------------------------------------------
std::string BresenhamCircle::plotOctants(int x, int y) {
    struct P { int dx, dy; };
    P pts[8] = {
        { x,  y}, {-x,  y}, { x, -y}, {-x, -y},
        { y,  x}, {-y,  x}, { y, -x}, {-y, -x}
    };
    std::vector<P> unique;
    for (auto& p : pts) {
        bool dup = false;
        for (auto& u : unique) if (u.dx==p.dx && u.dy==p.dy) { dup=true; break; }
        if (!dup) unique.push_back(p);
    }
    std::string out;
    for (auto& p : unique) {
        path.push_back({ cx+p.dx, cy+p.dy, 1.0f });
        out += "("+ci(cx+p.dx)+","+ci(cy+p.dy)+") ";
    }
    return out;
}

// ----------------------------------------------------------------
//  computeTotalSteps — dry-run of Bresenham circle (while x<=y)
// ----------------------------------------------------------------
void BresenhamCircle::computeTotalSteps() {
    if (r <= 0) { totalSteps = 0; return; }
    int tx = 0, ty = r, td = 3 - 2*r;
    totalSteps = 0;
    while (tx <= ty) {
        // count this iteration (plot at (tx,ty))
        totalSteps++;
        // update (matching step() exactly)
        if (td < 0) { td += 4*tx + 6; }
        else        { td += 4*(tx - ty) + 10; ty--; }
        tx++;
    }
}

// ----------------------------------------------------------------
//  init()
// ----------------------------------------------------------------
void BresenhamCircle::init(int x1, int y1, int x2, int /*y2*/) {
    cx = x1; cy = y1; r = std::abs(x2);
    x_ = 0;  y_ = r;  d_ = 3 - 2*r;
    currentStep = 0;
    path.clear();
    computeTotalSteps();

    lastState = AlgoState();
    lastState.totalSteps    = totalSteps;
    lastState.pixelsPerStep = 8;
}

// ----------------------------------------------------------------
//  step() — plot current (x_,y_), apply decision, advance x_
//  The algorithm structure (Version A):
//    while x <= y:  plot(x,y)  →  update d  →  x++
// ----------------------------------------------------------------
void BresenhamCircle::step() {
    if (isFinished()) return;

    // Capture state BEFORE any update
    int preX = x_, preY = y_, preD = d_;

    // --- Plot current position ---
    plotOctants(x_, y_);

    // --- Apply Bresenham decision (using OLD x, OLD y) ---
    bool y_dec = (preD >= 0);
    if (preD < 0) {
        d_ += 4 * preX + 6;               // East: y stays
    } else {
        d_ += 4 * (preX - preY) + 10;     // Southeast: y decrements
        y_--;
    }
    x_++;
    currentStep++;

    // ---- Build calc record ----
    lastState.hasCalculation  = true;
    lastState.currentStep     = currentStep;
    lastState.totalSteps      = totalSteps;
    lastState.pixelsPerStep   = 8;
    lastState.calcLines.clear();

    lastState.calcLines.push_back("--- Plotting at (x=" + ci(preX) + ", y=" + ci(preY) + ") ---");
    lastState.calcLines.push_back("  d (current) = " + ci(preD));
    lastState.calcLines.push_back("");
    lastState.calcLines.push_back("--- 8-Fold Symmetry Plotted ---");
    lastState.calcLines.push_back("  (+x,+y)=(" + ci(cx+preX) + "," + ci(cy+preY) + ")  "
                                + "(-x,+y)=(" + ci(cx-preX) + "," + ci(cy+preY) + ")");
    lastState.calcLines.push_back("  (+x,-y)=(" + ci(cx+preX) + "," + ci(cy-preY) + ")  "
                                + "(-x,-y)=(" + ci(cx-preX) + "," + ci(cy-preY) + ")");
    lastState.calcLines.push_back("  (+y,+x)=(" + ci(cx+preY) + "," + ci(cy+preX) + ")  "
                                + "(-y,+x)=(" + ci(cx-preY) + "," + ci(cy+preX) + ")");
    lastState.calcLines.push_back("  (+y,-x)=(" + ci(cx+preY) + "," + ci(cy-preX) + ")  "
                                + "(-y,-x)=(" + ci(cx-preY) + "," + ci(cy-preX) + ")");
    lastState.calcLines.push_back("");

    lastState.calcLines.push_back("--- Decision Parameter ---");
    lastState.calcLines.push_back("  d = " + ci(preD));
    if (!y_dec) {
        lastState.calcLines.push_back("  d < 0 ?  YES  (" + ci(preD) + " < 0)");
        lastState.calcLines.push_back("  Decision: EAST — Y stays at " + ci(preY));
        lastState.calcLines.push_back("  d_new = d + 4*x + 6");
        lastState.calcLines.push_back("  d_new = " + ci(preD) + " + 4*" + ci(preX)
                                    + " + 6 = " + ci(d_));
    } else {
        lastState.calcLines.push_back("  d >= 0 ?  YES  (" + ci(preD) + " >= 0)");
        lastState.calcLines.push_back("  Decision: SOUTH-EAST — Y decrements");
        lastState.calcLines.push_back("  y = " + ci(preY) + " - 1 = " + ci(y_));
        lastState.calcLines.push_back("  d_new = d + 4*(x - y_old) + 10");
        lastState.calcLines.push_back("  d_new = " + ci(preD) + " + 4*(" + ci(preX)
                                    + " - " + ci(preY) + ") + 10 = " + ci(d_));
    }
    lastState.calcLines.push_back("");
    lastState.calcLines.push_back("--- X increments (always) ---");
    lastState.calcLines.push_back("  x = " + ci(preX) + " + 1 = " + ci(x_));
    lastState.calcLines.push_back("  Next state: x=" + ci(x_) + "  y=" + ci(y_) + "  d=" + ci(d_));

    lastState.currentPixelInfo = ">> Plotted ("+ci(preX)+","+ci(preY)+")  next: x="
                               + ci(x_)+", y="+ci(y_)+", d="+ci(d_);
}

// ----------------------------------------------------------------
//  Silent helpers
// ----------------------------------------------------------------
static void brescSilentStep(int& x_, int& y_, int& d_) {
    if (d_ < 0) { d_ += 4*x_ + 6; }
    else        { d_ += 4*(x_ - y_) + 10; y_--; }
    x_++;
}

void BresenhamCircle::stepK(int k) {
    for (int i = 0; i < k && !isFinished(); ++i) {
        plotOctants(x_, y_);
        brescSilentStep(x_, y_, d_);
        currentStep++;
    }
    lastState.hasCalculation   = false;
    lastState.currentStep      = currentStep;
    lastState.totalSteps       = totalSteps;
    lastState.calcLines.clear();
    lastState.currentPixelInfo = "";
}

void BresenhamCircle::runToCompletion() {
    while (!isFinished()) {
        plotOctants(x_, y_);
        brescSilentStep(x_, y_, d_);
        currentStep++;
    }
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();
}

void BresenhamCircle::reset()                         { init(cx, cy, r, 0); }
bool BresenhamCircle::isFinished() const              { return currentStep >= totalSteps; }
std::vector<Pixel> BresenhamCircle::getHighlightedPixels() const { return path; }
AlgoState BresenhamCircle::getCurrentState() const    { return lastState; }
std::string BresenhamCircle::getName() const          { return "Bresenham's Circle Algorithm"; }

std::vector<std::string> BresenhamCircle::getInitInfo() const {
    int d0 = 3 - 2*r;
    return {
        "Centre:  (" + ci(cx) + ", " + ci(cy) + ")",
        "Radius:  r = " + ci(r),
        "Initial: x = 0,  y = r = " + ci(r),
        "Initial decision:  d0 = 3 - 2*r = 3 - 2*" + ci(r) + " = " + ci(d0),
        "Loop: while x <= y  (Version A — plot then update)",
        "Total loop iterations = " + ci(totalSteps),
        "Pixels per step = 8 (via 8-fold symmetry)",
    };
}

std::vector<std::string> BresenhamCircle::getCurrentVars() const {
    return {
        "Current x  : " + ci(x_),
        "Current y  : " + ci(y_),
        "Decision d : " + ci(d_) + (d_<0 ? "  (East next)" : "  (SE next)"),
        "r          : " + ci(r),
        "x <= y ?   : " + std::string(x_ <= y_ ? "YES (looping)" : "NO  (done)"),
        "Plotted    : " + ci((int)path.size()) + " pixels",
    };
}

std::string BresenhamCircle::getTheory() const {
    return
        "Bresenham's Circle Algorithm\n"
        "============================\n\n"
        "WHAT IS IT?\n"
        "  An integer-only circle rasterizer closely related to\n"
        "  the Midpoint Circle Algorithm. Both use 8-fold symmetry\n"
        "  and produce IDENTICAL pixel sequences — the difference\n"
        "  is purely in how the decision parameter is derived.\n\n"
        "DECISION PARAMETERS COMPARED:\n"
        "  Midpoint:  p = 1 - r\n"
        "  Bresenham: d = 3 - 2*r\n"
        "  Note: d ≈ 2*p + 1  (factor of ~2 difference)\n\n"
        "ALGORITHM STEPS:\n"
        "  1. Init: x = 0,  y = r,  d = 3 - 2*r\n"
        "  2. While x <= y:\n"
        "       a. Plot 8 symmetric pixels at (x, y)\n"
        "       b. if d < 0:\n"
        "            d = d + 4*x + 6          (East: y stays)\n"
        "          else:\n"
        "            d = d + 4*(x - y) + 10   (SE: y--)\n"
        "            y = y - 1\n"
        "       c. x = x + 1\n\n"
        "UPDATE FORMULA DERIVATION:\n"
        "  Circle function: F(x,y) = x² + y² - r²\n\n"
        "  At each step we choose between:\n"
        "    E  pixel: (x+1, y)   — East move\n"
        "    SE pixel: (x+1, y-1) — South-East move\n\n"
        "  EAST   increment  ΔE  = 2x + 3\n"
        "  SE increment     ΔSE = 2(x-y) + 5\n\n"
        "  Second differences (tell us how ΔE/ΔSE change per step):\n"
        "    d_new (East) = d + 4x + 6    (using OLD x)\n"
        "    d_new (SE)   = d + 4(x-y)+10 (using OLD x and OLD y)\n\n"
        "  The initialisation d = 3 - 2r comes from evaluating\n"
        "  F at the FIRST candidate point: F(1, r) - 3r/2 ≈ 3-2r.\n\n"
        "VS MIDPOINT UPDATE FORMULAS:\n"
        "  Midpoint d<0  (E):  p += 2*x_new + 1\n"
        "  Midpoint d>=0 (SE): p += 2*x_new - 2*y_new + 1\n"
        "  (uses NEW x and y — after incrementing/decrementing)\n\n"
        "  Bresenham d<0  (E):  d += 4*x + 6\n"
        "  Bresenham d>=0 (SE): d += 4*(x-y) + 10\n"
        "  (uses OLD x and y — before incrementing/decrementing)\n\n"
        "  Both produce the SAME pixel output for all r.\n\n"
        "WHY STUDY BOTH?\n"
        "  They illustrate different mathematical approaches to\n"
        "  the same rasterization problem:\n"
        "  - Midpoint thinks in terms of a 'probe point' at (x+1, y-0.5)\n"
        "  - Bresenham thinks in terms of second-order differences\n\n"
        "WORKED EXAMPLE  centre=(0,0),  r=4:\n"
        "  d0 = 3 - 2*4 = -5\n"
        "  Step1: plot(0,4).  d=-5<0 → d=-5+6=1.   x=1, y=4\n"
        "  Step2: plot(1,4).  d=1>=0 → d=1-6+10=5. y=3. x=2, y=3\n"
        "  Step3: plot(2,3).  d=5>=0 → d=5-4+10=11. y=2. 3<=2? NO → stop\n"
        "  → Total 3 octant points (matches Midpoint exactly)\n\n"
        "ADVANTAGES:\n"
        "+   Integer arithmetic only\n"
        "+   8-fold symmetry — only r/√2 steps\n"
        "+   Identical result to Midpoint Circle\n\n"
        "DISADVANTAGES:\n"
        "-   No anti-aliasing\n"
        "-   Circles only (not ellipses)\n\n"
        "TIME COMPLEXITY:  O(r)\n"
        "SPACE COMPLEXITY: O(r)\n";
}
