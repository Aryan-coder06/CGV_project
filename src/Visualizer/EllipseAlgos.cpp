#include "../../include/Visualizer/EllipseAlgos.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <string>

// ---------------------------------------------------------------
// Local formatting helpers
// ---------------------------------------------------------------
static std::string ei(int v)   { return std::to_string(v); }
static std::string ef2(float v) {
    std::ostringstream o; o << std::fixed << std::setprecision(2) << v; return o.str();
}

// ================================================================
//  MidpointEllipse
// ================================================================
MidpointEllipse::MidpointEllipse()
    : cx(0), cy(0), a(10), b(6),
      a2_(0), b2_(0), x_(0), y_(0), p1_(0), p2_(0),
      region_(EllipseRegion::DONE), currentStep(0), totalSteps(0) {}

// ----------------------------------------------------------------
//  plotSymmetric — 4-fold symmetry, deduplicated
// ----------------------------------------------------------------
void MidpointEllipse::plotSymmetric(int x, int y, int& delta) {
    int before = (int)path.size();
    path.push_back({ cx + x, cy + y, 1.0f });
    if (x != 0)           path.push_back({ cx - x, cy + y, 1.0f });
    if (y != 0)           path.push_back({ cx + x, cy - y, 1.0f });
    if (x != 0 && y != 0) path.push_back({ cx - x, cy - y, 1.0f });
    delta = (int)path.size() - before;
}

// ----------------------------------------------------------------
//  computeTotalSteps — dry-run through both regions
//  Algorithm structure:  plot(x,y) → update → repeat
// ----------------------------------------------------------------
void MidpointEllipse::computeTotalSteps() {
    if (a <= 0 || b <= 0) { totalSteps = 0; return; }
    float fa2 = (float)a2_, fb2 = (float)b2_;

    int tx = 0, ty = b;
    float tp1 = fb2 - fa2 * b + 0.25f * fa2;
    totalSteps = 0;

    // ---- Region 1: plot, then update ----
    while (2 * b2_ * tx < 2 * a2_ * ty) {
        totalSteps++;
        if (tp1 < 0.0f) {
            tx++;
            tp1 += 2.0f * fb2 * tx + fb2;
        } else {
            tx++;
            ty--;
            tp1 += 2.0f * fb2 * tx + fb2 - 2.0f * fa2 * ty;
        }
    }

    // ---- Region 2: compute p2 at transition, then plot + update ----
    float tp2 = fb2 * ((float)tx + 0.5f) * ((float)tx + 0.5f)
              + fa2 * (float)(ty - 1) * (float)(ty - 1)
              - fa2 * fb2;

    while (ty >= 0) {
        totalSteps++;
        if (tp2 > 0.0f) {
            ty--;
            tp2 += fa2 - 2.0f * fa2 * (float)ty;
        } else {
            ty--;
            tx++;
            tp2 += 2.0f * fb2 * (float)tx - 2.0f * fa2 * (float)ty + fa2;
        }
    }
}

// ----------------------------------------------------------------
//  init()
// ----------------------------------------------------------------
void MidpointEllipse::init(int x1, int y1, int x2, int y2) {
    cx = x1; cy = y1;
    a  = std::abs(x2);
    b  = std::abs(y2);
    a2_ = a * a;
    b2_ = b * b;

    x_ = 0; y_ = b;
    p1_ = (float)b2_ - (float)a2_ * b + 0.25f * (float)a2_;
    region_     = EllipseRegion::REGION1;
    currentStep = 0;
    path.clear();
    computeTotalSteps();

    // Edge: if already at transition, switch to Region 2
    if (a <= 0 || b <= 0) {
        region_ = EllipseRegion::DONE;
    } else if (2 * b2_ * x_ >= 2 * a2_ * y_) {
        p2_ = (float)b2_ * ((float)x_ + 0.5f) * ((float)x_ + 0.5f)
            + (float)a2_ * (float)(y_ - 1) * (float)(y_ - 1)
            - (float)a2_ * (float)b2_;
        region_ = EllipseRegion::REGION2;
    }

    lastState = AlgoState();
    lastState.totalSteps    = totalSteps;
    lastState.pixelsPerStep = 4;
}

