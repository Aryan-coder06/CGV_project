#include "../../include/Visualizer/VisualizerEngine.h"
#include "../../include/UI/RetroTheme.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

static const char *ALGO_NAMES[ALGO_COUNT] = {
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
    "Scanline Polygon Fill",
    "Cohen-Sutherland Line Clipping",
    "Sutherland-Hodgman Polygon Clipping",
    "2D Transformations (Translate, Rotate, Scale)",
    "Window-to-Viewport Transformation"};

// ================================================================
//  Factory: create algorithm by index
// ================================================================
std::unique_ptr<IAlgorithm> VisualizerEngine::createAlgorithm(int idx) {
  switch (idx) {
  case ALGO_BRESENHAM:
    return std::make_unique<BresenhamLine>();
  case ALGO_XIAOLIN_WU:
    return std::make_unique<XiaolinWuLine>();
  case ALGO_MIDPOINT_CIRCLE:
    return std::make_unique<MidpointCircle>();
  case ALGO_BRESENHAM_CIRCLE:
    return std::make_unique<BresenhamCircle>();
  case ALGO_MIDPOINT_ELLIPSE:
    return std::make_unique<MidpointEllipse>();
  case ALGO_FLOOD_FILL_4:
    return std::make_unique<FloodFill4>();
  case ALGO_FLOOD_FILL_8:
    return std::make_unique<FloodFill8>();
  case ALGO_BOUNDARY_FILL_4:
    return std::make_unique<BoundaryFill4>();
  case ALGO_BOUNDARY_FILL_8:
    return std::make_unique<BoundaryFill8>();
  case ALGO_SCANLINE_FILL:
    return std::make_unique<ScanlineFill>();
  case ALGO_COHEN_SUTHERLAND:
    return std::make_unique<CohenSutherland>();
  case ALGO_SUTHERLAND_HODGMAN:
    return std::make_unique<SutherlandHodgman>();
  case ALGO_TRANSFORM_2D:
    return std::make_unique<Transform2D>();
  case ALGO_WINDOW_VIEWPORT:
    return std::make_unique<WindowViewport>();
  case ALGO_DDA:
  default:
    return std::make_unique<DDALine>();
  }
}

VisualizerEngine::VisualizerEngine() {
  currentAlgo = createAlgorithm(ALGO_DDA);
  currentAlgo->init(inputX1, inputY1, inputX2, inputY2);
}

