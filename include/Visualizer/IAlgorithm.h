#pragma once
#include <string>
#include <vector>

struct Pixel {
  int x, y;
  float intensity = 1.0f;
  int type = 0; // 0: default(red), 1: frontier(orange), 2: filled(green), 3:
                // boundary(grey)
};

//  AlgoState — snapshot of one step for the UI panel

struct AlgoState {
  int currentStep = 0;
  int totalSteps = 0;
  bool hasCalculation = false;
  int pixelsPerStep = 1;
  std::vector<std::string> calcLines;
  std::string currentPixelInfo;
};

class IAlgorithm {
public:
  virtual ~IAlgorithm() = default;

  virtual void init(int x1, int y1, int x2, int y2) = 0;
  virtual void step() = 0;
  virtual void stepK(int k) = 0;
  virtual void runToCompletion() = 0;
  virtual void reset() = 0;

  virtual bool isFinished() const = 0;
  virtual std::vector<Pixel> getHighlightedPixels() const = 0;
  virtual AlgoState getCurrentState() const = 0;
  virtual std::vector<std::string> getInitInfo() const = 0;
  virtual std::vector<std::string> getCurrentVars() const = 0;
  virtual std::string getTheory() const = 0;
  virtual std::string getName() const = 0;

  virtual bool isCircleMode() const { return false; }

  virtual bool isEllipseMode() const { return false; }

  virtual bool isFillMode() const { return false; }

  virtual bool isScanlineMode() const { return false; }

  virtual bool isClipMode() const { return false; }

  virtual bool isPolygonClipMode() const { return false; }

  virtual bool isTransformMode() const { return false; }

  virtual bool isWindowViewportMode() const { return false; }
};