// ----------------------------------------------------------------
//  stepRegion1() — full-calc version
// ----------------------------------------------------------------
void MidpointEllipse::stepRegion1() {
    int preX = x_, preY = y_;
    float preP = p1_;

    // Plot current position
    int delta = 0;
    plotSymmetric(x_, y_, delta);

    // Apply decision (use OLD x, y in formulae)
    bool y_dec = (p1_ >= 0.0f);
    if (!y_dec) {
        x_++;
        p1_ += 2.0f * (float)b2_ * (float)x_ + (float)b2_;
    } else {
        x_++; y_--;
        p1_ += 2.0f * (float)b2_ * (float)x_ + (float)b2_
             - 2.0f * (float)a2_ * (float)y_;
    }
    currentStep++;

    // ---- Transition check ----
    bool transitioned = false;
    if (2 * b2_ * x_ >= 2 * a2_ * y_) {
        p2_ = (float)b2_ * ((float)x_ + 0.5f) * ((float)x_ + 0.5f)
            + (float)a2_ * (float)(y_ - 1) * (float)(y_ - 1)
            - (float)a2_ * (float)b2_;
        region_ = EllipseRegion::REGION2;
        transitioned = true;
    }

    // ---- Build calc record ----
    lastState.hasCalculation  = true;
    lastState.currentStep     = currentStep;
    lastState.totalSteps      = totalSteps;
    lastState.pixelsPerStep   = delta;
    lastState.calcLines.clear();

    lastState.calcLines.push_back("--- REGION 1  (x always increments) ---");
    lastState.calcLines.push_back("  Plot: (" + ei(preX) + ", " + ei(preY) + ")");
    lastState.calcLines.push_back("  p1 = " + ef2(preP));
    lastState.calcLines.push_back("");

    // 4-fold symmetry plot
    lastState.calcLines.push_back("--- 4-Fold Symmetry Plotted ---");
    lastState.calcLines.push_back("  (+x,+y) = (" + ei(cx+preX) + "," + ei(cy+preY) + ")  "
                                + "(-x,+y) = (" + ei(cx-preX) + "," + ei(cy+preY) + ")");
    if (preY != 0)
        lastState.calcLines.push_back("  (+x,-y) = (" + ei(cx+preX) + "," + ei(cy-preY) + ")  "
                                    + "(-x,-y) = (" + ei(cx-preX) + "," + ei(cy-preY) + ")");
    lastState.calcLines.push_back("");

    // Decision
    lastState.calcLines.push_back("--- Decision Parameter p1 ---");
    lastState.calcLines.push_back("  Midpoint at (x+1, y-0.5) = (" + ei(preX+1) + ", " + ef2(preY-0.5f) + ")");
    if (!y_dec) {
        lastState.calcLines.push_back("  p1 < 0 ?  YES  (" + ef2(preP) + " < 0)");
        lastState.calcLines.push_back("  Decision: Midpoint INSIDE — Y stays at " + ei(preY));
        lastState.calcLines.push_back("  x_new = " + ei(preX) + " + 1 = " + ei(x_));
        lastState.calcLines.push_back("  p1_new = p1 + 2*b²*x_new + b²");
        lastState.calcLines.push_back("  p1_new = " + ef2(preP)
                                    + " + 2*" + ei(b2_) + "*" + ei(x_)
                                    + " + " + ei(b2_) + " = " + ef2(p1_));
    } else {
        lastState.calcLines.push_back("  p1 >= 0 ?  YES  (" + ef2(preP) + " >= 0)");
        lastState.calcLines.push_back("  Decision: Midpoint OUTSIDE — Y decrements");
        lastState.calcLines.push_back("  x_new = " + ei(preX) + " + 1 = " + ei(x_));
        lastState.calcLines.push_back("  y_new = " + ei(preY) + " - 1 = " + ei(y_));
        lastState.calcLines.push_back("  p1_new = p1 + 2*b²*x_new + b² - 2*a²*y_new");
        lastState.calcLines.push_back("  p1_new = " + ef2(preP)
                                    + " + 2*" + ei(b2_) + "*" + ei(x_)
                                    + " + " + ei(b2_)
                                    + " - 2*" + ei(a2_) + "*" + ei(y_)
                                    + " = " + ef2(p1_));
    }
    lastState.calcLines.push_back("");

    // Transition check
    lastState.calcLines.push_back("--- Transition Check ---");
    lastState.calcLines.push_back("  2*b²*x = 2*" + ei(b2_) + "*" + ei(x_)
                                + " = " + ei(2*b2_*x_));
    lastState.calcLines.push_back("  2*a²*y = 2*" + ei(a2_) + "*" + ei(y_)
                                + " = " + ei(2*a2_*y_));
    if (transitioned) {
        lastState.calcLines.push_back("  2b²x >= 2a²y → YES — entering REGION 2 !");
        lastState.calcLines.push_back("  p2 init = b²(x+0.5)² + a²(y-1)² - a²b²");
        lastState.calcLines.push_back("  p2 = " + ef2(p2_));
    } else {
        lastState.calcLines.push_back("  2b²x < 2a²y → Still in REGION 1");
    }

    lastState.currentPixelInfo = ">> Plotted (" + ei(preX) + "," + ei(preY)
                               + ")  next: x=" + ei(x_) + " y=" + ei(y_);
}

