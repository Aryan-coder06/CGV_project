#include "../../include/Visualizer/VisualizerEngine.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>

// ================================================================
//  Constructor
// ================================================================
VisualizerEngine::VisualizerEngine() {
    ddaAlgorithm.init(inputX1, inputY1, inputX2, inputY2);
    lastMode = ExecMode::NONE;
}

// ================================================================
//  renderUI() — main ImGui panel, called every frame
// ================================================================
void VisualizerEngine::renderUI() {

    // ---- Set panel position and size (left ~380 px wide) ----
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(390, 720), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove     |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("##VisualizerPanel", nullptr, flags);

    // =========================================================
    //  Header
    // =========================================================
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 1.0f, 1.0f));
    ImGui::TextWrapped("SECTION 2 — ALGORITHM VISUALIZER");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::TextWrapped("%s", ddaAlgorithm.getName().c_str());
    ImGui::PopStyleColor();

    ImGui::Separator();
    ImGui::Spacing();

    // =========================================================
    //  Two top-level tabs: Theory | Visualize
    // =========================================================
    if (ImGui::BeginTabBar("##MainTabs")) {

        // ---- TAB 1: Theory ----
        if (ImGui::BeginTabItem("  Theory  ")) {
            activeTab = 0;
            renderTheoryTab();
            ImGui::EndTabItem();
        }

        // ---- TAB 2: Visualize ----
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
//  renderTheoryTab()
// ================================================================
void VisualizerEngine::renderTheoryTab() {
    ImGui::Spacing();

    // Scrollable text region for the theory
    ImGui::BeginChild("##TheoryScroll", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    std::string theory = ddaAlgorithm.getTheory();

    // Split the theory string by newline and render each line
    // so that wrapping + colour-coding is possible per section.
    std::istringstream stream(theory);
    std::string line;
    while (std::getline(stream, line)) {
        // Colour section headings differently
        bool isHeading =
            (!line.empty() && line.find("---") == std::string::npos &&
             (line.back() == ':' || line.find("===") != std::string::npos ||
              line.find("---") != std::string::npos));

        if (line.find("====") != std::string::npos ||
            line.find("----") != std::string::npos) {
            // Separator line — render as small text
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else if (isHeading) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else if (!line.empty() && line[0] == '+') {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else if (!line.empty() && line[0] == '-') {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::TextUnformatted(line.c_str());
        }
    }

    ImGui::EndChild();
}

// ================================================================
//  renderVisualizeTab()
// ================================================================
void VisualizerEngine::renderVisualizeTab() {
    ImGui::Spacing();
    AlgoState state = ddaAlgorithm.getCurrentState();

    // ---- Section: Coordinate Inputs ----
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::Text("INPUT COORDINATES");
    ImGui::PopStyleColor();
    ImGui::Separator();

    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("Start X", &inputX1);
    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("Start Y", &inputY1);
    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("End X",   &inputX2);
    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("End Y",   &inputY2);

    ImGui::Spacing();
    if (ImGui::Button("  Apply & Reset  ", ImVec2(-1, 0))) {
        ddaAlgorithm.init(inputX1, inputY1, inputX2, inputY2);
        lastMode = ExecMode::NONE;
    }

    // ---- Section: Initialisation Info ----
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::Text("INITIALIZATION");
    ImGui::PopStyleColor();
    ImGui::Separator();

    int dx = inputX2 - inputX1;
    int dy = inputY2 - inputY1;
    int steps = std::max(std::abs(dx), std::abs(dy));
    float xInc = (steps > 0) ? (float)dx / steps : 0.0f;
    float yInc = (steps > 0) ? (float)dy / steps : 0.0f;

    ImGui::Text("  dx = %d,  dy = %d", dx, dy);
    ImGui::Text("  steps = max(|%d|, |%d|) = %d", dx, dy, steps);
    ImGui::Text("  xInc = %d / %d = %.4f", dx, steps, xInc);
    ImGui::Text("  yInc = %d / %d = %.4f", dy, steps, yInc);

    // ---- Section: Execution Controls ----
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::Text("EXECUTION");
    ImGui::PopStyleColor();
    ImGui::Separator();

    // Note about what each mode shows
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.65f, 1.0f));
    ImGui::TextWrapped("Step +1 shows full calculation.");
    ImGui::TextWrapped("Step K and Run All show result only.");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // --- Step +1 ---
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
    if (ImGui::Button("  Step +1  (shows calculation)  ", ImVec2(-1, 0))) {
        if (!ddaAlgorithm.isFinished()) {
            ddaAlgorithm.step();
            lastMode = ExecMode::STEP_ONE;
        }
    }
    ImGui::PopStyleColor(3);

    // --- Step K ---
    ImGui::Spacing();
    ImGui::SetNextItemWidth(80);
    ImGui::InputInt("K (steps)", &kSteps);
    if (kSteps < 1) kSteps = 1;

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.1f, 0.3f, 0.6f, 1.0f));
    if (ImGui::Button("  Step K  (silent)  ", ImVec2(-1, 0))) {
        ddaAlgorithm.stepK(kSteps);
        lastMode = ExecMode::STEP_K;
    }
    ImGui::PopStyleColor(3);

    // --- Run All ---
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.7f, 0.3f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.6f, 0.2f, 0.0f, 1.0f));
    if (ImGui::Button("  Run to Completion  (silent)  ", ImVec2(-1, 0))) {
        ddaAlgorithm.runToCompletion();
        lastMode = ExecMode::RUN_ALL;
    }
    ImGui::PopStyleColor(3);

    // --- Reset ---
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.4f, 0.1f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.3f, 0.0f, 0.3f, 1.0f));
    if (ImGui::Button("  Reset  ", ImVec2(-1, 0))) {
        ddaAlgorithm.reset();
        lastMode = ExecMode::NONE;
    }
    ImGui::PopStyleColor(3);

    // ---- Section: Current State ----
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::Text("CURRENT STATE");
    ImGui::PopStyleColor();
    ImGui::Separator();

    state = ddaAlgorithm.getCurrentState();  // refresh

    // Progress bar
    float progress = (state.totalSteps > 0)
        ? (float)state.currentStep / (float)state.totalSteps
        : 1.0f;
    char progressLabel[64];
    snprintf(progressLabel, sizeof(progressLabel),
             "Step %d / %d", state.currentStep, state.totalSteps);
    ImGui::ProgressBar(progress, ImVec2(-1, 0), progressLabel);
    ImGui::Spacing();

    ImGui::Text("  Float X : %.4f",    ddaAlgorithm.getCurrentX());
    ImGui::Text("  Float Y : %.4f",    ddaAlgorithm.getCurrentY());
    ImGui::Text("  xInc    : %.4f",    ddaAlgorithm.getXInc());
    ImGui::Text("  yInc    : %.4f",    ddaAlgorithm.getYInc());
    ImGui::Text("  Pixels plotted: %d", (int)ddaAlgorithm.getHighlightedPixels().size());

    // Completion badge
    if (ddaAlgorithm.isFinished()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::Text("  *** Algorithm Complete! Line fully rasterized. ***");
        ImGui::PopStyleColor();
    }

    // ---- Section: Calculation Panel (Step +1 only) ----
    if (lastMode == ExecMode::STEP_ONE) {
        ImGui::Spacing();
        renderCalcPanel(state);
    }
}