void VisualizerEngine::renderUI() {
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(760, ImGui::GetIO().DisplaySize.y),
                           ImGuiCond_Always);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoCollapse;

  ImGui::Begin("##VisualizerPanel", nullptr, flags);
  ImGui::SetWindowFontScale(1.10f);
  const ImVec2 panelPos = ImGui::GetWindowPos();
  const ImVec2 panelSize = ImGui::GetWindowSize();
  const ImVec2 panelMax(panelPos.x + panelSize.x, panelPos.y + panelSize.y);
  RetroTheme::DrawNeonFrame(ImGui::GetWindowDrawList(), panelPos,
                            panelMax,
                            RetroTheme::NeonCyan(0.92f), (float)glfwGetTime(),
                            18.0f, 1.5f);
  RetroTheme::DrawCornerAccents(ImGui::GetWindowDrawList(), panelPos,
                                panelMax,
                                RetroTheme::NeonAmber(0.88f), 24.0f, 2.7f);

  // ---- Header ----
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.22f, 1.0f));
  ImGui::TextUnformatted("CGV CORE");
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::TextDisabled("raster, fill, clipping, transform");
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 1.0f, 1.0f));
  ImGui::TextUnformatted("SECTION 2 — ALGORITHM VISUALIZER");
  ImGui::PopStyleColor();

  ImGui::SameLine(ImGui::GetWindowWidth() - 220);
  ImGui::SetNextItemWidth(120);
  ImGui::SliderFloat("Font Scale", &ImGui::GetIO().FontGlobalScale, 0.5f, 3.0f, "%.1f");

  ImGui::Spacing();

  // ---- Algorithm dropdown ----
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
  ImGui::TextUnformatted("SELECT ALGORITHM");
  ImGui::PopStyleColor();

  ImGui::SetNextItemWidth(-1);
  if (ImGui::Combo("##AlgoCombo", &selectedAlgo, ALGO_NAMES, ALGO_COUNT)) {
    currentAlgo = createAlgorithm(selectedAlgo);
    if (currentAlgo->isWindowViewportMode()) {
      auto* wv = dynamic_cast<WindowViewport*>(currentAlgo.get());
      if (wv) {
        wv->setWorldWindow(inputWxMin, inputWxMax, inputWyMin, inputWyMax);
        wv->setViewport(inputVxMin, inputVxMax, inputVyMin, inputVyMax);
      }
      currentAlgo->init(inputWVCX, inputWVCY, inputWVSize, inputWVSides);
    } else if (currentAlgo->isTransformMode()) {
      auto* tf = dynamic_cast<Transform2D*>(currentAlgo.get());
      if (tf) {
        tf->setTranslation(inputTfTx, inputTfTy);
        tf->setRotation(inputTfAngle, (float)inputTfCX, (float)inputTfCY);
        tf->setScaling(inputTfSx, inputTfSy, (float)inputTfCX, (float)inputTfCY);
      }
      currentAlgo->init(inputTfCX, inputTfCY, inputTfSize, inputTfSides);
    } else if (currentAlgo->isPolygonClipMode()) {
      currentAlgo->init(inputSHCX, inputSHCY, inputSHSize, inputSHSides);
    } else if (currentAlgo->isClipMode())
      currentAlgo->init(inputX1, inputY1, inputX2, inputY2);
    else if (currentAlgo->isEllipseMode())
      currentAlgo->init(inputCX, inputCY, inputRX, inputRY);
    else if (currentAlgo->isFillMode())
      currentAlgo->init(inputSeedX, inputSeedY, inputHalfS, 0);
    else if (currentAlgo->isScanlineMode())
      currentAlgo->init(inputPolyCX, inputPolyCY, inputPolySize,
                        inputPolySides);
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
    bool isHeading = !line.empty() && !isSeparator &&
                     (line.back() == ':' || line.back() == '?');
    bool isPlus = !line.empty() && line[0] == '+';
    bool isMinus =
        !line.empty() && line[0] == '-' && line.size() > 1 && line[1] != '-';

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

void VisualizerEngine::renderVisualizeTab() {
  AlgoState state = currentAlgo->getCurrentState();

  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.11f, 0.16f, 1.0f));
  ImGui::BeginChild("##UpperBox", ImVec2(0, 430), true,
                    ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::PopStyleColor();
  const ImVec2 upperPos = ImGui::GetWindowPos();
  const ImVec2 upperSize = ImGui::GetWindowSize();
  const ImVec2 upperMax(upperPos.x + upperSize.x, upperPos.y + upperSize.y);
  RetroTheme::DrawNeonFrame(ImGui::GetWindowDrawList(), upperPos,
                            upperMax,
                            RetroTheme::NeonAmber(0.42f), (float)glfwGetTime() + 0.8f,
                            14.0f, 1.0f);

  // ---- Inputs ----
  bool isCircle     = currentAlgo->isCircleMode();
  bool isEllipse    = currentAlgo->isEllipseMode();
  bool isFill       = currentAlgo->isFillMode();
  bool isScanline   = currentAlgo->isScanlineMode();
  bool isClip       = currentAlgo->isClipMode();
  bool isPolyClip   = currentAlgo->isPolygonClipMode();
  bool isTransform  = currentAlgo->isTransformMode();
  bool isWinViewport= currentAlgo->isWindowViewportMode();

  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
  ImGui::TextUnformatted(isWinViewport? "INPUT  (World Window + Viewport + Polygon)"
                         : isTransform ? "INPUT  (Polygon + Transform Params)"
                         : isPolyClip  ? "INPUT  (Polygon + Clip Window)"
                         : isClip      ? "INPUT  (Line P1/P2 + Clip Window)"
                         : isScanline  ? "INPUT  (Polygon: Centre + Size + Sides)"
                         : isFill      ? "INPUT  (Seed + Half-side S)"
                         : isEllipse   ? "INPUT  (Centre + Semi-axes)"
                         : isCircle    ? "INPUT  (Centre + Radius)"
                                       : "INPUT COORDINATES");
  ImGui::PopStyleColor();
  ImGui::Separator();

  if (isWinViewport) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.3f, 0.9f));
    ImGui::TextUnformatted("  World Window:");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180); ImGui::InputFloat("Wx min##wv", &inputWxMin, 1.f, 5.f, "%.1f");
    ImGui::SetNextItemWidth(180); ImGui::InputFloat("Wx max##wv", &inputWxMax, 1.f, 5.f, "%.1f");
    ImGui::SetNextItemWidth(180); ImGui::InputFloat("Wy min##wv", &inputWyMin, 1.f, 5.f, "%.1f");
    ImGui::SetNextItemWidth(180); ImGui::InputFloat("Wy max##wv", &inputWyMax, 1.f, 5.f, "%.1f");
    if (inputWxMax <= inputWxMin) inputWxMax = inputWxMin + 1.f;
    if (inputWyMax <= inputWyMin) inputWyMax = inputWyMin + 1.f;
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.6f, 0.9f));
    ImGui::TextUnformatted("  Viewport (screen coords):");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180); ImGui::InputFloat("Vx min##wv", &inputVxMin, 1.f, 5.f, "%.1f");
    ImGui::SetNextItemWidth(180); ImGui::InputFloat("Vx max##wv", &inputVxMax, 1.f, 5.f, "%.1f");
    ImGui::SetNextItemWidth(180); ImGui::InputFloat("Vy min##wv", &inputVyMin, 1.f, 5.f, "%.1f");
    ImGui::SetNextItemWidth(180); ImGui::InputFloat("Vy max##wv", &inputVyMax, 1.f, 5.f, "%.1f");
    if (inputVxMax <= inputVxMin) inputVxMax = inputVxMin + 1.f;
    if (inputVyMax <= inputVyMin) inputVyMax = inputVyMin + 1.f;
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
    ImGui::TextUnformatted("  Polygon (in world coords):");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180); ImGui::InputInt("CX##wv",    &inputWVCX);
    ImGui::SetNextItemWidth(180); ImGui::InputInt("CY##wv",    &inputWVCY);
    ImGui::SetNextItemWidth(180); ImGui::InputInt("Size##wv",  &inputWVSize);
    ImGui::SetNextItemWidth(180); ImGui::InputInt("Sides##wv", &inputWVSides);
    if (inputWVSize  < 1)  inputWVSize  = 1;
    if (inputWVSides < 3)  inputWVSides = 3;
    if (inputWVSides > 12) inputWVSides = 12;
  } else if (isTransform) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
    ImGui::TextUnformatted("  Polygon:");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("CX##tf",    &inputTfCX);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("CY##tf",    &inputTfCY);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Size##tf",  &inputTfSize);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Sides##tf", &inputTfSides);
    if (inputTfSize  < 2)  inputTfSize  = 2;
    if (inputTfSides < 3)  inputTfSides = 3;
    if (inputTfSides > 12) inputTfSides = 12;
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.3f, 0.9f));
    ImGui::TextUnformatted("  Translation:");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180);
    ImGui::InputFloat("tx##tf", &inputTfTx, 0.5f, 1.0f, "%.1f");
    ImGui::SetNextItemWidth(180);
    ImGui::InputFloat("ty##tf", &inputTfTy, 0.5f, 1.0f, "%.1f");
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.3f, 0.9f));
    ImGui::TextUnformatted("  Rotation (deg, about centroid):");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180);
    ImGui::InputFloat("Angle##tf", &inputTfAngle, 5.0f, 15.0f, "%.1f");
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.3f, 0.9f));
    ImGui::TextUnformatted("  Scaling (about centroid):");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180);
    ImGui::InputFloat("sx##tf", &inputTfSx, 0.1f, 0.5f, "%.2f");
    ImGui::SetNextItemWidth(180);
    ImGui::InputFloat("sy##tf", &inputTfSy, 0.1f, 0.5f, "%.2f");
    if (inputTfSx < 0.01f) inputTfSx = 0.01f;
    if (inputTfSy < 0.01f) inputTfSy = 0.01f;
  } else if (isPolyClip) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
    ImGui::TextUnformatted("  Polygon:");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("CX##sh",    &inputSHCX);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("CY##sh",    &inputSHCY);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Size##sh",  &inputSHSize);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Sides##sh", &inputSHSides);
    if (inputSHSize  < 2)  inputSHSize  = 2;
    if (inputSHSides < 3)  inputSHSides = 3;
    if (inputSHSides > 12) inputSHSides = 12;
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
    ImGui::TextUnformatted("  Clip Window:");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("xMin##sh", &inputClipXMin);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("xMax##sh", &inputClipXMax);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("yMin##sh", &inputClipYMin);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("yMax##sh", &inputClipYMax);
    if (inputClipXMax <= inputClipXMin) inputClipXMax = inputClipXMin + 1;
    if (inputClipYMax <= inputClipYMin) inputClipYMax = inputClipYMin + 1;
  } else if (isClip) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
    ImGui::TextUnformatted("  Line Endpoints:");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("P1 X##clip", &inputX1);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("P1 Y##clip", &inputY1);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("P2 X##clip", &inputX2);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("P2 Y##clip", &inputY2);
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
    ImGui::TextUnformatted("  Clip Window:");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("xMin##cw", &inputClipXMin);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("xMax##cw", &inputClipXMax);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("yMin##cw", &inputClipYMin);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("yMax##cw", &inputClipYMax);
    if (inputClipXMax <= inputClipXMin) inputClipXMax = inputClipXMin + 1;
    if (inputClipYMax <= inputClipYMin) inputClipYMax = inputClipYMin + 1;
  } else if (isScanline) {
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Centre X", &inputPolyCX);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Centre Y", &inputPolyCY);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Poly Size", &inputPolySize);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Sides (3-10)", &inputPolySides);
    if (inputPolySize < 2)
      inputPolySize = 2;
    if (inputPolySides < 3)
      inputPolySides = 3;
    if (inputPolySides > 10)
      inputPolySides = 10;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
    ImGui::Text("  Regular %d-gon, radius %d", inputPolySides, inputPolySize);
    ImGui::PopStyleColor();
  } else if (isEllipse) {
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Centre X", &inputCX);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Centre Y", &inputCY);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Semi-a (X)", &inputRX);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Semi-b (Y)", &inputRY);
    if (inputRX < 1)
      inputRX = 1;
    if (inputRY < 1)
      inputRY = 1;
  } else if (isFill) {
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Seed X (cx)", &inputSeedX);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Seed Y (cy)", &inputSeedY);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Half-side S", &inputHalfS);
    if (inputHalfS < 2)
      inputHalfS = 2;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 0.85f));
    ImGui::Text("  Boundary: [%d..%d] x [%d..%d]", inputSeedX - inputHalfS,
                inputSeedX + inputHalfS, inputSeedY - inputHalfS,
                inputSeedY + inputHalfS);
    ImGui::PopStyleColor();
  } else if (isCircle) {
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Centre X", &inputCX);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Centre Y", &inputCY);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Radius", &inputRadius);
    if (inputRadius < 1)
      inputRadius = 1;
  } else {
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Start X", &inputX1);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("Start Y", &inputY1);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("End X", &inputX2);
    ImGui::SetNextItemWidth(180);
    ImGui::InputInt("End Y", &inputY2);
  }

  ImGui::Spacing();
  if (ImGui::Button("  Apply & Reset  ", ImVec2(-1, 0))) {
    if (isWinViewport) {
      currentAlgo = createAlgorithm(ALGO_WINDOW_VIEWPORT);
      auto* wv = dynamic_cast<WindowViewport*>(currentAlgo.get());
      if (wv) {
        wv->setWorldWindow(inputWxMin, inputWxMax, inputWyMin, inputWyMax);
        wv->setViewport(inputVxMin, inputVxMax, inputVyMin, inputVyMax);
      }
      currentAlgo->init(inputWVCX, inputWVCY, inputWVSize, inputWVSides);
    } else if (isTransform) {
      currentAlgo = createAlgorithm(ALGO_TRANSFORM_2D);
      auto* tf = dynamic_cast<Transform2D*>(currentAlgo.get());
      if (tf) {
        tf->setTranslation(inputTfTx, inputTfTy);
        tf->setRotation(inputTfAngle, (float)inputTfCX, (float)inputTfCY);
        tf->setScaling(inputTfSx, inputTfSy, (float)inputTfCX, (float)inputTfCY);
      }
      currentAlgo->init(inputTfCX, inputTfCY, inputTfSize, inputTfSides);
    } else if (isPolyClip) {
      currentAlgo = createAlgorithm(ALGO_SUTHERLAND_HODGMAN);
      auto* sh = dynamic_cast<SutherlandHodgman*>(currentAlgo.get());
      if (sh) sh->setClipWindow(inputClipXMin, inputClipXMax, inputClipYMin, inputClipYMax);
      currentAlgo->init(inputSHCX, inputSHCY, inputSHSize, inputSHSides);
    } else if (isClip) {
      currentAlgo = createAlgorithm(ALGO_COHEN_SUTHERLAND);
      auto* cs = dynamic_cast<CohenSutherland*>(currentAlgo.get());
      if (cs) cs->setClipWindow(inputClipXMin, inputClipXMax, inputClipYMin, inputClipYMax);
      currentAlgo->init(inputX1, inputY1, inputX2, inputY2);
    } else if (isScanline)
      currentAlgo->init(inputPolyCX, inputPolyCY, inputPolySize, inputPolySides);
    else if (isEllipse)
      currentAlgo->init(inputCX, inputCY, inputRX, inputRY);
    else if (isFill)
      currentAlgo->init(inputSeedX, inputSeedY, inputHalfS, 0);
    else if (isCircle)
      currentAlgo->init(inputCX, inputCY, inputRadius, 0);
    else
      currentAlgo->init(inputX1, inputY1, inputX2, inputY2);
    lastMode = ExecMode::NONE;
  }

  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
  ImGui::TextUnformatted("INITIALIZATION");
  ImGui::PopStyleColor();
  ImGui::Separator();
  for (const auto &s : currentAlgo->getInitInfo())
    ImGui::Text("  %s", s.c_str());

  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
  ImGui::TextUnformatted("EXECUTION");
  ImGui::PopStyleColor();
  ImGui::Separator();

  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.70f, 1.0f));
  ImGui::TextWrapped("Step +1: full calc  |  Step K / Run All: silent");
  ImGui::PopStyleColor();
  ImGui::Spacing();

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.15f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0.25f, 0.75f, 0.25f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0.10f, 0.45f, 0.10f, 1.0f));
  if (ImGui::Button("  Step +1  (shows calculation)  ", ImVec2(-1, 0))) {
    if (!currentAlgo->isFinished()) {
      currentAlgo->step();
      lastMode = ExecMode::STEP_ONE;
    }
  }
  ImGui::PopStyleColor(3);

  ImGui::Spacing();
  ImGui::SetNextItemWidth(130);
  ImGui::InputInt("K##steps", &kSteps);
  if (kSteps < 1)
    kSteps = 1;
  ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.35f, 0.65f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0.25f, 0.50f, 0.85f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0.10f, 0.28f, 0.55f, 1.0f));
  if (ImGui::Button("Step K  (silent)", ImVec2(-1, 0))) {
    currentAlgo->stepK(kSteps);
    lastMode = ExecMode::STEP_K;
  }
  ImGui::PopStyleColor(3);

  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.28f, 0.08f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0.85f, 0.38f, 0.10f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0.55f, 0.20f, 0.05f, 1.0f));
  if (ImGui::Button("  Run to Completion  (silent)  ", ImVec2(-1, 0))) {
    currentAlgo->runToCompletion();
    lastMode = ExecMode::RUN_ALL;
  }
  ImGui::PopStyleColor(3);

  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.40f, 0.10f, 0.40f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0.60f, 0.20f, 0.60f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0.30f, 0.05f, 0.30f, 1.0f));
  if (ImGui::Button("  Reset  ", ImVec2(-1, 0))) {
    currentAlgo->reset();
    lastMode = ExecMode::NONE;
  }
  ImGui::PopStyleColor(3);

  ImGui::EndChild(); // End of BOX 1

  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
  ImGui::TextUnformatted("  CURRENT STATE");
  ImGui::PopStyleColor();

  state = currentAlgo->getCurrentState(); // refresh

  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.10f, 0.15f, 1.0f));
  ImGui::BeginChild("##StateBox", ImVec2(0, 260), true);
  ImGui::PopStyleColor();
  const ImVec2 statePos = ImGui::GetWindowPos();
  const ImVec2 stateSize = ImGui::GetWindowSize();
  const ImVec2 stateMax(statePos.x + stateSize.x, statePos.y + stateSize.y);
  RetroTheme::DrawNeonFrame(ImGui::GetWindowDrawList(), statePos,
                            stateMax,
                            RetroTheme::NeonCyan(0.36f), (float)glfwGetTime() + 1.2f,
                            14.0f, 1.0f);

  float progress = (state.totalSteps > 0)
                       ? (float)state.currentStep / (float)state.totalSteps
                       : 1.0f;
  char lbl[64];
  snprintf(lbl, sizeof(lbl), "Step %d / %d", state.currentStep,
           state.totalSteps);
  ImGui::ProgressBar(progress, ImVec2(-1, 0), lbl);
  ImGui::Spacing();

  for (const auto &v : currentAlgo->getCurrentVars())
    ImGui::Text("  %s", v.c_str());

  if (currentAlgo->isFinished()) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
    ImGui::TextUnformatted("  *** Complete! ***");
    ImGui::PopStyleColor();
  }

  ImGui::EndChild(); // End of BOX 2

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
  ImGui::BeginChild("##CalcBox", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
  ImGui::PopStyleColor();
  const ImVec2 calcPos = ImGui::GetWindowPos();
  const ImVec2 calcSize = ImGui::GetWindowSize();
  const ImVec2 calcMax(calcPos.x + calcSize.x, calcPos.y + calcSize.y);
  RetroTheme::DrawNeonFrame(ImGui::GetWindowDrawList(), calcPos,
                            calcMax,
                            RetroTheme::NeonPink(0.26f), (float)glfwGetTime() + 1.6f,
                            14.0f, 1.0f);

  if (lastMode == ExecMode::STEP_ONE && state.hasCalculation &&
      !state.calcLines.empty()) {
    // ---- Full calculation ----
    for (const auto &line : state.calcLines) {
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

  ImGui::EndChild();
}

void VisualizerEngine::renderCalcPanel(const AlgoState & /*state*/) {
  // Calculation is now rendered inline in renderVisualizeTab BOX 3.
}

void VisualizerEngine::renderGrid(int windowWidth, int windowHeight) {
  const int panelW = 760;
  int gridAreaW = windowWidth - panelW;
  int gridAreaH = windowHeight;
  if (gridAreaW <= 0 || gridAreaH <= 0 || activeTab != 1)
    return;

  glViewport(panelW, 0, gridAreaW, gridAreaH);

  bool isCircle    = currentAlgo->isCircleMode();
  bool isEllipse   = currentAlgo->isEllipseMode();
  bool isFill      = currentAlgo->isFillMode();
  bool isScanline  = currentAlgo->isScanlineMode();
  bool isClip      = currentAlgo->isClipMode();
  bool isPolyClip  = currentAlgo->isPolygonClipMode();
  bool isTransform = currentAlgo->isTransformMode();
  bool isWinViewport = currentAlgo->isWindowViewportMode();

  // Get clip window coordinates
  float cwXMin = (float)inputClipXMin, cwXMax = (float)inputClipXMax;
  float cwYMin = (float)inputClipYMin, cwYMax = (float)inputClipYMax;
  if (isClip) {
    auto* cs = dynamic_cast<const CohenSutherland*>(currentAlgo.get());
    if (cs) { cwXMin=cs->getXMin(); cwXMax=cs->getXMax(); cwYMin=cs->getYMin(); cwYMax=cs->getYMax(); }
  }
  if (isPolyClip) {
    auto* sh = dynamic_cast<const SutherlandHodgman*>(currentAlgo.get());
    if (sh) { cwXMin=sh->getXMin(); cwXMax=sh->getXMax(); cwYMin=sh->getYMin(); cwYMax=sh->getYMax(); }
  }

  float gMinX, gMinY, gMaxX, gMaxY;
  if (isWinViewport) {
    // Show both world window and viewport with padding
    auto* wv = dynamic_cast<const WindowViewport*>(currentAlgo.get());
    float allX[4] = {inputWxMin, inputWxMax, inputVxMin, inputVxMax};
    float allY[4] = {inputWyMin, inputWyMax, inputVyMin, inputVyMax};
    if (wv) {
      allX[0]=wv->getWxMin(); allX[1]=wv->getWxMax();
      allX[2]=wv->getVxMin(); allX[3]=wv->getVxMax();
      allY[0]=wv->getWyMin(); allY[1]=wv->getWyMax();
      allY[2]=wv->getVyMin(); allY[3]=wv->getVyMax();
    }
    float xlo = *std::min_element(allX,allX+4);
    float xhi = *std::max_element(allX,allX+4);
    float ylo = *std::min_element(allY,allY+4);
    float yhi = *std::max_element(allY,allY+4);
    float pad = std::max(xhi-xlo, yhi-ylo) * 0.15f + 3.f;
    float aspect = (float)gridAreaH / (float)gridAreaW;
    float cx2 = (xlo+xhi)*0.5f, cy2 = (ylo+yhi)*0.5f;
    float rangeX = (xhi-xlo)*0.5f + pad;
    float rangeY = rangeX * aspect;
    gMinX = cx2 - rangeX; gMaxX = cx2 + rangeX;
    gMinY = cy2 - rangeY; gMaxY = cy2 + rangeY;
  } else if (isTransform) {
    // Show all transform phases: original + final
    auto* tf = dynamic_cast<const Transform2D*>(currentAlgo.get());
    float pad = (float)std::max(inputTfSize, 1) * 2.5f
              + std::max(std::abs(inputTfTx), std::abs(inputTfTy));
    if (pad < 15.0f) pad = 15.0f;
    float aspect = (float)gridAreaH / (float)gridAreaW;
    gMinX = (float)inputTfCX - pad; gMaxX = (float)inputTfCX + pad;
    gMinY = (float)inputTfCY - pad * aspect; gMaxY = (float)inputTfCY + pad * aspect;
    (void)tf; // used below
  } else if (isPolyClip) {
    float pad = (float)std::max(inputSHSize, 1) * 1.7f;
    if (pad < 15.0f) pad = 15.0f;
    float extra = std::max({std::abs(cwXMin), std::abs(cwXMax),
                            std::abs(cwYMin), std::abs(cwYMax)}) + 2.0f;
    if (extra > pad) pad = extra;
    float aspect = (float)gridAreaH / (float)gridAreaW;
    gMinX = (float)inputSHCX - pad; gMaxX = (float)inputSHCX + pad;
    gMinY = (float)inputSHCY - pad * aspect; gMaxY = (float)inputSHCY + pad * aspect;
  } else if (isClip) {
    // Show entire clip window + line with padding
    int maxC = std::max({std::abs(inputX1), std::abs(inputY1),
                         std::abs(inputX2), std::abs(inputY2),
                         (int)std::abs(cwXMin), (int)std::abs(cwXMax),
                         (int)std::abs(cwYMin), (int)std::abs(cwYMax), 1});
    float pad = (float)maxC * 1.35f;
    if (pad < 15.0f) pad = 15.0f;
    float aspect = (float)gridAreaH / (float)gridAreaW;
    gMinX = -pad; gMaxX = pad;
    gMinY = -pad * aspect; gMaxY = pad * aspect;
  } else if (isScanline) {

    float pad = (float)inputPolySize * 1.6f;
    if (pad < 5.0f)
      pad = 5.0f;
    float aspect = (float)gridAreaH / (float)gridAreaW;
    gMinX = (float)inputPolyCX - pad;
    gMaxX = (float)inputPolyCX + pad;
    gMinY = (float)inputPolyCY - pad * aspect;
    gMaxY = (float)inputPolyCY + pad * aspect;
  } else if (isFill) {

    float pad = (float)inputHalfS * 1.55f;
    if (pad < 5.0f)
      pad = 5.0f;
    float aspect = (float)gridAreaH / (float)gridAreaW;
    gMinX = (float)inputSeedX - pad;
    gMaxX = (float)inputSeedX + pad;
    gMinY = (float)inputSeedY - pad * aspect;
    gMaxY = (float)inputSeedY + pad * aspect;
  } else if (isEllipse) {
    int maxR = std::max(std::abs(inputRX), std::abs(inputRY));
    float pad = (float)maxR * 1.55f;
    if (pad < 15.0f)
      pad = 15.0f;
    float aspect = (float)gridAreaH / (float)gridAreaW;
    gMinX = (float)inputCX - pad;
    gMaxX = (float)inputCX + pad;
    gMinY = (float)inputCY - pad * aspect;
    gMaxY = (float)inputCY + pad * aspect;
  } else if (isCircle) {
    float pad = (float)inputRadius * 1.45f;
    if (pad < 15.0f)
      pad = 15.0f;
    float aspect = (float)gridAreaH / (float)gridAreaW;
    gMinX = (float)inputCX - pad;
    gMaxX = (float)inputCX + pad;
    gMinY = (float)inputCY - pad * aspect;
    gMaxY = (float)inputCY + pad * aspect;
  } else {
    int maxCoord = std::max({std::abs(inputX1), std::abs(inputY1),
                             std::abs(inputX2), std::abs(inputY2), 1});
    float gridMax = (float)maxCoord * 1.3f;
    if (gridMax < 20.0f)
      gridMax = 20.0f;
    float gridMaxY = gridMax * ((float)gridAreaH / (float)gridAreaW);
    gMinX = -0.5f;
    gMaxX = gridMax + 0.5f;
    gMinY = -0.5f;
    gMaxY = gridMaxY + 0.5f;
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
    glVertex2f((float)i, gMinY);
    glVertex2f((float)i, gMaxY);
  }
  for (int i = (int)std::floor(gMinY); i <= (int)std::ceil(gMaxY); i++) {
    glVertex2f(gMinX, (float)i);
    glVertex2f(gMaxX, (float)i);
  }
  glEnd();

  glColor3f(0.40f, 0.43f, 0.52f);
  glLineWidth(1.5f);
  glBegin(GL_LINES);
  if (0.0f >= gMinX && 0.0f <= gMaxX) {
    glVertex2f(0.0f, gMinY);
    glVertex2f(0.0f, gMaxY);
  }
  if (0.0f >= gMinY && 0.0f <= gMaxY) {
    glVertex2f(gMinX, 0.0f);
    glVertex2f(gMaxX, 0.0f);
  }
  glEnd();
  glLineWidth(1.0f);

  glColor3f(0.45f, 0.45f, 0.75f);
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(1, 0xAAAA);

  if (isWinViewport) {
    auto* wv2 = dynamic_cast<const WindowViewport*>(currentAlgo.get());
    float wxlo=inputWxMin, wxhi=inputWxMax, wylo=inputWyMin, wyhi=inputWyMax;
    float vxlo=inputVxMin, vxhi=inputVxMax, vylo=inputVyMin, vyhi=inputVyMax;
    if (wv2) {
      wxlo=wv2->getWxMin(); wxhi=wv2->getWxMax();
      wylo=wv2->getWyMin(); wyhi=wv2->getWyMax();
      vxlo=wv2->getVxMin(); vxhi=wv2->getVxMax();
      vylo=wv2->getVyMin(); vyhi=wv2->getVyMax();
    }
    // World window — dashed orange
    glColor3f(1.0f, 0.6f, 0.1f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(wxlo, wylo); glVertex2f(wxhi, wylo);
    glVertex2f(wxhi, wyhi); glVertex2f(wxlo, wyhi);
    glEnd();
    // Viewport — dashed green
    glColor3f(0.2f, 1.0f, 0.45f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(vxlo, vylo); glVertex2f(vxhi, vylo);
    glVertex2f(vxhi, vyhi); glVertex2f(vxlo, vyhi);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
  } else if (isPolyClip) {
    // Dashed clip rectangle (SH)
    glBegin(GL_LINE_LOOP);
    glVertex2f(cwXMin, cwYMin); glVertex2f(cwXMax, cwYMin);
    glVertex2f(cwXMax, cwYMax); glVertex2f(cwXMin, cwYMax);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    // Original polygon outline (dim)
    glColor3f(0.50f, 0.50f, 0.62f);
    const float pi2 = 2.0f * 3.14159265f;
    float sa = -3.14159265f / 2.0f;
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < inputSHSides; ++i) {
      float a = sa + (float)i * pi2 / (float)inputSHSides;
      glVertex2f((float)inputSHCX + 0.5f + (float)inputSHSize * cosf(a),
                 (float)inputSHCY + 0.5f + (float)inputSHSize * sinf(a));
    }
    glEnd();
  } else if (isClip) {
    // Dashed clip rectangle
    glBegin(GL_LINE_LOOP);
    glVertex2f(cwXMin, cwYMin);
    glVertex2f(cwXMax, cwYMin);
    glVertex2f(cwXMax, cwYMax);
    glVertex2f(cwXMin, cwYMax);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    // Full original line in dim colour
    glColor3f(0.55f, 0.55f, 0.65f);
    glBegin(GL_LINES);
    glVertex2f((float)inputX1 + 0.5f, (float)inputY1 + 0.5f);
    glVertex2f((float)inputX2 + 0.5f, (float)inputY2 + 0.5f);
    glEnd();
  } else if (isScanline) {

    float angleStep = 2.0f * 3.14159265f / (float)inputPolySides;
    float startAngle = -3.14159265f / 2.0f;
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < inputPolySides; i++) {
      float a = startAngle + (float)i * angleStep;
      glVertex2f((float)inputPolyCX + 0.5f + (float)inputPolySize * cosf(a),
                 (float)inputPolyCY + 0.5f + (float)inputPolySize * sinf(a));
    }
    glEnd();
  } else if (isFill) {

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
    glVertex2f((float)inputX1 + 0.5f, (float)inputY1 + 0.5f);
    glVertex2f((float)inputX2 + 0.5f, (float)inputY2 + 0.5f);
    glEnd();
  }

  glDisable(GL_LINE_STIPPLE);

  std::vector<Pixel> pixels = currentAlgo->getHighlightedPixels();
  AlgoState state = currentAlgo->getCurrentState();
  int numPixels = (int)pixels.size();

  int highlightStart = -1;
  if (lastMode == ExecMode::STEP_ONE && numPixels > 0) {
    highlightStart = numPixels - state.pixelsPerStep;
    if (highlightStart < 0)
      highlightStart = 0;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBegin(GL_QUADS);
  for (int i = 0; i < numPixels; i++) {
    float px = (float)pixels[i].x;
    float py = (float)pixels[i].y;
    float iv = pixels[i].intensity;

    bool isCurrentStep =
        (highlightStart >= 0 && i >= highlightStart && pixels[i].type != 3);

    if (isCurrentStep) {

      glColor4f(1.0f, 0.95f, 0.15f, iv);
    } else if (pixels[i].type == 3) {

      glColor4f(0.5f, 0.5f, 0.55f, 1.0f);
    } else if (pixels[i].type == 2) {

      glColor4f(0.2f, 0.8f, 0.35f, iv);
    } else if (pixels[i].type == 1) {

      glColor4f(1.0f, 0.6f, 0.0f, iv);
    } else if (i == 0 && pixels[i].type == 0) {

      glColor4f(0.2f, 0.9f, 0.35f, iv);
    } else {
      // Default line/circle pixel (Red)
      glColor4f(0.85f, 0.22f, 0.18f, iv);
    }

    glVertex2f(px + 0.07f, py + 0.07f);
    glVertex2f(px + 0.93f, py + 0.07f);
    glVertex2f(px + 0.93f, py + 0.93f);
    glVertex2f(px + 0.07f, py + 0.93f);
  }
  glEnd();
  glDisable(GL_BLEND);

  // ---- Sutherland-Hodgman: draw filled polygons ----
  if (isPolyClip) {
    auto* sh = dynamic_cast<const SutherlandHodgman*>(currentAlgo.get());
    if (sh) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // Original polygon — semi-transparent dim grey
      const auto& origPoly = sh->getOriginalPoly();
      if (origPoly.size() >= 3) {
        glColor4f(0.55f, 0.55f, 0.70f, 0.25f);
        glBegin(GL_TRIANGLE_FAN);
        // centroid as fan anchor
        float cx = 0, cy = 0;
        for (auto& v : origPoly) { cx += v.x; cy += v.y; }
        cx /= (float)origPoly.size(); cy /= (float)origPoly.size();
        glVertex2f(cx, cy);
        for (auto& v : origPoly) glVertex2f(v.x, v.y);
        glVertex2f(origPoly[0].x, origPoly[0].y); // close fan
        glEnd();
        // Solid border (dim)
        glColor4f(0.55f, 0.55f, 0.75f, 0.60f);
        glBegin(GL_LINE_LOOP);
        for (auto& v : origPoly) glVertex2f(v.x, v.y);
        glEnd();
      }

      // Clipped polygon — semi-transparent green fill
      const auto& clippedPoly = sh->getCurrentPoly();
      if (clippedPoly.size() >= 3) {
        glColor4f(0.15f, 0.80f, 0.30f, 0.40f);
        glBegin(GL_TRIANGLE_FAN);
        float cx = 0, cy = 0;
        for (auto& v : clippedPoly) { cx += v.x; cy += v.y; }
        cx /= (float)clippedPoly.size(); cy /= (float)clippedPoly.size();
        glVertex2f(cx, cy);
        for (auto& v : clippedPoly) glVertex2f(v.x, v.y);
        glVertex2f(clippedPoly[0].x, clippedPoly[0].y);
        glEnd();
        // Bright green border
        glColor4f(0.20f, 1.00f, 0.40f, 0.90f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (auto& v : clippedPoly) glVertex2f(v.x, v.y);
        glEnd();
        glLineWidth(1.0f);
      } else if (clippedPoly.size() == 2) {
        // Degenerate — just draw a line
        glColor4f(0.20f, 1.00f, 0.40f, 0.90f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(clippedPoly[0].x, clippedPoly[0].y);
        glVertex2f(clippedPoly[1].x, clippedPoly[1].y);
        glEnd();
        glLineWidth(1.0f);
      }

      glDisable(GL_BLEND);
    }
  }

  // ---- Transform2D: draw all phase polygons filled ----
  if (isTransform) {
    auto* tf = dynamic_cast<const Transform2D*>(currentAlgo.get());
    if (tf) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // Helper lambda to draw a filled + outlined polygon
      static const auto drawFilledPoly = [](const std::vector<Transform2D::Vertex>& poly,
                                            float r, float g, float b, float aFill, float aLine) {
        if (poly.size() < 3) return;
        float cx = 0, cy = 0;
        for (auto& v : poly) { cx += v.x; cy += v.y; }
        cx /= (float)poly.size(); cy /= (float)poly.size();
        // Fill
        glColor4f(r, g, b, aFill);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (auto& v : poly) glVertex2f(v.x, v.y);
        glVertex2f(poly[0].x, poly[0].y);
        glEnd();
        // Border
        glColor4f(r, g, b, aLine);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (auto& v : poly) glVertex2f(v.x, v.y);
        glEnd();
        glLineWidth(1.0f);
      };

      int phase = tf->getCurrentPhase();
      // Original — always dim blue-grey
      drawFilledPoly(tf->getOriginalPoly(), 0.55f, 0.55f, 0.80f, 0.20f, 0.55f);
      // Intermediate phases with increasing brightness
      if (phase >= 1) drawFilledPoly(tf->getPhase(1), 1.00f, 0.75f, 0.15f, 0.25f, 0.70f); // translate: orange
      if (phase >= 2) drawFilledPoly(tf->getPhase(2), 0.15f, 0.75f, 1.00f, 0.25f, 0.70f); // rotate: blue
      if (phase >= 3) drawFilledPoly(tf->getPhase(3), 0.20f, 1.00f, 0.40f, 0.40f, 0.95f); // scale: green

      glDisable(GL_BLEND);
    }
  }

  // ---- Window-to-Viewport: draw filled world + viewport polygons ----
  if (isWinViewport) {
    auto* wv3 = dynamic_cast<const WindowViewport*>(currentAlgo.get());
    if (wv3) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // Helper: fill + outline a polygon
      auto drawWVPoly = [](const std::vector<WindowViewport::Vertex>& poly,
                           float r, float g, float b, float aFill, float aLine) {
        if (poly.size() < 3) return;
        float cx = 0, cy = 0;
        for (auto& v : poly) { cx += v.x; cy += v.y; }
        cx /= (float)poly.size(); cy /= (float)poly.size();
        glColor4f(r, g, b, aFill);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (auto& v : poly) glVertex2f(v.x, v.y);
        glVertex2f(poly[0].x, poly[0].y);
        glEnd();
        glColor4f(r, g, b, aLine);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (auto& v : poly) glVertex2f(v.x, v.y);
        glEnd();
        glLineWidth(1.0f);
      };

      // World polygon — orange fill
      drawWVPoly(wv3->getWorldPoly(), 1.0f, 0.55f, 0.1f, 0.30f, 0.85f);
      // Viewport polygon — green fill (only after step 2)
      if (!wv3->getViewportPoly().empty())
        drawWVPoly(wv3->getViewportPoly(), 0.2f, 1.0f, 0.4f, 0.40f, 0.95f);

      glDisable(GL_BLEND);
    }
  }

  glLineWidth(2.0f);
  if (isWinViewport) {
    auto* wv4 = dynamic_cast<const WindowViewport*>(currentAlgo.get());
    float wxlo=inputWxMin, wxhi=inputWxMax, wylo=inputWyMin, wyhi=inputWyMax;
    float vxlo=inputVxMin, vxhi=inputVxMax, vylo=inputVyMin, vyhi=inputVyMax;
    if (wv4) {
      wxlo=wv4->getWxMin(); wxhi=wv4->getWxMax();
      wylo=wv4->getWyMin(); wyhi=wv4->getWyMax();
      vxlo=wv4->getVxMin(); vxhi=wv4->getVxMax();
      vylo=wv4->getVyMin(); vyhi=wv4->getVyMax();
    }
    // World window corners — orange
    float wCorners[4][2] = {{wxlo,wylo},{wxhi,wylo},{wxhi,wyhi},{wxlo,wyhi}};
    glColor3f(1.0f, 0.55f, 0.1f);
    glBegin(GL_QUADS);
    for (auto& c : wCorners) {
      glVertex2f(c[0]+0.1f, c[1]+0.1f); glVertex2f(c[0]+0.9f, c[1]+0.1f);
      glVertex2f(c[0]+0.9f, c[1]+0.9f); glVertex2f(c[0]+0.1f, c[1]+0.9f);
    }
    glEnd();
    // Viewport corners — green
    float vCorners[4][2] = {{vxlo,vylo},{vxhi,vylo},{vxhi,vyhi},{vxlo,vyhi}};
    glColor3f(0.2f, 1.0f, 0.45f);
    glBegin(GL_QUADS);
    for (auto& c : vCorners) {
      glVertex2f(c[0]+0.1f, c[1]+0.1f); glVertex2f(c[0]+0.9f, c[1]+0.1f);
      glVertex2f(c[0]+0.9f, c[1]+0.9f); glVertex2f(c[0]+0.1f, c[1]+0.9f);
    }
    glEnd();
  } else if (isTransform) {
    // Centre dot of original polygon (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    float bx = (float)inputTfCX + 0.25f, by = (float)inputTfCY + 0.25f;
    glVertex2f(bx, by); glVertex2f(bx+0.5f, by);
    glVertex2f(bx+0.5f, by+0.5f); glVertex2f(bx, by+0.5f);
    glEnd();
  } else if (isPolyClip) {
    // Clip-window corner dots (cyan)
    glColor3f(0.3f, 1.0f, 1.0f);
    float corners[4][2] = {
      {cwXMin, cwYMin}, {cwXMax, cwYMin},
      {cwXMax, cwYMax}, {cwXMin, cwYMax}
    };
    glBegin(GL_QUADS);
    for (auto& c : corners) {
      glVertex2f(c[0] + 0.1f, c[1] + 0.1f);
      glVertex2f(c[0] + 0.9f, c[1] + 0.1f);
      glVertex2f(c[0] + 0.9f, c[1] + 0.9f);
      glVertex2f(c[0] + 0.1f, c[1] + 0.9f);
    }
    glEnd();
    // Polygon centre dot (yellow)
    glColor3f(1.0f, 0.9f, 0.2f);
    glBegin(GL_QUADS);
    float bx = (float)inputSHCX + 0.25f, by = (float)inputSHCY + 0.25f;
    glVertex2f(bx, by); glVertex2f(bx+0.5f, by);
    glVertex2f(bx+0.5f, by+0.5f); glVertex2f(bx, by+0.5f);
    glEnd();
  } else if (isClip) {
    // P1 marker (green)
    glColor3f(0.2f, 1.0f, 0.4f);
    float sx = (float)inputX1, sy = (float)inputY1;
    glBegin(GL_LINE_LOOP);
    glVertex2f(sx + 0.02f, sy + 0.02f);
    glVertex2f(sx + 0.98f, sy + 0.02f);
    glVertex2f(sx + 0.98f, sy + 0.98f);
    glVertex2f(sx + 0.02f, sy + 0.98f);
    glEnd();
    // P2 marker (blue)
    glColor3f(0.35f, 0.65f, 1.0f);
    float ex = (float)inputX2, ey = (float)inputY2;
    glBegin(GL_LINE_LOOP);
    glVertex2f(ex + 0.02f, ey + 0.02f);
    glVertex2f(ex + 0.98f, ey + 0.02f);
    glVertex2f(ex + 0.98f, ey + 0.98f);
    glVertex2f(ex + 0.02f, ey + 0.98f);
    glEnd();
    // Clip-window corner dots (cyan)
    glColor3f(0.3f, 1.0f, 1.0f);
    float corners[4][2] = {
      {cwXMin, cwYMin}, {cwXMax, cwYMin},
      {cwXMax, cwYMax}, {cwXMin, cwYMax}
    };
    glBegin(GL_QUADS);
    for (auto& c : corners) {
      glVertex2f(c[0] + 0.1f, c[1] + 0.1f);
      glVertex2f(c[0] + 0.9f, c[1] + 0.1f);
      glVertex2f(c[0] + 0.9f, c[1] + 0.9f);
      glVertex2f(c[0] + 0.1f, c[1] + 0.9f);
    }
    glEnd();
  } else if (isScanline) {

    glColor3f(0.3f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    float bx = (float)inputPolyCX + 0.25f, by = (float)inputPolyCY + 0.25f;
    glVertex2f(bx, by);
    glVertex2f(bx + 0.5f, by);
    glVertex2f(bx + 0.5f, by + 0.5f);
    glVertex2f(bx, by + 0.5f);
    glEnd();
  } else if (isFill) {

    glColor3f(1.0f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    float bx = (float)inputSeedX + 0.3f, by = (float)inputSeedY + 0.3f;
    glVertex2f(bx, by);
    glVertex2f(bx + 0.4f, by);
    glVertex2f(bx + 0.4f, by + 0.4f);
    glVertex2f(bx, by + 0.4f);
    glEnd();
  } else if (isCircle || isEllipse) {
    // Centre dot
    glColor3f(0.2f, 1.0f, 0.4f);
    glBegin(GL_QUADS);
    float bx = (float)inputCX + 0.3f, by = (float)inputCY + 0.3f;
    glVertex2f(bx, by);
    glVertex2f(bx + 0.4f, by);
    glVertex2f(bx + 0.4f, by + 0.4f);
    glVertex2f(bx, by + 0.4f);
    glEnd();
  } else {
    // Start marker (green border)
    glColor3f(0.2f, 1.0f, 0.4f);
    float sx = (float)inputX1, sy = (float)inputY1;
    glBegin(GL_LINE_LOOP);
    glVertex2f(sx + 0.02f, sy + 0.02f);
    glVertex2f(sx + 0.98f, sy + 0.02f);
    glVertex2f(sx + 0.98f, sy + 0.98f);
    glVertex2f(sx + 0.02f, sy + 0.98f);
    glEnd();
    // End marker (blue border)
    glColor3f(0.35f, 0.65f, 1.0f);
    float ex = (float)inputX2, ey = (float)inputY2;
    glBegin(GL_LINE_LOOP);
    glVertex2f(ex + 0.02f, ey + 0.02f);
    glVertex2f(ex + 0.98f, ey + 0.02f);
    glVertex2f(ex + 0.98f, ey + 0.98f);
    glVertex2f(ex + 0.02f, ey + 0.98f);
    glEnd();
  }
  glLineWidth(1.0f);

  glViewport(0, 0, windowWidth, windowHeight);
}