// ----------------------------------------------------------------
//  stepRegion2() — full-calc version
// ----------------------------------------------------------------
void MidpointEllipse::stepRegion2() {
    int preX = x_, preY = y_;
    float preP = p2_;

    // Plot current position
    int delta = 0;
    plotSymmetric(x_, y_, delta);

    // Apply decision (y always decrements; x may increment)
    bool x_inc = (p2_ <= 0.0f);
    if (!x_inc) {
        // x stays
        y_--;
        p2_ += (float)a2_ - 2.0f * (float)a2_ * (float)y_;
    } else {
        y_--; x_++;
        p2_ += 2.0f * (float)b2_ * (float)x_
             - 2.0f * (float)a2_ * (float)y_ + (float)a2_;
    }
    currentStep++;
    if (y_ < 0) region_ = EllipseRegion::DONE;

    // ---- Build calc record ----
    lastState.hasCalculation  = true;
    lastState.currentStep     = currentStep;
    lastState.totalSteps      = totalSteps;
    lastState.pixelsPerStep   = delta;
    lastState.calcLines.clear();

    lastState.calcLines.push_back("--- REGION 2  (y always decrements) ---");
    lastState.calcLines.push_back("  Plot: (" + ei(preX) + ", " + ei(preY) + ")");
    lastState.calcLines.push_back("  p2 = " + ef2(preP));
    lastState.calcLines.push_back("");

    // 4-fold symmetry plot
    lastState.calcLines.push_back("--- 4-Fold Symmetry Plotted ---");
    lastState.calcLines.push_back("  (+x,+y) = (" + ei(cx+preX) + "," + ei(cy+preY) + ")  "
                                + "(-x,+y) = (" + ei(cx-preX) + "," + ei(cy+preY) + ")");
    if (preY != 0)
        lastState.calcLines.push_back("  (+x,-y) = (" + ei(cx+preX) + "," + ei(cy-preY) + ")  "
                                    + "(-x,-y) = (" + ei(cx-preX) + "," + ei(cy-preY) + ")");
    lastState.calcLines.push_back("");

    // Decision
    lastState.calcLines.push_back("--- Decision Parameter p2 ---");
    lastState.calcLines.push_back("  Midpoint at (x+0.5, y-1) = (" + ef2(preX+0.5f) + ", " + ei(preY-1) + ")");
    if (!x_inc) {
        lastState.calcLines.push_back("  p2 > 0 ?  YES  (" + ef2(preP) + " > 0)");
        lastState.calcLines.push_back("  Decision: Midpoint OUTSIDE — X stays at " + ei(preX));
        lastState.calcLines.push_back("  y_new = " + ei(preY) + " - 1 = " + ei(y_));
        lastState.calcLines.push_back("  p2_new = p2 + a² - 2*a²*y_new");
        lastState.calcLines.push_back("  p2_new = " + ef2(preP)
                                    + " + " + ei(a2_)
                                    + " - 2*" + ei(a2_) + "*" + ei(y_)
                                    + " = " + ef2(p2_));
    } else {
        lastState.calcLines.push_back("  p2 <= 0 ?  YES  (" + ef2(preP) + " <= 0)");
        lastState.calcLines.push_back("  Decision: Midpoint INSIDE — X increments");
        lastState.calcLines.push_back("  y_new = " + ei(preY) + " - 1 = " + ei(y_));
        lastState.calcLines.push_back("  x_new = " + ei(preX) + " + 1 = " + ei(x_));
        lastState.calcLines.push_back("  p2_new = p2 + 2*b²*x_new - 2*a²*y_new + a²");
        lastState.calcLines.push_back("  p2_new = " + ef2(preP)
                                    + " + 2*" + ei(b2_) + "*" + ei(x_)
                                    + " - 2*" + ei(a2_) + "*" + ei(y_)
                                    + " + " + ei(a2_)
                                    + " = " + ef2(p2_));
    }

    if (region_ == EllipseRegion::DONE) {
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("  y < 0 → Algorithm Complete!");
    } else {
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("  Next: x=" + ei(x_) + "  y=" + ei(y_));
    }

    lastState.currentPixelInfo = ">> Plotted (" + ei(preX) + "," + ei(preY)
                               + ")  next: x=" + ei(x_) + " y=" + ei(y_);
}

