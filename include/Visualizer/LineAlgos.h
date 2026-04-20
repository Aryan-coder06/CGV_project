#pragma once
#include "IAlgorithm.h"

// ============================================================
//  DDALine — Digital Differential Analyzer line algorithm
//
//  Inherits IAlgorithm and tracks full per-step calculations
//  so that the UI can show the exact math when the user clicks
//  "Step +1", but hides them for "Step K" and "Run All".
// ============================================================
class DDALine : public IAlgorithm {
private:
    // Endpoint storage (kept for reset())
    int   x1, y1, x2, y2;

    // Running floating-point position
    float currentX, currentY;

    // Computed once in init()
    float xIncrement, yIncrement;
    int   totalSteps, currentStep;

    // All pixels plotted so far (grows each step)
    std::vector<Pixel> path;

    // Most-recent step's state snapshot
    AlgoState lastState;

    // Internal helpers
    void calculateIncrements();
    void pushPixel(float cx, float cy);   // rounds & appends to path

public:
    DDALine();

    // ----- IAlgorithm overrides -----
    void init(int startX, int startY, int endX, int endY) override;

    // step() — advances 1 step AND populates lastState.calcLines
    void step() override;

    // stepK() — advances k steps silently (no calc lines stored)
    void stepK(int k) override;

    // runToCompletion() — runs all remaining steps silently
    void runToCompletion() override;

    bool                 isFinished()           const override;
    std::vector<Pixel>   getHighlightedPixels()  const override;
    AlgoState            getCurrentState()        const override;
    std::string          getTheory()              const override;
    std::string          getName()                const override;
    void                 reset()                  override;

    // ----- Fine-grained getters (used by VisualizerEngine) -----
    float getCurrentX()    const { return currentX; }
    float getCurrentY()    const { return currentY; }
    float getXInc()        const { return xIncrement; }
    float getYInc()        const { return yIncrement; }
    int   getSteps()       const { return totalSteps; }
    int   getCurrentStepNum() const { return currentStep; }
    int   getX1() const { return x1; }
    int   getY1() const { return y1; }
    int   getX2() const { return x2; }
    int   getY2() const { return y2; }
};