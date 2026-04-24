#include "../../include/Visualizer/VisualizerEngine.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>

static const char* ALGO_NAMES[ALGO_COUNT] = {
    "DDA  (Digital Differential Analyzer)",
    "Bresenham's Line Algorithm",
    "Xiaolin Wu's Anti-Aliased Line",
    "Midpoint Circle Algorithm",
    "Bresenham's Circle Algorithm",
    "Midpoint Ellipse Algorithm",
    "Flood Fill (4-Way)",
    "Flood Fill (8-Way)",
    "Boundary Fill (4-Way)",
    "Boundary Fill (8-Way)",
    "Scanline Polygon Fill"
};

// ================================================================
//  Factory: create algorithm by index
// ================================================================
std::unique_ptr<IAlgorithm> VisualizerEngine::createAlgorithm(int idx) {
    switch (idx) {
        case ALGO_BRESENHAM:          return std::make_unique<BresenhamLine>();
        case ALGO_XIAOLIN_WU:         return std::make_unique<XiaolinWuLine>();
        case ALGO_MIDPOINT_CIRCLE:    return std::make_unique<MidpointCircle>();
        case ALGO_BRESENHAM_CIRCLE:   return std::make_unique<BresenhamCircle>();
        case ALGO_MIDPOINT_ELLIPSE:   return std::make_unique<MidpointEllipse>();
        case ALGO_FLOOD_FILL_4:       return std::make_unique<FloodFill4>();
        case ALGO_FLOOD_FILL_8:       return std::make_unique<FloodFill8>();
        case ALGO_BOUNDARY_FILL_4:    return std::make_unique<BoundaryFill4>();
        case ALGO_BOUNDARY_FILL_8:    return std::make_unique<BoundaryFill8>();
        case ALGO_SCANLINE_FILL:      return std::make_unique<ScanlineFill>();
        case ALGO_DDA:
        default:                      return std::make_unique<DDALine>();
    }
}

// ================================================================
//  Constructor
// ================================================================
VisualizerEngine::VisualizerEngine() {
    currentAlgo = createAlgorithm(ALGO_DDA);
    currentAlgo->init(inputX1, inputY1, inputX2, inputY2);
}