// ----------------------------------------------------------------
//  step() — dispatch to region-specific version
// ----------------------------------------------------------------
void MidpointEllipse::step() {
    if (isFinished()) return;
    if (region_ == EllipseRegion::REGION1) stepRegion1();
    else if (region_ == EllipseRegion::REGION2) stepRegion2();
}

// ----------------------------------------------------------------
//  Silent step helpers
// ----------------------------------------------------------------
static void ellipseSilentStep1(int& x_, int& y_, float& p1_,
                                float a2f, float b2f, int a2, int b2) {
    if (p1_ < 0.0f) {
        x_++;
        p1_ += 2.0f * b2f * x_ + b2f;
    } else {
        x_++; y_--;
        p1_ += 2.0f * b2f * x_ + b2f - 2.0f * a2f * y_;
    }
    (void)a2; (void)b2;  // suppress warnings
}

static void ellipseSilentStep2(int& x_, int& y_, float& p2_,
                                float a2f, float b2f) {
    if (p2_ > 0.0f) {
        y_--;
        p2_ += a2f - 2.0f * a2f * (float)y_;
    } else {
        y_--; x_++;
        p2_ += 2.0f * b2f * (float)x_ - 2.0f * a2f * (float)y_ + a2f;
    }
}

void MidpointEllipse::stepK(int k) {
    float a2f = (float)a2_, b2f = (float)b2_;
    for (int i = 0; i < k && !isFinished(); ++i) {
        int dummy = 0;
        if (region_ == EllipseRegion::REGION1) {
            plotSymmetric(x_, y_, dummy);
            ellipseSilentStep1(x_, y_, p1_, a2f, b2f, a2_, b2_);
            currentStep++;
            if (2 * b2_ * x_ >= 2 * a2_ * y_) {
                p2_ = b2f * (x_+0.5f)*(x_+0.5f) + a2f*(float)(y_-1)*(float)(y_-1) - a2f*b2f;
                region_ = EllipseRegion::REGION2;
            }
        } else if (region_ == EllipseRegion::REGION2) {
            plotSymmetric(x_, y_, dummy);
            ellipseSilentStep2(x_, y_, p2_, a2f, b2f);
            currentStep++;
            if (y_ < 0) region_ = EllipseRegion::DONE;
        }
    }
    lastState.hasCalculation   = false;
    lastState.currentStep      = currentStep;
    lastState.totalSteps       = totalSteps;
    lastState.calcLines.clear();
    lastState.currentPixelInfo = "";
}

void MidpointEllipse::runToCompletion() {
    float a2f = (float)a2_, b2f = (float)b2_;
    while (!isFinished()) {
        int dummy = 0;
        if (region_ == EllipseRegion::REGION1) {
            plotSymmetric(x_, y_, dummy);
            ellipseSilentStep1(x_, y_, p1_, a2f, b2f, a2_, b2_);
            currentStep++;
            if (2 * b2_ * x_ >= 2 * a2_ * y_) {
                p2_ = b2f*(x_+0.5f)*(x_+0.5f) + a2f*(float)(y_-1)*(float)(y_-1) - a2f*b2f;
                region_ = EllipseRegion::REGION2;
            }
        } else if (region_ == EllipseRegion::REGION2) {
            plotSymmetric(x_, y_, dummy);
            ellipseSilentStep2(x_, y_, p2_, a2f, b2f);
            currentStep++;
            if (y_ < 0) region_ = EllipseRegion::DONE;
        }
    }
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();
}

