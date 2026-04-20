#pragma once
#include "LineAlgos.h"
#include "CircleAlgos.h"
#include "EllipseAlgos.h"
#include "FillAlgos.h"
#include <memory>

// ============================================================
//  Execution mode — controls whether the calc panel is shown
// ============================================================
enum class ExecMode { NONE, STEP_ONE, STEP_K, RUN_ALL };

// ============================================================
//  Algorithm index constants
// ============================================================
static constexpr int ALGO_DDA              = 0;
static constexpr int ALGO_BRESENHAM        = 1;
static constexpr int ALGO_XIAOLIN_WU       = 2;
static constexpr int ALGO_MIDPOINT_CIRCLE  = 3;
static constexpr int ALGO_BRESENHAM_CIRCLE = 4;
static constexpr int ALGO_MIDPOINT_ELLIPSE = 5;
static constexpr int ALGO_FLOOD_FILL_4       = 6;
static constexpr int ALGO_FLOOD_FILL_8       = 7;
static constexpr int ALGO_BOUNDARY_FILL_4    = 8;
static constexpr int ALGO_BOUNDARY_FILL_8    = 9;
static constexpr int ALGO_SCANLINE_FILL      = 10;
static constexpr int ALGO_COUNT              = 11;

// ============================================================
//  VisualizerEngine
//  Algorithm-agnostic: everything rendered through IAlgorithm*.
// ============================================================
class VisualizerEngine {
private:
    std::unique_ptr<IAlgorithm> currentAlgo;

    int selectedAlgo   = ALGO_DDA;  // active algorithm index
    int prevAlgo       = -1;        // detects combo-box change

    // Line-mode inputs
    int inputX1 = 0,  inputY1 = 0;
    int inputX2 = 10, inputY2 = 8;
    int kSteps  = 3;

    // Circle-mode inputs
    int inputCX = 0, inputCY = 0, inputRadius = 8;

    // Ellipse-mode inputs
    int inputRX = 10, inputRY = 6;

    // Fill-mode inputs  (seed = center of the square boundary)
    int inputSeedX = 10, inputSeedY = 10, inputHalfS = 8;

    // Scanline-mode inputs  (regular polygon)
    int inputPolyCX = 10, inputPolyCY = 10, inputPolySize = 7, inputPolySides = 5;

    // UI state
    int      activeTab = 1;           // 0=Theory, 1=Visualize
    ExecMode lastMode  = ExecMode::NONE;

    // ---- private helpers ----
    std::unique_ptr<IAlgorithm> createAlgorithm(int idx);

    void renderTheoryTab();
    void renderVisualizeTab();
    void renderCalcPanel(const AlgoState& state);

public:
    VisualizerEngine();

    void renderUI();
    void renderGrid(int windowWidth, int windowHeight);
};