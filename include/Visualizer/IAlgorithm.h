#pragma once
#include <vector>
#include <string>

// ============================================================
//  Pixel — a single rasterized grid cell
// ============================================================
struct Pixel {
    int x, y;
};

// ============================================================
//  AlgoState — snapshot of one algorithm step for the UI panel
//  hasCalculation is TRUE only when the user clicked "Step +1"
//  For K-step and Run-All modes it stays FALSE so the UI
//  omits the detailed math panel.
// ============================================================
struct AlgoState {
    int currentStep  = 0;
    int totalSteps   = 0;

    // Filled only during single-step (Step +1) execution
    bool                     hasCalculation = false;
    std::vector<std::string> calcLines;        // one line per sub-calculation
    std::string              currentPixelInfo; // e.g. ">> Plotting pixel (4, 3)"
};

// ============================================================
//  IAlgorithm — abstract base for every CGV algorithm
// ============================================================
class IAlgorithm {
public:
    virtual ~IAlgorithm() = default;

    // ----- Core execution controls -----
    virtual void init(int startX, int startY, int endX, int endY) = 0;
    virtual void step()              = 0;   // advance 1 step; records calculations
    virtual void stepK(int k)        = 0;   // advance k steps; no detailed calcs
    virtual void runToCompletion()   = 0;   // run all; no detailed calcs

    // ----- State queries -----
    virtual bool                 isFinished()          const = 0;
    virtual std::vector<Pixel>   getHighlightedPixels() const = 0;
    virtual AlgoState            getCurrentState()      const = 0;

    // ----- Static info for the Theory panel -----
    virtual std::string getTheory() const = 0;
    virtual std::string getName()   const = 0;

    // ----- Utility -----
    virtual void reset() = 0;
};