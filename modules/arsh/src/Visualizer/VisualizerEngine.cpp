#include "../../include/Visualizer/VisualizerEngine.h"
#include "UI/RetroTheme.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>
#include <cstring>

static std::vector<int> parseCSV(const char* buf) {
    std::vector<int> result;
    std::string s(buf);
    std::istringstream stream(s);
    std::string token;
    while (std::getline(stream, token, ',')) {
        size_t start = token.find_first_not_of(" \t");
        size_t end = token.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        token = token.substr(start, end - start + 1);
        if (token.empty()) continue;
        try {
            int val = std::stoi(token);
            if (val > 0 && val < 1000) result.push_back(val);
        } catch (...) {}
    }
    return result;
}

static const char* SORT_NAMES[]   = {"Bubble Sort","Selection Sort","Insertion Sort","Merge Sort","Quick Sort","Heap Sort"};
static const char* SEARCH_NAMES[] = {"Linear Search","Binary Search"};
static const char* GRAPH_NAMES[]  = {"BFS (Breadth-First)","DFS (Depth-First)", "Dijkstra's Algorithm"};

static int sortCount   = 6;
static int searchCount = 2;
static int graphCount  = 3;

std::unique_ptr<IAlgorithm> VisualizerEngine::createAlgorithm(Category cat, int idx) {
    switch (cat) {
        case Category::SORT:
            switch (idx) {
                case 0: return std::make_unique<BubbleSort>();
                case 1: return std::make_unique<SelectionSort>();
                case 2: return std::make_unique<InsertionSort>();
                case 3: return std::make_unique<MergeSort>();
                case 4: return std::make_unique<QuickSort>();
                case 5: return std::make_unique<HeapSort>();
                default: return std::make_unique<BubbleSort>();
            }
        case Category::SEARCH:
            switch (idx) {
                case 0: return std::make_unique<LinearSearch>();
                case 1: return std::make_unique<BinarySearch>();
                default: return std::make_unique<LinearSearch>();
            }
        case Category::GRAPH:
            switch (idx) {
                case 0: return std::make_unique<BFS>();
                case 1: return std::make_unique<DFS>();
                case 2: return std::make_unique<Dijkstra>();
                default: return std::make_unique<BFS>();
            }
    }
    return std::make_unique<BubbleSort>();
}

void VisualizerEngine::selectAlgorithm(Category cat, int idx) {
    currentCategory = cat;
    selectedAlgoInCategory = idx;
    isPlaying = false;
    stepTimer = 0.0f;
    currentAlgo = createAlgorithm(cat, idx);
    if (cat == Category::SORT)
        currentAlgo->setArraySize(arraySize);
    else if (cat == Category::SEARCH)
        currentAlgo->setArraySize(arraySize > 40 ? 20 : arraySize);
    currentAlgo->init();
    displayedHeights.clear();
}

VisualizerEngine::VisualizerEngine() {
    lastFrameTime = (float)glfwGetTime();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg]       = ImVec4(0.05f, 0.07f, 0.10f, 1.0f);
    style.Colors[ImGuiCol_FrameBg]        = ImVec4(0.10f, 0.13f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_Button]         = ImVec4(0.10f, 0.22f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered]  = ImVec4(0.15f, 0.40f, 0.65f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive]   = ImVec4(0.00f, 0.60f, 0.90f, 1.0f);
    style.Colors[ImGuiCol_Tab]            = ImVec4(0.08f, 0.10f, 0.16f, 1.0f);
    style.Colors[ImGuiCol_TabActive]      = ImVec4(0.00f, 0.45f, 0.70f, 1.0f);
    style.Colors[ImGuiCol_TabHovered]     = ImVec4(0.10f, 0.55f, 0.85f, 1.0f);
    style.Colors[ImGuiCol_Header]         = ImVec4(0.10f, 0.22f, 0.38f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered]  = ImVec4(0.00f, 0.50f, 0.80f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab]     = ImVec4(0.00f, 0.90f, 1.00f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.17f, 0.47f, 1.0f);
    style.Colors[ImGuiCol_CheckMark]      = ImVec4(0.00f, 0.90f, 1.00f, 1.0f);
    style.Colors[ImGuiCol_Separator]      = ImVec4(0.15f, 0.22f, 0.35f, 1.0f);
    style.FrameRounding  = 4.0f;
    style.GrabRounding   = 4.0f;
    style.WindowRounding = 0.0f;

    selectAlgorithm(Category::SORT, 0);
}

