#pragma once
#include "IAlgorithm.h"
#include <cmath>

// ============================================================
//  DDALine
// ============================================================
class DDALine : public IAlgorithm {
private:
    int   x1, y1, x2, y2;
    float currentX, currentY;
    float xIncrement, yIncrement;
    int   totalSteps, currentStep;
    std::vector<Pixel> path;
    AlgoState lastState;
    void calculateIncrements();
public:
    DDALine();
    void init(int x1,int y1,int x2,int y2) override;
    void step()            override;
    void stepK(int k)      override;
    void runToCompletion() override;
    void reset()           override;
    bool                     isFinished()           const override;
    std::vector<Pixel>       getHighlightedPixels() const override;
    AlgoState                getCurrentState()      const override;
    std::vector<std::string> getInitInfo()          const override;
    std::vector<std::string> getCurrentVars()       const override;
    std::string              getTheory()            const override;
    std::string              getName()              const override;
};

// ============================================================
//  BresenhamLine
// ============================================================
class BresenhamLine : public IAlgorithm {
private:
    int x1,y1,x2,y2, x,y, dx,dy, sx,sy, p;
    bool steep;
    int  totalSteps, currentStep;
    std::vector<Pixel> path;
    AlgoState lastState;
public:
    BresenhamLine();
    void init(int x1,int y1,int x2,int y2) override;
    void step()            override;
    void stepK(int k)      override;
    void runToCompletion() override;
    void reset()           override;
    bool                     isFinished()           const override;
    std::vector<Pixel>       getHighlightedPixels() const override;
    AlgoState                getCurrentState()      const override;
    std::vector<std::string> getInitInfo()          const override;
    std::vector<std::string> getCurrentVars()       const override;
    std::string              getTheory()            const override;
    std::string              getName()              const override;
    int  getP()    const { return p; }
    int  getDx()   const { return dx; }
    int  getDy()   const { return dy; }
    bool isSteep() const { return steep; }
};

// ============================================================
//  XiaolinWuLine — anti-aliased line via fractional intensities
//  Plots 2 pixels per step (upper + lower neighbour) with
//  complementary intensities derived from frac(y).
// ============================================================
class XiaolinWuLine : public IAlgorithm {
private:
    // Original inputs (stored for reset / display)
    int ox0, oy0, ox1, oy1;

    // Processed co-ordinates (after steep-swap and direction-swap)
    float fpx0, fpy0, fpx1, fpy1;

    // Algorithm invariants
    bool  steep;
    float gradient;
    int   xpxl1, xpxl2;       // x range for main loop
    float yend1,  xgap1;      // first endpoint fractional data
    float yend2,  xgap2;      // last  endpoint fractional data

    // Running state
    float intery;              // interpolated y (advances by gradient each step)
    int   currentX;
    int   currentStep, totalSteps;

    std::vector<Pixel> path;
    AlgoState lastState;

    // ---------- helpers ----------
    static float wu_frac (float x) { return x - std::floor(x); }
    static float wu_rfrac(float x) { return 1.0f - wu_frac(x); }
    static int   wu_ipart(float x) { return (int)std::floor(x); }
    static int   wu_round(float x) { return wu_ipart(x + 0.5f); }

    // Plot one pixel, un-swapping co-ordinates if line was steep
    void plotPixel(int x, int y, float intensity);

    // Raw (silent) single-step — used by stepK / runToCompletion
    void doStep();

public:
    XiaolinWuLine();
    void init(int x0,int y0,int x1,int y1) override;
    void step()            override;   // advances + records calculation
    void stepK(int k)      override;   // silent
    void runToCompletion() override;   // silent
    void reset()           override;
    bool                     isFinished()           const override;
    std::vector<Pixel>       getHighlightedPixels() const override;
    AlgoState                getCurrentState()      const override;
    std::vector<std::string> getInitInfo()          const override;
    std::vector<std::string> getCurrentVars()       const override;
    std::string              getTheory()            const override;
    std::string              getName()              const override;
};