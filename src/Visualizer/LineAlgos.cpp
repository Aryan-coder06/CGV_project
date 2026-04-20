#include "../../include/Visualizer/LineAlgos.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

// ----------------------------------------------------------------
// Internal helper: format a float to 4 decimal places for display
// ----------------------------------------------------------------
static std::string fmt4(float v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << v;
    return oss.str();
}

static std::string fmt2(float v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << v;
    return oss.str();
}

// ================================================================
//  Constructor
// ================================================================
DDALine::DDALine()
    : x1(0), y1(0), x2(0), y2(0),
      currentX(0.0f), currentY(0.0f),
      xIncrement(0.0f), yIncrement(0.0f),
      totalSteps(0), currentStep(0)
{}

// ================================================================
//  init()  —  Reset and pre-compute all invariant quantities
// ================================================================
void DDALine::init(int startX, int startY, int endX, int endY) {
    x1 = startX;  y1 = startY;
    x2 = endX;    y2 = endY;

    currentX = (float)x1;
    currentY = (float)y1;
    currentStep = 0;
    path.clear();

    calculateIncrements();

    // Plot the very first pixel immediately (step 0)
    path.push_back({ (int)std::round(currentX), (int)std::round(currentY) });

    // Initial state (no calculation yet)
    lastState = AlgoState();
    lastState.currentStep = 0;
    lastState.totalSteps  = totalSteps;
    lastState.hasCalculation = false;
}

// ================================================================
//  calculateIncrements()
// ================================================================
void DDALine::calculateIncrements() {
    int dx = x2 - x1;
    int dy = y2 - y1;

    totalSteps = std::max(std::abs(dx), std::abs(dy));

    if (totalSteps > 0) {
        xIncrement = (float)dx / (float)totalSteps;
        yIncrement = (float)dy / (float)totalSteps;
    } else {
        xIncrement = 0.0f;
        yIncrement = 0.0f;
    }
}

// ================================================================
//  step()  —  Advance exactly ONE step and record full calculation
// ================================================================
void DDALine::step() {
    if (isFinished()) return;

    float prevX = currentX;
    float prevY = currentY;

    currentX += xIncrement;
    currentY += yIncrement;
    currentStep++;

    int px = (int)std::round(currentX);
    int py = (int)std::round(currentY);
    path.push_back({ px, py });

    // ---- Build the detailed calculation record ----
    lastState.hasCalculation = true;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();

    // Sub-step 1: x update
    lastState.calcLines.push_back("--- X-coordinate ---");
    lastState.calcLines.push_back("  x  =  x_prev  +  xInc");
    lastState.calcLines.push_back("  x  =  " + fmt4(prevX) + "  +  " + fmt4(xIncrement));
    lastState.calcLines.push_back("  x  =  " + fmt4(currentX));

    // Sub-step 2: y update
    lastState.calcLines.push_back("--- Y-coordinate ---");
    lastState.calcLines.push_back("  y  =  y_prev  +  yInc");
    lastState.calcLines.push_back("  y  =  " + fmt4(prevY) + "  +  " + fmt4(yIncrement));
    lastState.calcLines.push_back("  y  =  " + fmt4(currentY));

    // Sub-step 3: rounding decision
    lastState.calcLines.push_back("--- Rounding (nearest pixel) ---");
    lastState.calcLines.push_back("  round(" + fmt4(currentX) + ")  =  " + std::to_string(px));
    lastState.calcLines.push_back("  round(" + fmt4(currentY) + ")  =  " + std::to_string(py));

    // Result
    lastState.currentPixelInfo =
        ">> Pixel plotted: (" + std::to_string(px) + ", " + std::to_string(py) + ")";
}

// ================================================================
//  stepK()  —  Advance k steps silently (no detailed calc lines)
// ================================================================
void DDALine::stepK(int k) {
    for (int i = 0; i < k; ++i) {
        if (isFinished()) break;
        currentX += xIncrement;
        currentY += yIncrement;
        currentStep++;
        path.push_back({ (int)std::round(currentX), (int)std::round(currentY) });
    }
    // Update state but mark no detailed calculation
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();
    lastState.currentPixelInfo = "";
}

// ================================================================
//  runToCompletion()  —  Run all remaining steps silently
// ================================================================
void DDALine::runToCompletion() {
    while (!isFinished()) {
        currentX += xIncrement;
        currentY += yIncrement;
        currentStep++;
        path.push_back({ (int)std::round(currentX), (int)std::round(currentY) });
    }
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();
    lastState.currentPixelInfo = "";
}

