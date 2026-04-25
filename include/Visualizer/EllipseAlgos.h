#pragma once
#include "IAlgorithm.h"
#include <cmath>

enum class EllipseRegion { REGION1, REGION2, DONE };

// ============================================================
//  MidpointEllipse
//  Maps:  init(cx, cy, a, b)
//    a  = semi-axis in X direction
//    b  = semi-axis in Y direction
//  4-fold symmetry: (+x,+y), (-x,+y), (+x,-y), (-x,-y)
//
//  REGION 1  (|slope| > 1): x always increments
//  REGION 2  (|slope| < 1): y always decrements
//  Transition when:  2b²x >= 2a²y
// ============================================================
class MidpointEllipse : public IAlgorithm {
private:
  int cx, cy, a, b;
  int a2_, b2_;

  int x_, y_;
  float p1_, p2_;
  EllipseRegion region_;

  int currentStep, totalSteps;

  std::vector<Pixel> path;
  AlgoState lastState;

  void plotSymmetric(int x, int y, int &delta);

  void computeTotalSteps();

  void stepRegion1();
  void stepRegion2();

public:
  MidpointEllipse();

  void init(int x1, int y1, int x2, int y2) override;
  void step() override;
  void stepK(int k) override;
  void runToCompletion() override;
  void reset() override;

  bool isFinished() const override;
  std::vector<Pixel> getHighlightedPixels() const override;
  AlgoState getCurrentState() const override;
  std::vector<std::string> getInitInfo() const override;
  std::vector<std::string> getCurrentVars() const override;
  std::string getTheory() const override;
  std::string getName() const override;

  bool isCircleMode() const override { return false; }
  bool isEllipseMode() const override { return true; }
};