void VisualizerEngine::drawGlowRect(float x, float y, float w, float h, float r, float g, float b) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 4; i >= 1; i--) {
        float pad = i * 3.0f;
        float alpha = 0.08f * i;
        glColor4f(r, g, b, alpha);
        glBegin(GL_QUADS);
        glVertex2f(x - pad, y - pad);
        glVertex2f(x + w + pad, y - pad);
        glVertex2f(x + w + pad, y + h + pad);
        glVertex2f(x - pad, y + h + pad);
        glEnd();
    }
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void VisualizerEngine::drawGlowCircle(float cx, float cy, float radius, float r, float g, float b, int segments) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const float PI = 3.14159265f;
    for (int gi = 4; gi >= 1; gi--) {
        float gr = radius + gi * 4.0f;
        float alpha = 0.07f * gi;
        glColor4f(r, g, b, alpha);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int s = 0; s <= segments; s++) {
            float angle = s * 2.0f * PI / segments;
            glVertex2f(cx + cosf(angle) * gr, cy + sinf(angle) * gr);
        }
        glEnd();
    }
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int s = 0; s <= segments; s++) {
        float angle = s * 2.0f * PI / segments;
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    glEnd();
}

void VisualizerEngine::updatePlayback(float deltaTime) {
    if (!isPlaying || !currentAlgo || currentAlgo->isFinished()) {
        if (currentAlgo && currentAlgo->isFinished()) isPlaying = false;
        return;
    }
    stepTimer += deltaTime;
    float interval = stepInterval / speedMultiplier;
    if (interval < 0.02f) interval = 0.02f;
    while (stepTimer >= interval && !currentAlgo->isFinished()) {
        currentAlgo->step();
        stepTimer -= interval;
    }
}

void VisualizerEngine::renderPseudoCode(const std::vector<std::string>& lines, int activeLine) {
    for (int i = 0; i < (int)lines.size(); i++) {
        bool active = (i == activeLine);
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            ImVec2 cp = ImGui::GetCursorScreenPos();
            ImVec2 sz = ImGui::CalcTextSize(lines[i].c_str());
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(cp.x - 4, cp.y - 1),
                ImVec2(cp.x + ImGui::GetContentRegionAvail().x, cp.y + sz.y + 2),
                IM_COL32(0, 230, 255, 200), 3.0f);
            ImGui::Text("  %s", lines[i].c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.65f, 0.75f, 1.0f));
            ImGui::Text("  %s", lines[i].c_str());
            ImGui::PopStyleColor();
        }
    }
}

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