// ================================================================
//  isFinished()
// ================================================================
bool DDALine::isFinished() const {
    return currentStep >= totalSteps;
}

// ================================================================
//  getHighlightedPixels()
// ================================================================
std::vector<Pixel> DDALine::getHighlightedPixels() const {
    return path;
}

// ================================================================
//  getCurrentState()
// ================================================================
AlgoState DDALine::getCurrentState() const {
    return lastState;
}

// ================================================================
//  getName()
// ================================================================
std::string DDALine::getName() const {
    return "DDA  (Digital Differential Analyzer)";
}

// ================================================================
//  reset()
// ================================================================
void DDALine::reset() {
    init(x1, y1, x2, y2);
}

// ================================================================
//  getTheory()  —  Full educational explanation shown in Theory tab
// ================================================================
std::string DDALine::getTheory() const {
    return
        "DDA — Digital Differential Analyzer\n"
        "====================================\n"
        "\n"
        "WHAT IS IT?\n"
        "  The DDA algorithm is one of the earliest and simplest\n"
        "  line rasterization techniques in Computer Graphics.\n"
        "  It converts a mathematical line defined by two endpoints\n"
        "  into a series of discrete pixels on a grid.\n"
        "\n"
        "CORE IDEA:\n"
        "  Instead of computing y = mx + c for every pixel\n"
        "  (which requires a multiplication per step), DDA adds\n"
        "  a constant floating-point increment to both x and y\n"
        "  at every iteration — only an addition per step.\n"
        "\n"
        "ALGORITHM STEPS:\n"
        "  1. Compute:   dx = x2 - x1\n"
        "                dy = y2 - y1\n"
        "\n"
        "  2. Compute:   steps = max( |dx|, |dy| )\n"
        "     (ensures we never skip a pixel column/row)\n"
        "\n"
        "  3. Compute:   xInc = dx / steps\n"
        "                yInc = dy / steps\n"
        "     (one of these will always be exactly +/-1.0)\n"
        "\n"
        "  4. Set:       x = x1,  y = y1\n"
        "     Plot pixel at ( round(x), round(y) )\n"
        "\n"
        "  5. Repeat 'steps' times:\n"
        "       x = x + xInc\n"
        "       y = y + yInc\n"
        "       Plot pixel at ( round(x), round(y) )\n"
        "\n"
        "WHY max(|dx|, |dy|)?\n"
        "  If slope < 1  (gentle line): |dx| > |dy|, so we step\n"
        "  primarily along X — xInc = 1.0, yInc = slope.\n"
        "  If slope > 1  (steep line):  |dy| > |dx|, so we step\n"
        "  primarily along Y — yInc = 1.0, xInc = 1/slope.\n"
        "  This guarantees a connected, gap-free line.\n"
        "\n"
        "WHY round()?\n"
        "  After each add, x and y are floating-point values.\n"
        "  round() snaps them to the nearest integer grid cell\n"
        "  (the nearest pixel). This is the 'rasterization' step.\n"
        "\n"
        "WORKED EXAMPLE  (0,0) -> (5,3):\n"
        "  dx=5, dy=3, steps=5\n"
        "  xInc=1.0, yInc=0.6\n"
        "  Step 0: (0,   0)  -> pixel (0,0)\n"
        "  Step 1: (1.0, 0.6)-> pixel (1,1)\n"
        "  Step 2: (2.0, 1.2)-> pixel (2,1)\n"
        "  Step 3: (3.0, 1.8)-> pixel (3,2)\n"
        "  Step 4: (4.0, 2.4)-> pixel (4,2)\n"
        "  Step 5: (5.0, 3.0)-> pixel (5,3)\n"
        "\n"
        "ADVANTAGES:\n"
        "  + Simple to understand and implement\n"
        "  + Only addition per step (no multiply per pixel)\n"
        "  + Works for any slope (both |m| < 1 and |m| > 1)\n"
        "\n"
        "DISADVANTAGES:\n"
        "  - Uses floating-point: slower on early hardware\n"
        "  - Rounding error can accumulate for very long lines\n"
        "  - Bresenham's algorithm solves this with integers only\n"
        "\n"
        "TIME COMPLEXITY:  O( max(|dx|, |dy|) )\n"
        "SPACE COMPLEXITY: O( n )  where n = number of pixels\n";
}