void MidpointEllipse::reset()                          { init(cx, cy, a, b); }
bool MidpointEllipse::isFinished() const               { return region_ == EllipseRegion::DONE || currentStep >= totalSteps; }
std::vector<Pixel> MidpointEllipse::getHighlightedPixels() const { return path; }
AlgoState MidpointEllipse::getCurrentState() const     { return lastState; }
std::string MidpointEllipse::getName() const           { return "Midpoint Ellipse Algorithm"; }

// ----------------------------------------------------------------
//  getInitInfo()
// ----------------------------------------------------------------
std::vector<std::string> MidpointEllipse::getInitInfo() const {
    float p1init = (float)b2_ - (float)a2_ * b + 0.25f * (float)a2_;
    std::string reg1cond = "2*b²*x < 2*a²*y  i.e.  2*" + ei(b2_) + "*x < 2*" + ei(a2_) + "*y";
    return {
        "Centre:   (" + ei(cx) + ", " + ei(cy) + ")",
        "Semi-a (X-axis): a = " + ei(a),
        "Semi-b (Y-axis): b = " + ei(b),
        "a² = " + ei(a2_) + ",   b² = " + ei(b2_),
        "Init:  x=0, y=b=" + ei(b),
        "Init p1 = b² - a²*b + 0.25*a²",
        "     p1 = " + ei(b2_) + " - " + ei(a2_) + "*" + ei(b) + " + 0.25*" + ei(a2_) + " = " + ef2(p1init),
        "Region 1 condition: " + reg1cond,
        "Total steps = " + ei(totalSteps),
        "Pixels per step = 4 (4-fold symmetry)",
    };
}

// ----------------------------------------------------------------
//  getCurrentVars()
// ----------------------------------------------------------------
std::vector<std::string> MidpointEllipse::getCurrentVars() const {
    std::string regStr =
        (region_ == EllipseRegion::REGION1) ? "1  (x increments)" :
        (region_ == EllipseRegion::REGION2) ? "2  (y decrements)" : "Done";
    std::vector<std::string> v;
    v.push_back("Region     : " + regStr);
    v.push_back("Current x  : " + ei(x_));
    v.push_back("Current y  : " + ei(y_));
    v.push_back("a=" + ei(a) + "  b=" + ei(b) + "  a²=" + ei(a2_) + "  b²=" + ei(b2_));
    if (region_ == EllipseRegion::REGION1) {
        v.push_back("p1         : " + ef2(p1_));
        v.push_back("2b²x=" + ei(2*b2_*x_) + "  2a²y=" + ei(2*a2_*y_));
    } else if (region_ == EllipseRegion::REGION2) {
        v.push_back("p2         : " + ef2(p2_));
    }
    v.push_back("Plotted    : " + ei((int)path.size()) + " pixels");
    return v;
}