// ================================================================
//  renderUI() — full ImGui panel, called every frame
// ================================================================
void VisualizerEngine::renderUI() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(640, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("##VisualizerPanel", nullptr, flags);
    ImGui::SetWindowFontScale(1.08f);

    // ---- Header ----
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 1.0f, 1.0f));
    ImGui::TextUnformatted("SECTION 2 — ALGORITHM VISUALIZER");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // ---- Algorithm dropdown ----
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::TextUnformatted("SELECT ALGORITHM");
    ImGui::PopStyleColor();

    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##AlgoCombo", &selectedAlgo, ALGO_NAMES, ALGO_COUNT)) {
        currentAlgo = createAlgorithm(selectedAlgo);
        if (currentAlgo->isEllipseMode())
            currentAlgo->init(inputCX, inputCY, inputRX, inputRY);
        else if (currentAlgo->isFillMode())
            currentAlgo->init(inputSeedX, inputSeedY, inputHalfS, 0);
        else if (currentAlgo->isScanlineMode())
            currentAlgo->init(inputPolyCX, inputPolyCY, inputPolySize, inputPolySides);
        else if (currentAlgo->isCircleMode())
            currentAlgo->init(inputCX, inputCY, inputRadius, 0);
        else
            currentAlgo->init(inputX1, inputY1, inputX2, inputY2);
        lastMode = ExecMode::NONE;
    }

    ImGui::Separator();
    ImGui::Spacing();

    // ---- Two tabs: Theory | Visualize ----
    if (ImGui::BeginTabBar("##MainTabs")) {
        if (ImGui::BeginTabItem("  Theory  ")) {
            activeTab = 0;
            renderTheoryTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("  Visualize  ")) {
            activeTab = 1;
            renderVisualizeTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

// ================================================================
//  Theory tab
// ================================================================
void VisualizerEngine::renderTheoryTab() {
    ImGui::Spacing();
    ImGui::BeginChild("##TheoryScroll", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    std::string theory = currentAlgo->getTheory();
    std::istringstream stream(theory);
    std::string line;

    while (std::getline(stream, line)) {
        bool isSeparator = line.find("====") != std::string::npos ||
                           line.find("----") != std::string::npos;
        bool isHeading   = !line.empty() && !isSeparator &&
                           (line.back() == ':' || line.back() == '?');
        bool isPlus      = !line.empty() && line[0] == '+';
        bool isMinus     = !line.empty() && line[0] == '-' &&
                           line.size() > 1 && line[1] != '-';

        if (isSeparator) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.35f, 0.45f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else if (isHeading) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else if (isPlus) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.45f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else if (isMinus) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.45f, 0.45f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::TextUnformatted(line.c_str());
        }
    }

    ImGui::EndChild();
}

// ================================================================
//  Visualize tab — 3-box layout, no panel-level scrolling needed
// ================================================================
void VisualizerEngine::renderVisualizeTab() {
    AlgoState state = currentAlgo->getCurrentState();

    // ------------------------------------------------------------------
    //  BOX 1 — Upper scrollable region: inputs + init info + buttons
    //  Fixed height so boxes 2 & 3 are always visible below it.
    // ------------------------------------------------------------------
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.11f, 0.16f, 1.0f));
    ImGui::BeginChild("##UpperBox", ImVec2(0, 380), true,
                      ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PopStyleColor();

    // ---- Inputs ----
    bool isCircle   = currentAlgo->isCircleMode();
    bool isEllipse  = currentAlgo->isEllipseMode();
    bool isFill     = currentAlgo->isFillMode();
    bool isScanline = currentAlgo->isScanlineMode();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::TextUnformatted(isScanline  ? "INPUT  (Polygon: Centre + Size + Sides)" :
                           isFill      ? "INPUT  (Seed + Half-side S)"  :
                           isEllipse   ? "INPUT  (Centre + Semi-axes)" :
                           isCircle    ? "INPUT  (Centre + Radius)"    :
                                         "INPUT COORDINATES");
    ImGui::PopStyleColor();
    ImGui::Separator();

    if (isScanline) {
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Centre X",  &inputPolyCX);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Centre Y",  &inputPolyCY);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Poly Size", &inputPolySize);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Sides (3-10)", &inputPolySides);
        if (inputPolySize  < 2)  inputPolySize  = 2;
        if (inputPolySides < 3)  inputPolySides = 3;
        if (inputPolySides > 10) inputPolySides = 10;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
        ImGui::Text("  Regular %d-gon, radius %d", inputPolySides, inputPolySize);
        ImGui::PopStyleColor();
    } else if (isEllipse) {
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Centre X",   &inputCX);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Centre Y",   &inputCY);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Semi-a (X)", &inputRX);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Semi-b (Y)", &inputRY);
        if (inputRX < 1) inputRX = 1;
        if (inputRY < 1) inputRY = 1;
    } else if (isFill) {
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Seed X (cx)",  &inputSeedX);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Seed Y (cy)",  &inputSeedY);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Half-side S",  &inputHalfS);
        if (inputHalfS < 2) inputHalfS = 2;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
        ImGui::Text("  Boundary: [%d..%d] x [%d..%d]",
                    inputSeedX - inputHalfS, inputSeedX + inputHalfS,
                    inputSeedY - inputHalfS, inputSeedY + inputHalfS);
        ImGui::PopStyleColor();
    } else if (isCircle) {
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Centre X", &inputCX);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Centre Y", &inputCY);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Radius",   &inputRadius);
        if (inputRadius < 1) inputRadius = 1;
    } else {
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Start X", &inputX1);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("Start Y", &inputY1);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("End X",   &inputX2);
        ImGui::SetNextItemWidth(180); ImGui::InputInt("End Y",   &inputY2);
    }

    ImGui::Spacing();
    if (ImGui::Button("  Apply & Reset  ", ImVec2(-1, 0))) {
        if (isScanline)     currentAlgo->init(inputPolyCX, inputPolyCY, inputPolySize, inputPolySides);
        else if (isEllipse) currentAlgo->init(inputCX, inputCY, inputRX, inputRY);
        else if (isFill)    currentAlgo->init(inputSeedX, inputSeedY, inputHalfS, 0);
        else if (isCircle)  currentAlgo->init(inputCX, inputCY, inputRadius, 0);
        else                currentAlgo->init(inputX1, inputY1, inputX2, inputY2);
        lastMode = ExecMode::NONE;
    }


    // ---- Init info ----
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::TextUnformatted("INITIALIZATION");
    ImGui::PopStyleColor();
    ImGui::Separator();
    for (const auto& s : currentAlgo->getInitInfo())
        ImGui::Text("  %s", s.c_str());

    // ---- Execution buttons ----
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::TextUnformatted("EXECUTION");
    ImGui::PopStyleColor();
    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.70f, 1.0f));
    ImGui::TextWrapped("Step +1: full calc  |  Step K / Run All: silent");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Step +1 (green)
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.55f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.75f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f, 0.45f, 0.10f, 1.0f));
    if (ImGui::Button("  Step +1  (shows calculation)  ", ImVec2(-1, 0))) {
        if (!currentAlgo->isFinished()) {
            currentAlgo->step();
            lastMode = ExecMode::STEP_ONE;
        }
    }
    ImGui::PopStyleColor(3);

    // Step K (blue)
    ImGui::Spacing();
    ImGui::SetNextItemWidth(130);
    ImGui::InputInt("K##steps", &kSteps);
    if (kSteps < 1) kSteps = 1;
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.35f, 0.65f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.50f, 0.85f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f, 0.28f, 0.55f, 1.0f));
    if (ImGui::Button("Step K  (silent)", ImVec2(-1, 0))) {
        currentAlgo->stepK(kSteps);
        lastMode = ExecMode::STEP_K;
    }
    ImGui::PopStyleColor(3);

    // Run All (orange)
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.65f, 0.28f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.38f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.55f, 0.20f, 0.05f, 1.0f));
    if (ImGui::Button("  Run to Completion  (silent)  ", ImVec2(-1, 0))) {
        currentAlgo->runToCompletion();
        lastMode = ExecMode::RUN_ALL;
    }
    ImGui::PopStyleColor(3);

    // Reset (purple)
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.40f, 0.10f, 0.40f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 0.60f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.30f, 0.05f, 0.30f, 1.0f));
    if (ImGui::Button("  Reset  ", ImVec2(-1, 0))) {
        currentAlgo->reset();
        lastMode = ExecMode::NONE;
    }
    ImGui::PopStyleColor(3);

    ImGui::EndChild();   // End of BOX 1

    // ------------------------------------------------------------------
    //  BOX 2 — Current State (always visible, fixed height ~145px)
    // ------------------------------------------------------------------
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::TextUnformatted("  CURRENT STATE");
    ImGui::PopStyleColor();

    state = currentAlgo->getCurrentState();  // refresh

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.10f, 0.15f, 1.0f));
    ImGui::BeginChild("##StateBox", ImVec2(0, 220), true);
    ImGui::PopStyleColor();

    // Progress bar
    float progress = (state.totalSteps > 0)
        ? (float)state.currentStep / (float)state.totalSteps : 1.0f;
    char lbl[64];
    snprintf(lbl, sizeof(lbl), "Step %d / %d", state.currentStep, state.totalSteps);
    ImGui::ProgressBar(progress, ImVec2(-1, 0), lbl);
    ImGui::Spacing();

    for (const auto& v : currentAlgo->getCurrentVars())
        ImGui::Text("  %s", v.c_str());

    if (currentAlgo->isFinished()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::TextUnformatted("  *** Complete! ***");
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();   // End of BOX 2

    // ------------------------------------------------------------------
    //  BOX 3 — Step Calculation (always visible, fills remaining space)
    //  Shows hint text when not in STEP_ONE mode.
    // ------------------------------------------------------------------
    ImGui::Spacing();

    if (lastMode == ExecMode::STEP_ONE && state.hasCalculation) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
        ImGui::Text("  STEP %d  CALCULATION", state.currentStep);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.65f, 1.0f));
        ImGui::TextUnformatted("  STEP CALCULATION");
        ImGui::PopStyleColor();
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.10f, 0.15f, 1.0f));
    // No BeginChild — content renders directly so nothing is clipped or scrollable

    if (lastMode == ExecMode::STEP_ONE && state.hasCalculation && !state.calcLines.empty()) {
        // ---- Full calculation ----
        for (const auto& line : state.calcLines) {
            if (line.find("---") != std::string::npos) {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.85f, 1.0f, 1.0f));
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();
            } else if (line.empty()) {
                ImGui::Spacing();
            } else if (line.find("YES") != std::string::npos) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.3f, 1.0f));
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();
            } else if (line.find("stays") != std::string::npos) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.4f, 0.4f, 1.0f));
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();
            } else if (line.find("increments") != std::string::npos ||
                       line.find("decrements") != std::string::npos) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.5f, 1.0f));
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();
            } else if (line.find(">>") != std::string::npos) {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::TextUnformatted(line.c_str());
            }
        }
        if (!state.currentPixelInfo.empty()) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
            ImGui::TextUnformatted(state.currentPixelInfo.c_str());
            ImGui::PopStyleColor();
        }
    } else {
        // ---- Hint ----
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.42f, 0.42f, 0.52f, 1.0f));
        ImGui::TextWrapped(
            "Click  'Step +1'  to execute one step and see the full\n"
            "mathematical calculation displayed here.\n\n"
            "'Step K'  and  'Run All'  advance silently\n"
            "(no per-step math shown, only result).");
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor();  // ChildBg (unused now, but keeps push/pop balanced)
}

