#pragma once
#include "LineAlgos.h"

// ============================================================
//  Execution mode — controls whether the calculation panel
//  is shown in the UI after a button press.
// ============================================================
enum class ExecMode {
    NONE,       // just initialised / reset
    STEP_ONE,   // user pressed "Step +1" → show calculations
    STEP_K,     // user pressed "Step K"  → hide calculations
    RUN_ALL     // user pressed "Run All" → hide calculations
};

// ============================================================
//  VisualizerEngine
//  Owns one DDALine instance and renders:
//    • A Dear ImGui panel (Theory tab + Visualize tab)
//    • An OpenGL pixel grid with dynamic zoom
// ============================================================
class VisualizerEngine {
private:
    DDALine ddaAlgorithm;

    // UI input state
    int inputX1 = 0,  inputY1 = 0;
    int inputX2 = 10, inputY2 = 8;
    int kSteps  = 3;

    // Which tab is open: 0 = Theory, 1 = Visualize
    int activeTab = 1;

    // Last execution mode (controls calc panel visibility)
    ExecMode lastMode = ExecMode::NONE;

    // ---- private render helpers ----
    void renderTheoryTab();
    void renderVisualizeTab();
    void renderCalcPanel(const AlgoState& state);
    void renderGridGL(int windowWidth, int windowHeight);

public:
    VisualizerEngine();

    // Called every frame from main.cpp
    void renderUI();
    void renderGrid(int windowWidth, int windowHeight);
};