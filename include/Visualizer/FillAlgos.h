#pragma once
#include "IAlgorithm.h"
#include <queue>
#include <set>
#include <map>
#include <algorithm>

// ============================================================
//  FloodFill (4-way and 8-way)
//  init(cx, cy, halfS, 0):
//    cx,cy  = fill seed AND centre of the square boundary
//    halfS  = half the side length  (boundary occupies
//             [cx-halfS .. cx+halfS] x [cy-halfS .. cy+halfS])
// ============================================================
class FloodFill : public IAlgorithm {
protected:
    int seedX, seedY;
    int halfS;                  // dynamic square boundary half-side
    int currentStep, totalSteps;

    // Internal state
    std::queue<Pixel>                  frontier;
    std::set<std::pair<int,int>>       filledRaw;

    std::vector<Pixel> path;    // filled(green) pixels
    AlgoState          lastState;

    bool is8Way;

    // Dynamic boundary based on (seedX, seedY, halfS)
    bool isBoundary(int x, int y) const;
    bool isValid   (int x, int y) const;

    void buildObstacles();
    std::vector<Pixel> obstacles;

    void computeTotalSteps();

public:
    FloodFill(bool eightWay);

    // x1=centerX  y1=centerY  x2=halfS  y2=unused
    void init(int x1, int y1, int x2, int y2) override;
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

    bool isFillMode() const override { return true; }
    int  getHalfS()   const          { return halfS; }
};

class FloodFill4 : public FloodFill {
public:
    FloodFill4() : FloodFill(false) {}
    std::string getName() const override { return "Flood Fill (4-Way)"; }
};

class FloodFill8 : public FloodFill {
public:
    FloodFill8() : FloodFill(true) {}
    std::string getName() const override { return "Flood Fill (8-Way)"; }
};

// ============================================================
//  BoundaryFill (4-way and 8-way)
//  init(cx, cy, halfS, 0):
//    cx,cy  = fill seed AND centre of the square boundary
//    halfS  = half the side length
//  Key difference: stops when neighbour IS the boundary color,
//  not by a visited-set check.
// ============================================================
class BoundaryFill : public IAlgorithm {
protected:
    int seedX, seedY;
    int halfS;                  // dynamic square boundary half-side
    int currentStep, totalSteps;

    // BFS frontier queue and tracking sets
    std::queue<Pixel>            frontier;
    std::set<std::pair<int,int>> enqueued;   // prevents double-enqueue
    std::set<std::pair<int,int>> filled;     // already filled pixels

    std::vector<Pixel> path;       // filled pixels
    std::vector<Pixel> obstacles;  // boundary pixels rendered grey
    AlgoState          lastState;

    bool is8Way;

    bool isBoundary(int x, int y) const;
    void buildObstacles();
    void computeTotalSteps();

public:
    BoundaryFill(bool eightWay);

    // x1=centerX  y1=centerY  x2=halfS  y2=unused
    void init(int x1, int y1, int x2, int y2) override;
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

    bool isFillMode() const override { return true; }
    int  getHalfS()   const          { return halfS; }
};

class BoundaryFill4 : public BoundaryFill {
public:
    BoundaryFill4() : BoundaryFill(false) {}
    std::string getName() const override { return "Boundary Fill (4-Way)"; }
};

class BoundaryFill8 : public BoundaryFill {
public:
    BoundaryFill8() : BoundaryFill(true) {}
    std::string getName() const override { return "Boundary Fill (8-Way)"; }
};

// ============================================================
//  ScanlineFill — Active Edge Table scanline polygon fill
//  init(cx, cy, polySize, numSides):
//    cx, cy    = polygon centre
//    polySize  = circumscribed circle radius
//    numSides  = number of sides (3-10)
//  Works on regular convex polygons; processes one scanline per step.
// ============================================================
class ScanlineFill : public IAlgorithm {
private:
    struct SGEdge {
        int   yMax;
        float x;          // x-intercept at current scanline
        float invSlope;   // dx/dy
    };

    int cx, cy, polySize, numSides;

    std::vector<std::pair<float,float>>   vertices;
    std::map<int, std::vector<SGEdge>>    edgeTable;   // ET
    std::vector<SGEdge>                   activeEdges; // AET

    int  currentY, yMinPoly, yMaxPoly;
    int  currentStep, totalSteps;
    bool done;

    std::vector<Pixel> path;
    AlgoState          lastState;

    void generatePolygon();
    void buildEdgeTable();
    void doOneStep();          // process one scanline Y

public:
    ScanlineFill();

    // x1=cx  y1=cy  x2=polySize  y2=numSides (3-10)
    void init(int x1, int y1, int x2, int y2) override;
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

    bool isScanlineMode() const override { return true; }

    // Engine uses this to draw the polygon outline in renderGrid
    const std::vector<std::pair<float,float>>& getVertices() const { return vertices; }
};