// ----------------------------------------------------------------
//  getTheory()
// ----------------------------------------------------------------
std::string MidpointEllipse::getTheory() const {
    return
        "Midpoint Ellipse Algorithm\n"
        "==========================\n\n"
        "WHAT IS IT?\n"
        "  Integer (and near-integer) algorithm for rasterizing\n"
        "  an axis-aligned ellipse with semi-axes a (X) and b (Y).\n"
        "  Uses the same midpoint evaluation idea as the circle\n"
        "  algorithm but requires TWO separate regions because\n"
        "  the ellipse slope changes from steep to shallow.\n\n"
        "THE ELLIPSE EQUATION:\n"
        "  F(x,y) = b²x² + a²y² - a²b²\n"
        "  F < 0: inside ellipse\n"
        "  F = 0: on ellipse\n"
        "  F > 0: outside ellipse\n\n"
        "4-FOLD SYMMETRY:\n"
        "  An ellipse (a≠b) is symmetric about BOTH coordinate\n"
        "  axes but NOT about the diagonals (unlike a circle).\n"
        "  So we reflect in 4 ways only:\n"
        "    (+x,+y)  (-x,+y)  (+x,-y)  (-x,-y)\n\n"
        "TWO REGIONS:\n"
        "  The ellipse tangent has slope = -b²x / a²y.\n"
        "  When |slope| > 1: steep — scan in X (Region 1)\n"
        "  When |slope| < 1: shallow — scan in Y (Region 2)\n"
        "  Transition: |slope| = 1  ⟹  b²x = a²y\n"
        "              equivalently: 2b²x = 2a²y\n\n"
        "REGION 1  (starting at (0, b)):\n"
        "  x always increments by 1.\n"
        "  Decision: midpoint at (x+1, y-0.5).\n"
        "  p1 = F(1, b-0.5) = b² - a²b + 0.25a²\n\n"
        "  While 2b²x < 2a²y:\n"
        "    Plot 4 symmetric pixels at (x, y)\n"
        "    if p1 < 0:\n"
        "      Midpoint inside → y stays\n"
        "      x++\n"
        "      p1 += 2b²*x_new + b²\n"
        "    else:\n"
        "      Midpoint outside → y--\n"
        "      x++, y--\n"
        "      p1 += 2b²*x_new + b² - 2a²*y_new\n\n"
        "REGION 2  (starting after transition):\n"
        "  y always decrements by 1.\n"
        "  Decision: midpoint at (x+0.5, y-1).\n"
        "  p2 = F(x+0.5, y-1) at transition point\n\n"
        "  While y >= 0:\n"
        "    Plot 4 symmetric pixels at (x, y)\n"
        "    if p2 > 0:\n"
        "      Midpoint outside → x stays\n"
        "      y--\n"
        "      p2 += a² - 2a²*y_new\n"
        "    else:\n"
        "      Midpoint inside → x++\n"
        "      y--, x++\n"
        "      p2 += 2b²*x_new - 2a²*y_new + a²\n\n"
        "UPDATE FORMULA DERIVATION (Region 1, y stays):\n"
        "  Old midpoint: F(x+1, y-0.5)\n"
        "  New midpoint: F(x+2, y-0.5)  [x incremented by 1]\n"
        "  Δ = b²[(x+2)²-(x+1)²] = b²(2x+3)\n"
        "    = 2b²(x+1) + b²  = 2b²*x_new + b²  ✓\n\n"
        "UPDATE FORMULA DERIVATION (Region 1, y decrements):\n"
        "  New midpoint: F(x+2, y-1.5)\n"
        "  Δ = b²(2x+3) + a²[(y-1.5)²-(y-0.5)²]\n"
        "    = 2b²*x_new + b² - 2a²*(y-1)\n"
        "    = 2b²*x_new + b² - 2a²*y_new  ✓\n\n"
        "WORKED EXAMPLE  a=4, b=3, centre=(0,0):\n"
        "  p1 = 9 - 16*3 + 0.25*16 = 9-48+4 = -35\n"
        "  Init: x=0, y=3.  Transition: 2*9*x < 2*16*y = 32\n"
        "  Step1: plot(0,3). p1=-35<0. x=1. p1+=-35+2*9*1+9=-35+27=-8.\n"
        "  Step2: plot(1,3). p1=-8<0.  x=2. p1+=-8+2*9*2+9=-8+45=37.\n"
        "  Step3: 2*9*2=36>=2*16*3=96? No. plot(2,3).\n"
        "         p1=37>=0. y=2. x=3. p1+=2*9*3+9-2*16*2=54+9-64=-1.\n"
        "  Check: 2*9*3=54<2*16*2=64. Still R1.\n"
        "  Step4: plot(3,2). p1=-1<0. x=4. p1+=-1+2*9*4+9=-1+81=80.\n"
        "  Check: 2*9*4=72>=2*16*2=64. → Region 2!\n"
        "  p2 = 9*(4.5)² + 16*(2-1)² - 144 = 182.25+16-144=54.25\n"
        "  ... region 2 continues until y=-1.\n\n"
        "ADVANTAGES:\n"
        "+   Integer arithmetic (except float for p1,p2 init)\n"
        "+   Only ~a+b iterations total\n"
        "+   Generalises to any a,b (circle is a=b)\n\n"
        "DISADVANTAGES:\n"
        "-   Two-region logic is more complex than circle\n"
        "-   No anti-aliasing\n\n"
        "TIME COMPLEXITY:  O(a+b)\n"
        "SPACE COMPLEXITY: O(a+b) — ellipse perimeter pixels\n";
}
