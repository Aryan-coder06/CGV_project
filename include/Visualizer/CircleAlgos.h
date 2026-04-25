#pragma once
#include "IAlgorithm.h"
#include <cmath>

// ============================================================
//  MidpointCircle
//  Interface mapping:  init(cx, cy, r, 0)
//                       x1  y1  x2  (y2 ignored)
//  Produces 8 pixels per step via 8-fold symmetry.
//  Uses only integer arithmetic (no trig, no sqrt).
// ============================================================
class MidpointCircle : public IAlgorithm {
private:
  int cx, cy, r;

  int x_, y_, p_;
  int currentStep, totalSteps;

  std::vector<Pixel> path;
  AlgoState lastState;

  std::string plotOctants(int x, int y);

  void computeTotalSteps();

public:
  MidpointCircle();

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
  bool isCircleMode() const override { return true; }
};

// ============================================================
//  BresenhamCircle
//  Uses decision parameter  d = 3 - 2*r  (Bresenham's formulation).
//  Produces identical pixels to MidpointCircle but with a
//  different derivation — useful for pedagogical comparison.
//  Interface mapping:  init(cx, cy, r, 0)
// ============================================================
class BresenhamCircle : public IAlgorithm {
private:
  int cx, cy, r;

  // Running state
  int x_, y_, d_;
  int currentStep, totalSteps;

  std::vector<Pixel> path;
  AlgoState lastState;

  std::string plotOctants(int x, int y);

  void computeTotalSteps();

public:
  BresenhamCircle();

  // Use x1=cx, y1=cy, x2=r (y2 ignored)
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
  bool isCircleMode() const override { return true; }
};