// ================================================================
//  renderCalcPanel — kept for compatibility (no longer called)
// ================================================================
void VisualizerEngine::renderCalcPanel(const AlgoState& /*state*/) {
    // Calculation is now rendered inline in renderVisualizeTab BOX 3.
}





// ================================================================
//  renderGrid() — OpenGL pixel grid, called after ImGui render
// ================================================================
void VisualizerEngine::renderGrid(int windowWidth, int windowHeight) {
    const int panelW  = 640;
    int gridAreaW = windowWidth - panelW;
    int gridAreaH = windowHeight;
    if (gridAreaW <= 0 || gridAreaH <= 0 || activeTab != 1) return;

    glViewport(panelW, 0, gridAreaW, gridAreaH);

    bool isCircle   = currentAlgo->isCircleMode();
    bool isEllipse  = currentAlgo->isEllipseMode();
    bool isFill     = currentAlgo->isFillMode();
    bool isScanline = currentAlgo->isScanlineMode();

    // ---- Compute viewport bounds ----
    float gMinX, gMinY, gMaxX, gMaxY;
    if (isScanline) {
        // Centre on polygon, pad by polySize
        float pad    = (float)inputPolySize * 1.6f;
        if (pad < 5.0f) pad = 5.0f;
        float aspect = (float)gridAreaH / (float)gridAreaW;
        gMinX = (float)inputPolyCX - pad;
        gMaxX = (float)inputPolyCX + pad;
        gMinY = (float)inputPolyCY - pad * aspect;
        gMaxY = (float)inputPolyCY + pad * aspect;
    } else if (isFill) {
        // Dynamic: centre on seed, pad by halfS + margin
        float pad    = (float)inputHalfS * 1.55f;
        if (pad < 5.0f) pad = 5.0f;
        float aspect = (float)gridAreaH / (float)gridAreaW;
        gMinX = (float)inputSeedX - pad;
        gMaxX = (float)inputSeedX + pad;
        gMinY = (float)inputSeedY - pad * aspect;
        gMaxY = (float)inputSeedY + pad * aspect;
    } else if (isEllipse) {
        int maxR = std::max(std::abs(inputRX), std::abs(inputRY));
        float pad = (float)maxR * 1.55f;
        if (pad < 15.0f) pad = 15.0f;
        float aspect = (float)gridAreaH / (float)gridAreaW;
        gMinX = (float)inputCX - pad;
        gMaxX = (float)inputCX + pad;
        gMinY = (float)inputCY - pad * aspect;
        gMaxY = (float)inputCY + pad * aspect;
    } else if (isCircle) {
        float pad = (float)inputRadius * 1.45f;
        if (pad < 15.0f) pad = 15.0f;
        float aspect = (float)gridAreaH / (float)gridAreaW;
        gMinX = (float)inputCX - pad;
        gMaxX = (float)inputCX + pad;
        gMinY = (float)inputCY - pad * aspect;
        gMaxY = (float)inputCY + pad * aspect;
    } else {
        int maxCoord = std::max({ std::abs(inputX1), std::abs(inputY1),
                                  std::abs(inputX2), std::abs(inputY2), 1 });
        float gridMax  = (float)maxCoord * 1.3f;
        if (gridMax < 20.0f) gridMax = 20.0f;
        float gridMaxY = gridMax * ((float)gridAreaH / (float)gridAreaW);
        gMinX = -0.5f;  gMaxX = gridMax + 0.5f;
        gMinY = -0.5f;  gMaxY = gridMaxY + 0.5f;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(gMinX, gMaxX, gMinY, gMaxY, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ---- Grid lines ----
    glColor3f(0.22f, 0.25f, 0.32f);
    glBegin(GL_LINES);
    for (int i = (int)std::floor(gMinX); i <= (int)std::ceil(gMaxX); i++) {
        glVertex2f((float)i, gMinY); glVertex2f((float)i, gMaxY);
    }
    for (int i = (int)std::floor(gMinY); i <= (int)std::ceil(gMaxY); i++) {
        glVertex2f(gMinX, (float)i); glVertex2f(gMaxX, (float)i);
    }
    glEnd();

    // ---- Axes (only if 0 is in range) ----
    glColor3f(0.40f, 0.43f, 0.52f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    if (0.0f >= gMinX && 0.0f <= gMaxX) {
        glVertex2f(0.0f, gMinY); glVertex2f(0.0f, gMaxY);
    }
    if (0.0f >= gMinY && 0.0f <= gMaxY) {
        glVertex2f(gMinX, 0.0f); glVertex2f(gMaxX, 0.0f);
    }
    glEnd();
    glLineWidth(1.0f);

    // ---- Reference shape (dashed) ----
    glColor3f(0.45f, 0.45f, 0.75f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xAAAA);

    if (isScanline) {
        // Polygon outline using GL_LINE_LOOP through computed vertices
        float angleStep  = 2.0f * 3.14159265f / (float)inputPolySides;
        float startAngle = -3.14159265f / 2.0f;
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < inputPolySides; i++) {
            float a = startAngle + (float)i * angleStep;
            glVertex2f((float)inputPolyCX + 0.5f + (float)inputPolySize * cosf(a),
                       (float)inputPolyCY + 0.5f + (float)inputPolySize * sinf(a));
        }
        glEnd();
    } else if (isFill) {
        // Draw the configurable square boundary as a dashed rectangle
        float lo_x = (float)(inputSeedX - inputHalfS);
        float hi_x = (float)(inputSeedX + inputHalfS) + 1.0f;
        float lo_y = (float)(inputSeedY - inputHalfS);
        float hi_y = (float)(inputSeedY + inputHalfS) + 1.0f;
        glBegin(GL_LINE_LOOP);
        glVertex2f(lo_x, lo_y);
        glVertex2f(hi_x, lo_y);
        glVertex2f(hi_x, hi_y);
        glVertex2f(lo_x, hi_y);
        glEnd();
    } else if (isEllipse) {
        glBegin(GL_LINE_LOOP);
        for (int deg = 0; deg < 360; deg++) {
            float rad = (float)deg * 3.14159265f / 180.0f;
            glVertex2f((float)inputCX + 0.5f + inputRX * cosf(rad),
                       (float)inputCY + 0.5f + inputRY * sinf(rad));
        }
        glEnd();
    } else if (isCircle) {
        glBegin(GL_LINE_LOOP);
        for (int deg = 0; deg < 360; deg++) {
            float rad = (float)deg * 3.14159265f / 180.0f;
            glVertex2f((float)inputCX + 0.5f + inputRadius * cosf(rad),
                       (float)inputCY + 0.5f + inputRadius * sinf(rad));
        }
        glEnd();
    } else {
        glBegin(GL_LINES);
        glVertex2f((float)inputX1+0.5f, (float)inputY1+0.5f);
        glVertex2f((float)inputX2+0.5f, (float)inputY2+0.5f);
        glEnd();
    }

    glDisable(GL_LINE_STIPPLE);


    // ---- Plotted pixels — alpha blended for Wu intensity ----
    std::vector<Pixel> pixels = currentAlgo->getHighlightedPixels();
    AlgoState state  = currentAlgo->getCurrentState();
    int numPixels    = (int)pixels.size();

    int highlightStart = -1;
    if (lastMode == ExecMode::STEP_ONE && numPixels > 0) {
        highlightStart = numPixels - state.pixelsPerStep;
        if (highlightStart < 0) highlightStart = 0;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    for (int i = 0; i < numPixels; i++) {
        float px = (float)pixels[i].x;
        float py = (float)pixels[i].y;
        float iv = pixels[i].intensity;

        // --- Pixel color: isCurrentStep fires for ANY non-boundary type ---
        bool isCurrentStep = (highlightStart >= 0 && i >= highlightStart
                              && pixels[i].type != 3);

        if (isCurrentStep) {
            // Yellow: current-step highlight (works for line, fill, scanline)
            glColor4f(1.0f, 0.95f, 0.15f, iv);
        } else if (pixels[i].type == 3) {
            // Obstacle/Boundary (Grey)
            glColor4f(0.5f, 0.5f, 0.55f, 1.0f);
        } else if (pixels[i].type == 2) {
            // Filled (Green)
            glColor4f(0.2f, 0.8f, 0.35f, iv);
        } else if (pixels[i].type == 1) {
            // Frontier (Orange)
            glColor4f(1.0f, 0.6f, 0.0f, iv);
        } else if (i == 0 && pixels[i].type == 0) {
            // Start pixel for line/circle (Bright Green)
            glColor4f(0.2f, 0.9f, 0.35f, iv);
        } else {
            // Default line/circle pixel (Red)
            glColor4f(0.85f, 0.22f, 0.18f, iv);
        }

        glVertex2f(px+0.07f, py+0.07f);
        glVertex2f(px+0.93f, py+0.07f);
        glVertex2f(px+0.93f, py+0.93f);
        glVertex2f(px+0.07f, py+0.93f);
    }
    glEnd();
    glDisable(GL_BLEND);

    // ---- Markers ----
    glLineWidth(2.0f);
    if (isScanline) {
        // Polygon centre dot (cyan)
        glColor3f(0.3f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        float bx = (float)inputPolyCX + 0.25f, by = (float)inputPolyCY + 0.25f;
        glVertex2f(bx,      by);
        glVertex2f(bx+0.5f, by);
        glVertex2f(bx+0.5f, by+0.5f);
        glVertex2f(bx,      by+0.5f);
        glEnd();
    } else if (isFill) {
        // Seed dot (pink)
        glColor3f(1.0f, 0.2f, 0.8f);
        glBegin(GL_QUADS);
        float bx = (float)inputSeedX + 0.3f, by = (float)inputSeedY + 0.3f;
        glVertex2f(bx,      by);
        glVertex2f(bx+0.4f, by);
        glVertex2f(bx+0.4f, by+0.4f);
        glVertex2f(bx,      by+0.4f);
        glEnd();
    } else if (isCircle || isEllipse) {
        // Centre dot
        glColor3f(0.2f, 1.0f, 0.4f);
        glBegin(GL_QUADS);
        float bx = (float)inputCX + 0.3f, by = (float)inputCY + 0.3f;
        glVertex2f(bx,      by);
        glVertex2f(bx+0.4f, by);
        glVertex2f(bx+0.4f, by+0.4f);
        glVertex2f(bx,      by+0.4f);
        glEnd();
    } else {
        // Start marker (green border)
        glColor3f(0.2f, 1.0f, 0.4f);
        float sx = (float)inputX1, sy = (float)inputY1;
        glBegin(GL_LINE_LOOP);
        glVertex2f(sx+0.02f, sy+0.02f); glVertex2f(sx+0.98f, sy+0.02f);
        glVertex2f(sx+0.98f, sy+0.98f); glVertex2f(sx+0.02f, sy+0.98f);
        glEnd();
        // End marker (blue border)
        glColor3f(0.35f, 0.65f, 1.0f);
        float ex = (float)inputX2, ey = (float)inputY2;
        glBegin(GL_LINE_LOOP);
        glVertex2f(ex+0.02f, ey+0.02f); glVertex2f(ex+0.98f, ey+0.02f);
        glVertex2f(ex+0.98f, ey+0.98f); glVertex2f(ex+0.02f, ey+0.98f);
        glEnd();
    }
    glLineWidth(1.0f);

    glViewport(0, 0, windowWidth, windowHeight);
}