// ================================================================
//  renderCalcPanel() — shows the detailed math for one step
// ================================================================
void VisualizerEngine::renderCalcPanel(const AlgoState& state) {
    if (!state.hasCalculation || state.calcLines.empty()) return;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::Text("STEP %d CALCULATION", state.currentStep);
    ImGui::PopStyleColor();
    ImGui::Separator();

    // Scrollable box so long outputs don't overflow the panel
    ImGui::BeginChild("##CalcScroll", ImVec2(0, 200), true);

    for (const auto& line : state.calcLines) {
        // Colour code sub-section headings vs values
        if (line.find("---") != std::string::npos) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 1.0f, 1.0f));
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::TextUnformatted(line.c_str());
        }
    }

    // Final result — printed in green and bold-ish
    if (!state.currentPixelInfo.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::TextUnformatted(state.currentPixelInfo.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

// ================================================================
//  renderGrid() — called from main.cpp after the ImGui render
// ================================================================
void VisualizerEngine::renderGrid(int windowWidth, int windowHeight) {
    // The grid panel sits to the right of the ImGui panel (390px)
    int panelWidth = 390;
    int gridAreaW  = windowWidth - panelWidth;
    int gridAreaH  = windowHeight;

    if (gridAreaW <= 0 || gridAreaH <= 0) return;

    // Only render on the Visualize tab
    if (activeTab != 1) return;

    // ---- Viewport: right portion of the window ----
    glViewport(panelWidth, 0, gridAreaW, gridAreaH);

    // ---- Dynamic zoom ----
    // Find the bounding box of the line
    int maxCoord = std::max({ std::abs(inputX1), std::abs(inputY1),
                              std::abs(inputX2), std::abs(inputY2), 1 });
    float gridMax = (float)maxCoord * 1.3f;   // 30 % padding around the line
    if (gridMax < 20.0f) gridMax = 20.0f;     // minimum zoom: 20-unit grid

    // Keep aspect ratio square
    float aspectRatio = (float)gridAreaH / (float)gridAreaW;
    float gridMaxY    = gridMax * aspectRatio;

    // ---- Projection ----
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Slightly negative origin so the (0,0) cell is fully visible
    glOrtho(-0.5, gridMax + 0.5, -0.5, gridMaxY + 0.5, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // ---- Background ----
    glClearColor(0.12f, 0.12f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ================================================================
    //  Draw grid lines
    // ================================================================
    glColor3f(0.25f, 0.28f, 0.35f);
    glBegin(GL_LINES);
    for (int i = 0; i <= (int)gridMax + 1; i++) {
        // Vertical line
        glVertex2f((float)i,        -0.5f);
        glVertex2f((float)i,        gridMaxY + 0.5f);
        // Horizontal line
        glVertex2f(-0.5f,           (float)i);
        glVertex2f(gridMax + 0.5f,  (float)i);
    }
    glEnd();

    // Highlight the X and Y axes slightly brighter
    glColor3f(0.45f, 0.48f, 0.55f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(0.0f,       -0.5f);      glVertex2f(0.0f,       gridMaxY + 0.5f);
    glVertex2f(-0.5f,       0.0f);      glVertex2f(gridMax + 0.5f, 0.0f);
    glEnd();
    glLineWidth(1.0f);

    // ================================================================
    //  Draw plotted pixels
    // ================================================================
    std::vector<Pixel> pixels = ddaAlgorithm.getHighlightedPixels();
    AlgoState          state  = ddaAlgorithm.getCurrentState();

    // The last pixel in the path is the most recently plotted one
    int lastIdx = (int)pixels.size() - 1;

    glBegin(GL_QUADS);
    for (int i = 0; i < (int)pixels.size(); i++) {
        const Pixel& p = pixels[i];

        if (i == 0) {
            // Start pixel — green
            glColor3f(0.2f, 0.9f, 0.4f);
        } else if (i == lastIdx && lastMode == ExecMode::STEP_ONE) {
            // Most recently plotted pixel (step mode) — bright yellow
            glColor3f(1.0f, 1.0f, 0.2f);
        } else {
            // Previously plotted pixels — red-orange
            glColor3f(0.85f, 0.25f, 0.2f);
        }

        // Draw a filled square for each pixel cell
        float x = (float)p.x;
        float y = (float)p.y;
        glVertex2f(x + 0.05f, y + 0.05f);
        glVertex2f(x + 0.95f, y + 0.05f);
        glVertex2f(x + 0.95f, y + 0.95f);
        glVertex2f(x + 0.05f, y + 0.95f);
    }
    glEnd();

    // ================================================================
    //  Draw the ideal mathematical line (thin, dim, for reference)
    // ================================================================
    glColor4f(0.6f, 0.6f, 1.0f, 0.5f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xAAAA);   // dashed
    glBegin(GL_LINES);
    glVertex2f((float)inputX1 + 0.5f, (float)inputY1 + 0.5f);
    glVertex2f((float)inputX2 + 0.5f, (float)inputY2 + 0.5f);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // ================================================================
    //  Draw start and end markers
    // ================================================================
    // Start: solid green circle-ish (a small square)
    glColor3f(0.2f, 1.0f, 0.4f);
    glBegin(GL_QUADS);
    float sx = (float)inputX1, sy = (float)inputY1;
    glVertex2f(sx + 0.1f, sy + 0.1f);
    glVertex2f(sx + 0.9f, sy + 0.1f);
    glVertex2f(sx + 0.9f, sy + 0.9f);
    glVertex2f(sx + 0.1f, sy + 0.9f);
    glEnd();

    // End: solid blue
    glColor3f(0.3f, 0.6f, 1.0f);
    glBegin(GL_QUADS);
    float ex = (float)inputX2, ey = (float)inputY2;
    glVertex2f(ex + 0.1f, ey + 0.1f);
    glVertex2f(ex + 0.9f, ey + 0.1f);
    glVertex2f(ex + 0.9f, ey + 0.9f);
    glVertex2f(ex + 0.1f, ey + 0.9f);
    glEnd();

    // Reset viewport for ImGui (render to full window)
    glViewport(0, 0, windowWidth, windowHeight);
}