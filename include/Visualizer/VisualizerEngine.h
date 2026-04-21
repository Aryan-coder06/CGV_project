#pragma once
#include "CircleAlgos.h"
#include "ClipAlgos.h"
#include "EllipseAlgos.h"
#include "FillAlgos.h"
#include "LineAlgos.h"
#include "TransformAlgos.h"
#include "WindowViewportAlgo.h"
#include <memory>

//  Execution mode — controls whether the calc panel is shown

enum class ExecMode { NONE, STEP_ONE, STEP_K, RUN_ALL };

//  Algorithm index constants

static constexpr int ALGO_DDA = 0;
static constexpr int ALGO_BRESENHAM = 1;
static constexpr int ALGO_XIAOLIN_WU = 2;
static constexpr int ALGO_MIDPOINT_CIRCLE = 3;
static constexpr int ALGO_BRESENHAM_CIRCLE = 4;
static constexpr int ALGO_MIDPOINT_ELLIPSE = 5;
static constexpr int ALGO_FLOOD_FILL_4 = 6;
static constexpr int ALGO_FLOOD_FILL_8 = 7;
static constexpr int ALGO_BOUNDARY_FILL_4 = 8;
static constexpr int ALGO_BOUNDARY_FILL_8 = 9;
static constexpr int ALGO_SCANLINE_FILL       = 10;
static constexpr int ALGO_COHEN_SUTHERLAND   = 11;
static constexpr int ALGO_SUTHERLAND_HODGMAN = 12;
static constexpr int ALGO_TRANSFORM_2D       = 13;
static constexpr int ALGO_WINDOW_VIEWPORT    = 14;
static constexpr int ALGO_COUNT              = 15;

class VisualizerEngine {
private:
  std::unique_ptr<IAlgorithm> currentAlgo;

  int selectedAlgo = ALGO_DDA;
  int prevAlgo = -1;

  // Line-mode inputs
  int inputX1 = 0, inputY1 = 0;
  int inputX2 = 10, inputY2 = 8;
  int kSteps = 3;

  int inputCX = 0, inputCY = 0, inputRadius = 8;

  int inputRX = 10, inputRY = 6;

  int inputSeedX = 10, inputSeedY = 10, inputHalfS = 8;

  int inputPolyCX = 10, inputPolyCY = 10, inputPolySize = 7, inputPolySides = 5;

  // Cohen-Sutherland / Sutherland-Hodgman shared clip window
  int inputClipXMin = -6, inputClipXMax = 6;
  int inputClipYMin = -5, inputClipYMax = 5;

  // Sutherland-Hodgman polygon inputs
  int inputSHCX = 0, inputSHCY = 0, inputSHSize = 8, inputSHSides = 5;

  // 2D Transform inputs
  int   inputTfCX = 0, inputTfCY = 0, inputTfSize = 8, inputTfSides = 5;
  float inputTfTx = 4.0f,  inputTfTy = 3.0f;
  float inputTfAngle = 45.0f;
  float inputTfSx = 1.5f,  inputTfSy = 1.5f;

  // Window-to-Viewport inputs
  //  World window
  float inputWxMin = -10.f, inputWxMax = 10.f;
  float inputWyMin = -10.f, inputWyMax = 10.f;
  //  Viewport
  float inputVxMin = 20.f, inputVxMax = 40.f;
  float inputVyMin =  5.f, inputVyMax = 25.f;
  //  Polygon in world coords
  int inputWVCX = 0, inputWVCY = 0, inputWVSize = 5, inputWVSides = 5;

  int activeTab = 1; // 0=Theory, 1=Visualize
  ExecMode lastMode = ExecMode::NONE;

  std::unique_ptr<IAlgorithm> createAlgorithm(int idx);

  void renderTheoryTab();
  void renderVisualizeTab();
  void renderCalcPanel(const AlgoState &state);

public:
  VisualizerEngine();

  void renderUI();
  void renderGrid(int windowWidth, int windowHeight);
};