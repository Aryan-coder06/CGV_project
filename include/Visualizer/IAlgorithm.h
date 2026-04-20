#pragma once
#include <vector>
#include <string>

// ============================================================
//  Pixel — a single rasterized grid cell
//  intensity: 1.0 = fully opaque, 0.0 = invisible
//             (always 1.0 for DDA/Bresenham; fractional for Wu)
// ============================================================
struct Pixel {
    int   x, y;
    float intensity = 1.0f;
    int   type = 0;  // 0: default(red), 1: frontier(orange), 2: filled(green), 3: boundary(grey)
};

// ============================================================
//  AlgoState — snapshot of one step for the UI panel
// ============================================================
struct AlgoState {
    int  currentStep   = 0;
    int  totalSteps    = 0;
    bool hasCalculation = false;
    int  pixelsPerStep  = 1;    // Wu uses 2 per step; DDA/Bresenham use 1
    std::vector<std::string> calcLines;
    std::string currentPixelInfo;
};

// ============================================================
//  IAlgorithm — abstract base for every CGV algorithm
// ============================================================
class IAlgorithm {
public:
    virtual ~IAlgorithm() = default;

    virtual void init(int x1, int y1, int x2, int y2) = 0;
    virtual void step()            = 0;
    virtual void stepK(int k)      = 0;
    virtual void runToCompletion() = 0;
    virtual void reset()           = 0;

    virtual bool                     isFinished()           const = 0;
    virtual std::vector<Pixel>        getHighlightedPixels() const = 0;
    virtual AlgoState                 getCurrentState()      const = 0;
    virtual std::vector<std::string>  getInitInfo()          const = 0;
    virtual std::vector<std::string>  getCurrentVars()       const = 0;
    virtual std::string               getTheory()            const = 0;
    virtual std::string               getName()              const = 0;

    // Returns true for algorithms that use (cx,cy,r) instead of two endpoints
    virtual bool isCircleMode()  const { return false; }

    // Returns true for algorithms that use (cx,cy,a,b) — ellipse semi-axes
    virtual bool isEllipseMode() const { return false; }

    // Returns true for filling algorithms that require a seed point (e.g. Flood Fill)
    virtual bool isFillMode() const { return false; }

    // Returns true for scanline polygon fill (uses cx,cy,polySize,numSides)
    virtual bool isScanlineMode() const { return false; }
};