void VisualizerEngine::renderUI() {
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    elapsedTime = currentTime;

    updatePlayback(deltaTime);

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(760, (float)ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("##DSAPanel", nullptr, flags);
    ImGui::SetWindowFontScale(1.10f);
    const ImVec2 panelPos = ImGui::GetWindowPos();
    const ImVec2 panelSize = ImGui::GetWindowSize();
    const ImVec2 panelMax(panelPos.x + panelSize.x, panelPos.y + panelSize.y);
    RetroTheme::DrawNeonFrame(ImGui::GetWindowDrawList(), panelPos,
                              panelMax,
                              RetroTheme::NeonCyan(0.92f), elapsedTime, 18.0f, 1.5f);
    RetroTheme::DrawCornerAccents(ImGui::GetWindowDrawList(), panelPos,
                                  panelMax,
                                  RetroTheme::NeonAmber(0.88f), 24.0f, 2.7f);

    if (ImGui::IsKeyPressed(ImGuiKey_Space))       { isPlaying = !isPlaying; }
    if (ImGui::IsKeyPressed(ImGuiKey_R))           { currentAlgo->reset(); isPlaying = false; }
    if (ImGui::IsKeyPressed(ImGuiKey_S))           { currentAlgo->shuffle(); isPlaying = false; }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && !isPlaying)  { currentAlgo->step(); }
    if (ImGui::IsKeyPressed(ImGuiKey_1)) selectAlgorithm(Category::SORT, 0);
    if (ImGui::IsKeyPressed(ImGuiKey_2)) selectAlgorithm(Category::SORT, 1);
    if (ImGui::IsKeyPressed(ImGuiKey_3)) selectAlgorithm(Category::SORT, 2);
    if (ImGui::IsKeyPressed(ImGuiKey_4)) selectAlgorithm(Category::SORT, 3);
    if (ImGui::IsKeyPressed(ImGuiKey_5)) selectAlgorithm(Category::SORT, 4);
    if (ImGui::IsKeyPressed(ImGuiKey_6)) selectAlgorithm(Category::SORT, 5);

    float blink = (sinf(elapsedTime * 3.0f) + 1.0f) * 0.5f;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.22f, 1.0f));
    ImGui::TextUnformatted("DSA CORE");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextDisabled("sorting, search, graph");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.9f, 1.0f, 1.0f));
    ImGui::Text("  DSA VISUALIZER");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, blink));
    ImGui::TextUnformatted(" *");
    ImGui::PopStyleColor(2);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    float tabW = 220.0f;
    bool sortActive   = (currentCategory == Category::SORT);
    bool searchActive = (currentCategory == Category::SEARCH);
    bool graphActive  = (currentCategory == Category::GRAPH);

    auto catButton = [&](const char* label, Category cat, bool active) {
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.45f, 0.70f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.55f, 0.85f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.10f, 0.16f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.30f, 0.50f, 1.0f));
        }
        if (ImGui::Button(label, ImVec2(tabW, 50))) {
            selectAlgorithm(cat, 0);
        }
        ImGui::PopStyleColor(2);
    };

    catButton(" SORTING ", Category::SORT, sortActive);
    ImGui::SameLine();
    catButton(" SEARCHING ", Category::SEARCH, searchActive);
    ImGui::SameLine();
    catButton(" GRAPH ", Category::GRAPH, graphActive);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
    ImGui::TextUnformatted("  ALGORITHMS");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    const char** names = nullptr;
    int count = 0;
    switch (currentCategory) {
        case Category::SORT:   names = SORT_NAMES;   count = sortCount;   break;
        case Category::SEARCH: names = SEARCH_NAMES; count = searchCount; break;
        case Category::GRAPH:  names = GRAPH_NAMES;  count = graphCount;  break;
    }

    for (int i = 0; i < count; i++) {
        bool selected = (i == selectedAlgoInCategory);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.00f, 0.45f, 0.70f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.00f, 0.55f, 0.85f, 1.0f));
        }
        if (ImGui::Selectable(names[i], selected)) {
            selectAlgorithm(currentCategory, i);
        }
        if (selected) ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##MainTabs")) {
        if (ImGui::BeginTabItem("  Theory  ")) {
            activeTab = 0;
            renderTheoryTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("  Visualize  ")) {
            activeTab = 1;

            AlgoState state = currentAlgo->getCurrentState();

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::TextUnformatted("  CONTROLS");
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();

            float btnW = 126.0f;

            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.40f, 0.10f, 0.40f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 0.60f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.30f, 0.05f, 0.30f, 1.0f));
            if (ImGui::Button("Reset", ImVec2(btnW, 0))) {
                currentAlgo->reset();
                isPlaying = false;
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            const char* playLabel = isPlaying ? "Pause" : "Play";
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.55f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.75f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f, 0.45f, 0.10f, 1.0f));
            if (ImGui::Button(playLabel, ImVec2(btnW, 0))) {
                if (currentAlgo->isFinished()) {
                    currentAlgo->reset();
                }
                isPlaying = !isPlaying;
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.35f, 0.65f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.50f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f, 0.28f, 0.55f, 1.0f));
            if (ImGui::Button("Step", ImVec2(btnW, 0))) {
                if (!currentAlgo->isFinished()) {
                    currentAlgo->step();
                    isPlaying = false;
                }
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.65f, 0.28f, 0.08f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.38f, 0.10f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.55f, 0.20f, 0.05f, 1.0f));
            if (ImGui::Button("Shuffle", ImVec2(btnW, 0))) {
                currentAlgo->shuffle();
                isPlaying = false;
            }
            ImGui::PopStyleColor(3);

            ImGui::Spacing();

            ImGui::Text("Speed");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(270);
            ImGui::SliderFloat("##speed", &speedMultiplier, 0.25f, 5.0f, "%.1fx");

            if (currentCategory != Category::GRAPH) {
                ImGui::Text("Size ");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(270);
                int maxSize = (currentCategory == Category::SEARCH) ? 40 : 80;
                if (ImGui::SliderInt("##size", &arraySize, 10, maxSize)) {
                    currentAlgo->setArraySize(arraySize);
                    isPlaying = false;
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::TextUnformatted("  CUSTOM INPUT");
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();

            if (currentCategory == Category::SORT || currentCategory == Category::SEARCH) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.6f, 0.75f, 1.0f));
                ImGui::TextWrapped("  Enter values (comma-separated):");
                ImGui::PopStyleColor();
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##customArr", customArrayBuf, sizeof(customArrayBuf));

                if (currentCategory == Category::SEARCH) {
                    ImGui::Text("  Target:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(220);
                    ImGui::InputInt("##searchTarget", &customSearchTarget);
                    if (customSearchTarget < 1) customSearchTarget = 1;
                }

                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.45f, 0.35f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.65f, 0.50f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.05f, 0.35f, 0.25f, 1.0f));
                if (ImGui::Button("  Apply Custom Array  ", ImVec2(-1, 0))) {
                    std::vector<int> parsed = parseCSV(customArrayBuf);
                    if (!parsed.empty()) {
                        isPlaying = false;
                        displayedHeights.clear();
                        if (currentCategory == Category::SEARCH) {
                            currentAlgo->setCustomArray(parsed);
                            currentAlgo->setTargetValue(customSearchTarget);
                        } else {
                            currentAlgo->setCustomArray(parsed);
                        }
                        arraySize = currentAlgo->getArraySize();
                    }
                }
                ImGui::PopStyleColor(3);

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.42f, 0.52f, 1.0f));
                ImGui::TextWrapped("  e.g. 15, 3, 42, 7, 28, 9, 1");
                ImGui::PopStyleColor();
            }

            if (currentCategory == Category::GRAPH) {
                ImGui::Text("  Start Node:");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(220);
                if (ImGui::InputInt("##startNode", &graphStartNode)) {
                    if (graphStartNode < 0) graphStartNode = 0;
                    if (graphStartNode > 9) graphStartNode = 9;
                }
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.45f, 0.35f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.65f, 0.50f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.05f, 0.35f, 0.25f, 1.0f));
                if (ImGui::Button("  Apply Start Node  ", ImVec2(-1, 0))) {
                    isPlaying = false;
                    currentAlgo->setStartNode(graphStartNode);
                }
                ImGui::PopStyleColor(3);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.42f, 0.52f, 1.0f));
                ImGui::TextWrapped("  Choose start node (0-9)");
                ImGui::PopStyleColor();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            state = currentAlgo->getCurrentState();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::TextUnformatted("  STATS");
            ImGui::PopStyleColor();
            ImGui::Separator();

            float progress = (state.totalSteps > 0)
                ? (float)state.currentStep / (float)state.totalSteps : 0.0f;
            char lbl[64];
            snprintf(lbl, sizeof(lbl), "Step %d / %d", state.currentStep, state.totalSteps);
            ImGui::ProgressBar(progress, ImVec2(-1, 0), lbl);

            ImGui::Text("  Comparisons:  %d", state.comparisons);
            ImGui::Text("  Swaps/Moves:  %d", state.swaps);

            if (currentAlgo->isFinished()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, 1.0f));
                ImGui::TextUnformatted("  *** Complete! ***");
                ImGui::PopStyleColor();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::TextUnformatted("  PSEUDOCODE");
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();

            renderPseudoCode(currentAlgo->getPseudoCode(), state.pseudoCodeLine);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::TextUnformatted("  COMPLEXITY");
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.85f, 1.0f, 1.0f));
            ImGui::TextWrapped("  %s", currentAlgo->getComplexity().c_str());
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::TextUnformatted("  STEP LOG");
            ImGui::PopStyleColor();
            ImGui::Separator();

            ImGui::BeginChild("##StepLog", ImVec2(0, 220), true);
            const ImVec2 stepLogPos = ImGui::GetWindowPos();
            const ImVec2 stepLogSize = ImGui::GetWindowSize();
            const ImVec2 stepLogMax(stepLogPos.x + stepLogSize.x, stepLogPos.y + stepLogSize.y);
            RetroTheme::DrawNeonFrame(ImGui::GetWindowDrawList(), stepLogPos,
                                      stepLogMax,
                                      RetroTheme::NeonPink(0.24f), elapsedTime + 1.0f,
                                      14.0f, 1.0f);
            for (const auto& cl : state.calcLines) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.75f, 0.85f, 1.0f));
                ImGui::TextWrapped("  %s", cl.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::EndChild();

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void VisualizerEngine::renderGrid(int windowWidth, int windowHeight) {
    const int panelW = 760;
    int canvasW = windowWidth - panelW;
    int canvasH = windowHeight;
    if (canvasW <= 0 || canvasH <= 0 || activeTab != 1) return;

    glViewport(panelW, 0, canvasW, canvasH);

    VisMode mode = currentAlgo->getVisMode();
    switch (mode) {
        case VisMode::SORT_BARS:
            renderSortCanvas(panelW, 0, canvasW, canvasH);
            break;
        case VisMode::SEARCH_ARRAY:
            renderSearchCanvas(panelW, 0, canvasW, canvasH);
            break;
        case VisMode::GRAPH:
            renderGraphCanvas(panelW, 0, canvasW, canvasH);
            break;
    }

    glViewport(0, 0, windowWidth, windowHeight);
}

void VisualizerEngine::renderSortCanvas(int x, int y, int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.08f, 0.10f, 0.15f);
    glBegin(GL_LINES);
    for (int gy = 80; gy < h; gy += 80) {
        glVertex2f(0, (float)gy);
        glVertex2f((float)w, (float)gy);
    }
    glEnd();

    std::vector<Cell> cells = currentAlgo->getCells();
    int n = (int)cells.size();
    if (n == 0) return;

    int maxVal = 1;
    for (auto& c : cells) if (c.value > maxVal) maxVal = c.value;

    float barArea = (float)w;
    float gap = 2.0f;
    float barW = (barArea - gap * (n + 1)) / n;
    if (barW < 2.0f) { barW = barArea / n; gap = 0; }

    float maxBarH = h * 0.85f;
    float bottomPad = 30.0f;

    if ((int)displayedHeights.size() != n) {
        displayedHeights.resize(n);
        for (int i = 0; i < n; i++)
            displayedHeights[i] = (float)cells[i].value / maxVal * maxBarH;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    for (int i = 0; i < n; i++) {
        float target = (float)cells[i].value / maxVal * maxBarH;
        displayedHeights[i] += (target - displayedHeights[i]) * 0.15f;

        float bx = gap + i * (barW + gap);
        float by = bottomPad;
        float bh = displayedHeights[i];

        float cr = cells[i].r, cg = cells[i].g, cb = cells[i].b;

        if (cells[i].highlighted) {
            drawGlowRect(bx, by, barW, bh, cr, cg, cb);
        } else {
            bool isSorted = (cr < 0.1f && cg > 0.9f && cb > 0.4f);
            if (isSorted) {
                glColor3f(cr, cg, cb);
                glBegin(GL_QUADS);
                glVertex2f(bx, by); glVertex2f(bx + barW, by);
                glVertex2f(bx + barW, by + bh); glVertex2f(bx, by + bh);
                glEnd();
            } else {
                glBegin(GL_QUADS);
                glColor3f(0.5f, 0.1f, 0.8f);
                glVertex2f(bx, by); glVertex2f(bx + barW, by);
                glColor3f(0.0f, 0.9f, 1.0f);
                glVertex2f(bx + barW, by + bh); glVertex2f(bx, by + bh);
                glEnd();
            }
        }

        if (n < 35) {
            char valStr[16];
            snprintf(valStr, sizeof(valStr), "%d", cells[i].value);
            float textX = (float)x + bx + barW * 0.5f - 5.0f;
            float textY = (float)(h) - by - bh - 16.0f;
            drawList->AddText(ImVec2(textX, textY),
                IM_COL32(255, 255, 255, 220), valStr);
        }
    }

    glDisable(GL_BLEND);
}

void VisualizerEngine::renderSearchCanvas(int x, int y, int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    std::vector<Cell> cells = currentAlgo->getCells();
    int n = (int)cells.size();
    if (n == 0) return;

    float tileSize = (n > 20) ? 36.0f : 52.0f;
    float tileGap = 5.0f;
    float totalW = n * (tileSize + tileGap) - tileGap;
    float startX = (w - totalW) * 0.5f;
    float startY = h * 0.5f - tileSize * 0.5f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    char searchLabel[64];
    snprintf(searchLabel, sizeof(searchLabel), "SEARCHING FOR: %d", currentAlgo->getTargetValue());
    float labelX = (float)x + w * 0.5f - 80.0f;
    float labelY = (float)(h) - startY - tileSize - 60.0f;
    drawList->AddText(nullptr, 22.0f, ImVec2(labelX, labelY),
        IM_COL32(0, 230, 255, 255), searchLabel);

    AlgoState state = currentAlgo->getCurrentState();

    int leftIdx = -1, rightIdx = -1, midIdx = -1;
    if (!state.calcLines.empty()) {
        std::string desc = state.calcLines[0];
        size_t lp = desc.find("Left=");
        size_t mp = desc.find("Mid=");
        size_t rp = desc.find("Right=");
        if (lp != std::string::npos) leftIdx = std::stoi(desc.substr(lp + 5));
        if (mp != std::string::npos) midIdx = std::stoi(desc.substr(mp + 4));
        if (rp != std::string::npos) rightIdx = std::stoi(desc.substr(rp + 6));
    }

    for (int i = 0; i < n; i++) {
        float tx = startX + i * (tileSize + tileGap);
        float ty = startY;
        float cr = cells[i].r, cg = cells[i].g, cb = cells[i].b;

        bool isFound = (cr < 0.1f && cg > 0.9f && cb > 0.4f);
        bool isCurrent = cells[i].highlighted && !isFound;

        bool isEliminated = (cr < 0.2f && cg < 0.2f && cb < 0.25f);

        if (isFound) {
            float pulse = (sinf(elapsedTime * 4.0f) + 1.0f) * 0.5f;
            float glow = 0.7f + pulse * 0.3f;
            drawGlowRect(tx, ty, tileSize, tileSize, 0.0f, glow, 0.4f);
        } else if (isCurrent) {
            drawGlowRect(tx, ty, tileSize, tileSize, 1.0f, 0.85f, 0.0f);
        } else if (isEliminated) {
            glColor4f(0.08f, 0.08f, 0.12f, 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(tx, ty); glVertex2f(tx + tileSize, ty);
            glVertex2f(tx + tileSize, ty + tileSize); glVertex2f(tx, ty + tileSize);
            glEnd();
        } else {
            glColor3f(cr, cg, cb);
            glBegin(GL_QUADS);
            glVertex2f(tx, ty); glVertex2f(tx + tileSize, ty);
            glVertex2f(tx + tileSize, ty + tileSize); glVertex2f(tx, ty + tileSize);
            glEnd();
            glColor3f(0.20f, 0.30f, 0.45f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(tx, ty); glVertex2f(tx + tileSize, ty);
            glVertex2f(tx + tileSize, ty + tileSize); glVertex2f(tx, ty + tileSize);
            glEnd();
        }

        char valStr[16];
        snprintf(valStr, sizeof(valStr), "%d", cells[i].value);
        float textX = (float)x + tx + tileSize * 0.5f - 8.0f;
        float textY = (float)(h) - ty - tileSize * 0.5f - 6.0f;
        ImU32 textCol = isEliminated ? IM_COL32(100, 100, 120, 150) : IM_COL32(255, 255, 255, 240);
        drawList->AddText(ImVec2(textX, textY), textCol, valStr);

        float lblY = (float)(h) - ty + 6.0f;
        float lblX = (float)x + tx + tileSize * 0.5f - 4.0f;
        if (i == leftIdx) {
            drawList->AddText(ImVec2(lblX, lblY), IM_COL32(0, 200, 200, 255), "L");
        }
        if (i == midIdx) {
            drawList->AddText(ImVec2(lblX, lblY), IM_COL32(255, 220, 0, 255), "M");
        }
        if (i == rightIdx) {
            drawList->AddText(ImVec2(lblX + 1, lblY), IM_COL32(0, 200, 200, 255), "R");
        }
    }

    glDisable(GL_BLEND);
}

void VisualizerEngine::renderGraphCanvas(int x, int y, int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    std::vector<Cell> cells = currentAlgo->getCells();
    std::vector<Edge> edges = currentAlgo->getEdges();
    if (cells.empty()) return;

    GraphAlgoBase* graphAlgo = dynamic_cast<GraphAlgoBase*>(currentAlgo.get());
    if (!graphAlgo) return;
    const auto& positions = graphAlgo->getNodePositions();

    float pad = 60.0f;
    float nodeRadius = 26.0f;

    auto mapX = [&](float nx) { return pad + nx * (w - 2 * pad); };
    auto mapY = [&](float ny) { return pad + ny * (h - 2 * pad); };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& edge : edges) {
        float x1 = mapX(positions[edge.from].first);
        float y1 = mapY(positions[edge.from].second);
        float x2 = mapX(positions[edge.to].first);
        float y2 = mapY(positions[edge.to].second);

        if (edge.traversed) {
            for (int g = 3; g >= 1; g--) {
                glColor4f(0.0f, 0.9f, 1.0f, 0.08f * g);
                glLineWidth((float)(3 + g * 2));
                glBegin(GL_LINES);
                glVertex2f(x1, y1); glVertex2f(x2, y2);
                glEnd();
            }
            glColor3f(0.0f, 0.9f, 1.0f);
            glLineWidth(3.0f);
        } else {
            glColor3f(0.25f, 0.28f, 0.35f);
            glLineWidth(2.0f);
        }
        glBegin(GL_LINES);
        glVertex2f(x1, y1); glVertex2f(x2, y2);
        glEnd();
    }
    glLineWidth(1.0f);

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    for (int i = 0; i < (int)cells.size(); i++) {
        if (i >= (int)positions.size()) break;
        float cx = mapX(positions[i].first);
        float cy = mapY(positions[i].second);

        float cr = cells[i].r, cg = cells[i].g, cb = cells[i].b;
        bool isCurrent = cells[i].highlighted;

        if (isCurrent) {
            float pulse = (sinf(elapsedTime * 4.0f) + 1.0f) * 0.5f;
            float r = nodeRadius + pulse * 3.0f;
            drawGlowCircle(cx, cy, r, cr, cg, cb);
        } else {
            bool isVisited = (cr > 0.3f && cb > 0.6f);
            if (isVisited) {
                drawGlowCircle(cx, cy, nodeRadius, cr, cg, cb);
            } else {
                drawGlowCircle(cx, cy, nodeRadius, cr, cg, cb);
                glColor3f(0.5f, 0.1f, 0.9f);
                glBegin(GL_LINE_LOOP);
                for (int s = 0; s < 32; s++) {
                    float angle = s * 2.0f * 3.14159265f / 32;
                    glVertex2f(cx + cosf(angle) * nodeRadius, cy + sinf(angle) * nodeRadius);
                }
                glEnd();
            }
        }

        char nodeStr[8];
        snprintf(nodeStr, sizeof(nodeStr), "%d", i);
        float textX = (float)x + cx - 4.0f;
        float textY = (float)(h) - cy - 6.0f;
        drawList->AddText(ImVec2(textX, textY),
            IM_COL32(255, 255, 255, 240), nodeStr);
    }

    AlgoState state = currentAlgo->getCurrentState();
    if (state.calcLines.size() > 1) {
        float chipX = 20.0f;
        float chipY = 30.0f;
        drawList->AddText(ImVec2((float)x + chipX, (float)(h) - chipY),
            IM_COL32(180, 180, 200, 200), state.calcLines[1].c_str());
    }
    if (state.calcLines.size() > 2) {
        float chipX = 20.0f;
        float chipY = 50.0f;
        drawList->AddText(ImVec2((float)x + chipX, (float)(h) - chipY),
            IM_COL32(150, 150, 180, 180), state.calcLines[2].c_str());
    }

    glDisable(GL_BLEND);
}
