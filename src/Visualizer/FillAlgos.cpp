#include "../../include/Visualizer/FillAlgos.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

static std::string ei(int v)   { return std::to_string(v); }

// ================================================================
//  Shared helpers
// ================================================================

// Square boundary:  the four walls of [cx-h .. cx+h] x [cy-h .. cy+h]
static bool isSquareBoundary(int x, int y, int cx, int cy, int h) {
    int lo_x = cx - h, hi_x = cx + h;
    int lo_y = cy - h, hi_y = cy + h;
    if (x < lo_x || x > hi_x || y < lo_y || y > hi_y) return false; // out of box entirely
    return (x == lo_x || x == hi_x || y == lo_y || y == hi_y);
}

static bool isInsideBox(int x, int y, int cx, int cy, int h) {
    return (x > cx - h && x < cx + h && y > cy - h && y < cy + h);
}


// ================================================================
//  FloodFill
// ================================================================
FloodFill::FloodFill(bool eightWay)
    : is8Way(eightWay), seedX(10), seedY(10), halfS(8),
      currentStep(0), totalSteps(0) {
    buildObstacles();
}

bool FloodFill::isBoundary(int x, int y) const {
    return isSquareBoundary(x, y, seedX, seedY, halfS);
}

bool FloodFill::isValid(int x, int y) const {
    if (!isInsideBox(x, y, seedX, seedY, halfS)) return false;
    if (filledRaw.count({x, y})) return false;
    return true;
}

void FloodFill::buildObstacles() {
    obstacles.clear();
    int lo_x = seedX - halfS, hi_x = seedX + halfS;
    int lo_y = seedY - halfS, hi_y = seedY + halfS;
    for (int x = lo_x; x <= hi_x; ++x)
        for (int y = lo_y; y <= hi_y; ++y)
            if (isBoundary(x, y))
                obstacles.push_back({x, y, 1.0f, 3});
}

void FloodFill::computeTotalSteps() {
    std::queue<std::pair<int,int>> q;
    std::set<std::pair<int,int>>   vis;

    if (isInsideBox(seedX, seedY, seedX, seedY, halfS) || halfS > 0) {
        // seed is the center — always interior
        q.push({seedX, seedY});
        vis.insert({seedX, seedY});
    }

    totalSteps = 0;
    int dx[] = {1, 0, -1, 0,  1, 1, -1, -1};
    int dy[] = {0, 1,  0,-1, -1, 1, -1,  1};
    int cnt  = is8Way ? 8 : 4;

    while (!q.empty()) {
        auto [px, py] = q.front(); q.pop();
        totalSteps++;
        for (int i = 0; i < cnt; ++i) {
            int nx = px + dx[i], ny = py + dy[i];
            if (isInsideBox(nx, ny, seedX, seedY, halfS) && !vis.count({nx, ny})) {
                vis.insert({nx, ny});
                q.push({nx, ny});
            }
        }
    }
}

void FloodFill::init(int x1, int y1, int x2, int y2) {
    (void)y2;
    seedX = x1;
    seedY = y1;
    halfS = (x2 > 1) ? x2 : 2;   // enforce minimum halfS=2

    while (!frontier.empty()) frontier.pop();
    filledRaw.clear();
    path.clear();
    currentStep = 0;

    buildObstacles();
    computeTotalSteps();

    // Seed is the center — always interior
    frontier.push({seedX, seedY, 1.0f, 1});
    filledRaw.insert({seedX, seedY});

    lastState = AlgoState();
    lastState.totalSteps    = totalSteps;
    lastState.pixelsPerStep = 1;
}

void FloodFill::step() {
    if (isFinished()) return;

    Pixel p = frontier.front();
    frontier.pop();

    path.push_back({p.x, p.y, 1.0f, 2});   // filled = green
    currentStep++;

    int dx[] = {1, 0, -1, 0,  1, 1, -1, -1};
    int dy[] = {0, 1,  0,-1, -1, 1, -1,  1};
    int cnt  = is8Way ? 8 : 4;

    int enqueued = 0;
    for (int i = 0; i < cnt; ++i) {
        int nx = p.x + dx[i];
        int ny = p.y + dy[i];
        if (isValid(nx, ny)) {
            frontier.push({nx, ny, 1.0f, 1});
            filledRaw.insert({nx, ny});
            enqueued++;
        }
    }

    lastState.hasCalculation = true;
    lastState.currentStep    = currentStep;
    lastState.calcLines.clear();
    lastState.calcLines.push_back("--- Step " + ei(currentStep) + " / " + ei(totalSteps) + " ---");
    lastState.calcLines.push_back("  Popped: (" + ei(p.x) + ", " + ei(p.y) + ") -> FILLED");
    lastState.calcLines.push_back("  Boundary: x in [" + ei(seedX-halfS) + ".." + ei(seedX+halfS)
                                    + "]  y in [" + ei(seedY-halfS) + ".." + ei(seedY+halfS) + "]");
    lastState.calcLines.push_back("  Enqueued " + ei(enqueued) + " new neighbour(s).");
    lastState.calcLines.push_back("  Frontier Queue Size: " + ei((int)frontier.size()));
    lastState.calcLines.push_back("");
    if (isFinished())
        lastState.calcLines.push_back("Queue empty — Flood Fill complete!");
    else
        lastState.calcLines.push_back("Next pixel: (" + ei(frontier.front().x) + ", " + ei(frontier.front().y) + ")");
}

void FloodFill::stepK(int k) {
    int dx[] = {1, 0, -1, 0,  1, 1, -1, -1};
    int dy[] = {0, 1,  0,-1, -1, 1, -1,  1};
    int cnt  = is8Way ? 8 : 4;

    for (int i = 0; i < k && !isFinished(); ++i) {
        Pixel p = frontier.front(); frontier.pop();
        path.push_back({p.x, p.y, 1.0f, 2});
        currentStep++;
        for (int j = 0; j < cnt; ++j) {
            int nx = p.x + dx[j], ny = p.y + dy[j];
            if (isValid(nx, ny)) {
                frontier.push({nx, ny, 1.0f, 1});
                filledRaw.insert({nx, ny});
            }
        }
    }
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.calcLines.clear();
    lastState.calcLines.push_back("Stepped " + ei(k) + " silently.  Queue: " + ei((int)frontier.size()));
}

void FloodFill::runToCompletion() {
    while (!isFinished()) stepK(100);
}

void FloodFill::reset() { init(seedX, seedY, halfS, 0); }

bool FloodFill::isFinished() const { return frontier.empty(); }

std::vector<Pixel> FloodFill::getHighlightedPixels() const {
    std::vector<Pixel> full = obstacles;
    full.insert(full.end(), path.begin(), path.end());
    std::queue<Pixel> copy = frontier;
    while (!copy.empty()) { full.push_back(copy.front()); copy.pop(); }
    return full;
}

AlgoState FloodFill::getCurrentState() const { return lastState; }

std::vector<std::string> FloodFill::getInitInfo() const {
    return {
        "Algorithm : " + getName(),
        "Center (seed): (" + ei(seedX) + ", " + ei(seedY) + ")",
        "Half-side  : " + ei(halfS) + "  (side = " + ei(2*halfS) + ")",
        "Boundary box: x[" + ei(seedX-halfS) + ".." + ei(seedX+halfS) + "]"
                       " y[" + ei(seedY-halfS) + ".." + ei(seedY+halfS) + "]",
        "Region pixels: " + ei(totalSteps),
        "Queue: BFS  |  Mode: " + std::string(is8Way ? "8-Way" : "4-Way")
    };
}

std::vector<std::string> FloodFill::getCurrentVars() const {
    return {
        "Queue Size : " + ei((int)frontier.size()),
        "Filled     : " + ei(currentStep),
        "Total Goal : " + ei(totalSteps),
        "Half-side  : " + ei(halfS)
    };
}

std::string FloodFill::getTheory() const {
    return
        getName() + "\n"
        "==========================\n\n"
        "WHAT IS IT?\n"
        "  Flood fill (also called seed fill) is an algorithm\n"
        "  that determines the area connected to a given node\n"
        "  in a multi-dimensional array.\n\n"
        "  Commonly used in 'bucket fill' tools in paint\n"
        "  programs to fill connected, similarly-colored areas.\n\n"
        "THIS VISUALISER:\n"
        "  Seed X, Seed Y  = center of the square boundary\n"
        "                    AND the starting fill point.\n"
        "  Half-side S     = boundary walls placed at\n"
        "                    cx ± S, cy ± S.\n"
        "  Change S to resize the square on-the-fly.\n\n"
        "HOW IT WORKS:\n"
        "  1. Push seed pixel into a Queue.\n"
        "  2. Pop front pixel, fill it (mark as 'filled').\n"
        "  3. Check its 4 (or 8) neighbors:\n"
        "       - Is it INSIDE the box? AND not yet visited?\n"
        "       -> Push to Queue, mark visited.\n"
        "  4. Repeat until Queue is empty.\n\n"
        "4-WAY VS 8-WAY:\n"
        "  4-way scans: Up / Down / Left / Right.\n"
        "  8-way scans: all 4 above + the 4 diagonals.\n\n"
        "  For a plain square, both fill identically.\n"
        "  The difference shows when there are diagonal gaps\n"
        "  in the boundary (like connected corners touching).\n\n"
        "TIME COMPLEXITY:  O(N), N = number of interior pixels\n"
        "SPACE COMPLEXITY: O(N) for the queue\n";
}

std::string FloodFill::getName() const { return "Flood Fill"; }


// ================================================================
//  BoundaryFill
// ================================================================
BoundaryFill::BoundaryFill(bool eightWay)
    : is8Way(eightWay), seedX(10), seedY(10), halfS(8),
      currentStep(0), totalSteps(0) {
    buildObstacles();
}

bool BoundaryFill::isBoundary(int x, int y) const {
    return isSquareBoundary(x, y, seedX, seedY, halfS);
}

void BoundaryFill::buildObstacles() {
    obstacles.clear();
    int lo_x = seedX - halfS, hi_x = seedX + halfS;
    int lo_y = seedY - halfS, hi_y = seedY + halfS;
    for (int x = lo_x; x <= hi_x; ++x)
        for (int y = lo_y; y <= hi_y; ++y)
            if (isBoundary(x, y))
                obstacles.push_back({x, y, 1.0f, 3});
}

void BoundaryFill::computeTotalSteps() {
    std::queue<std::pair<int,int>> q;
    std::set<std::pair<int,int>>   vis;

    if (halfS > 0) {
        q.push({seedX, seedY});
        vis.insert({seedX, seedY});
    }
    totalSteps = 0;
    int dx[] = {1, 0, -1, 0,  1, 1, -1, -1};
    int dy[] = {0, 1,  0,-1, -1, 1, -1,  1};
    int cnt  = is8Way ? 8 : 4;

    while (!q.empty()) {
        auto [px, py] = q.front(); q.pop();
        totalSteps++;
        for (int i = 0; i < cnt; ++i) {
            int nx = px + dx[i], ny = py + dy[i];
            if (!isBoundary(nx, ny) && isInsideBox(nx, ny, seedX, seedY, halfS)
                && !vis.count({nx, ny})) {
                vis.insert({nx, ny});
                q.push({nx, ny});
            }
        }
    }
}

void BoundaryFill::init(int x1, int y1, int x2, int y2) {
    (void)y2;
    seedX = x1;
    seedY = y1;
    halfS = (x2 > 1) ? x2 : 2;

    while (!frontier.empty()) frontier.pop();
    enqueued.clear();
    filled.clear();
    path.clear();
    currentStep = 0;

    buildObstacles();
    computeTotalSteps();

    // Seed = center, always interior
    frontier.push({seedX, seedY, 1.0f, 1});
    enqueued.insert({seedX, seedY});

    lastState = AlgoState();
    lastState.totalSteps    = totalSteps;
    lastState.pixelsPerStep = 1;
}

void BoundaryFill::step() {
    if (isFinished()) return;

    Pixel p = frontier.front();
    frontier.pop();

    filled.insert({p.x, p.y});
    path.push_back({p.x, p.y, 1.0f, 2});
    currentStep++;

    int dx[] = {1, 0, -1, 0,  1,  1, -1, -1};
    int dy[] = {0, 1,  0,-1, -1,  1, -1,  1};
    int cnt  = is8Way ? 8 : 4;

    int enqCount     = 0;
    int boundaryHits = 0;
    int alreadyFill  = 0;

    std::vector<std::string> nbLog;

    for (int i = 0; i < cnt; ++i) {
        int nx = p.x + dx[i];
        int ny = p.y + dy[i];

        if (isBoundary(nx, ny)) {
            nbLog.push_back("  (" + ei(nx) + "," + ei(ny) + ")  BOUNDARY COLOR  -- stop");
            boundaryHits++;
        } else if (filled.count({nx, ny}) || enqueued.count({nx, ny})) {
            nbLog.push_back("  (" + ei(nx) + "," + ei(ny) + ")  already filled  -- skip");
            alreadyFill++;
        } else if (isInsideBox(nx, ny, seedX, seedY, halfS)) {
            frontier.push({nx, ny, 1.0f, 1});
            enqueued.insert({nx, ny});
            nbLog.push_back("  (" + ei(nx) + "," + ei(ny) + ")  interior        -- ENQUEUE");
            enqCount++;
        } else {
            nbLog.push_back("  (" + ei(nx) + "," + ei(ny) + ")  out of box      -- skip");
        }
    }

    lastState.hasCalculation = true;
    lastState.currentStep    = currentStep;
    lastState.calcLines.clear();
    lastState.calcLines.push_back("--- Step " + ei(currentStep) + " / " + ei(totalSteps) + " ---");
    lastState.calcLines.push_back("  Popped: (" + ei(p.x) + ", " + ei(p.y) + ")");
    lastState.calcLines.push_back("  NOT boundary color -> FILL IT");
    lastState.calcLines.push_back("  Boundary box: x[" + ei(seedX-halfS) + ".." + ei(seedX+halfS)
                                    + "] y[" + ei(seedY-halfS) + ".." + ei(seedY+halfS) + "]");
    lastState.calcLines.push_back("");
    lastState.calcLines.push_back("--- Neighbour Boundary-Color Checks ---");
    for (auto& s : nbLog)
        lastState.calcLines.push_back(s);
    lastState.calcLines.push_back("");
    lastState.calcLines.push_back("  Enqueued : " + ei(enqCount));
    lastState.calcLines.push_back("  Boundary : " + ei(boundaryHits) + "  (boundary color -> blocked)");
    lastState.calcLines.push_back("  Already  : " + ei(alreadyFill));
    lastState.calcLines.push_back("  Queue size: " + ei((int)frontier.size()));
    if (isFinished())
        lastState.calcLines.push_back("  Queue empty -> Boundary Fill DONE!");
    else
        lastState.calcLines.push_back("  Next: (" + ei(frontier.front().x) + "," + ei(frontier.front().y) + ")");
}

void BoundaryFill::stepK(int k) {
    int dx[] = {1, 0, -1, 0,  1,  1, -1, -1};
    int dy[] = {0, 1,  0,-1, -1,  1, -1,  1};
    int cnt  = is8Way ? 8 : 4;

    for (int i = 0; i < k && !isFinished(); ++i) {
        Pixel p = frontier.front(); frontier.pop();
        filled.insert({p.x, p.y});
        path.push_back({p.x, p.y, 1.0f, 2});
        currentStep++;
        for (int j = 0; j < cnt; ++j) {
            int nx = p.x + dx[j], ny = p.y + dy[j];
            if (!isBoundary(nx, ny) && isInsideBox(nx, ny, seedX, seedY, halfS)
                && !filled.count({nx, ny}) && !enqueued.count({nx, ny})) {
                frontier.push({nx, ny, 1.0f, 1});
                enqueued.insert({nx, ny});
            }
        }
    }
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.calcLines.clear();
    lastState.calcLines.push_back("Stepped " + ei(k) + " silently.  Queue: " + ei((int)frontier.size()));
}

void BoundaryFill::runToCompletion() { while (!isFinished()) stepK(100); }

void BoundaryFill::reset() { init(seedX, seedY, halfS, 0); }

bool BoundaryFill::isFinished() const { return frontier.empty(); }

std::vector<Pixel> BoundaryFill::getHighlightedPixels() const {
    std::vector<Pixel> full = obstacles;
    full.insert(full.end(), path.begin(), path.end());
    std::queue<Pixel> copy = frontier;
    while (!copy.empty()) { full.push_back(copy.front()); copy.pop(); }
    return full;
}

AlgoState BoundaryFill::getCurrentState() const { return lastState; }

std::vector<std::string> BoundaryFill::getInitInfo() const {
    return {
        "Algorithm : " + getName(),
        "Center (seed): (" + ei(seedX) + ", " + ei(seedY) + ")",
        "Half-side  : " + ei(halfS) + "  (side = " + ei(2*halfS) + ")",
        "Boundary box: x[" + ei(seedX-halfS) + ".." + ei(seedX+halfS) + "]"
                       " y[" + ei(seedY-halfS) + ".." + ei(seedY+halfS) + "]",
        "Region pixels: " + ei(totalSteps),
        "Stop rule  : neighbour IS boundary color -> block",
        "Mode: " + std::string(is8Way ? "8-Way" : "4-Way")
    };
}

std::vector<std::string> BoundaryFill::getCurrentVars() const {
    return {
        "Queue Size : " + ei((int)frontier.size()),
        "Filled     : " + ei(currentStep),
        "Total Goal : " + ei(totalSteps),
        "Half-side  : " + ei(halfS)
    };
}

std::string BoundaryFill::getTheory() const {
    return
        getName() + "\n"
        "==========================\n\n"
        "WHAT IS IT?\n"
        "  Boundary Fill fills an enclosed region starting from\n"
        "  a seed point, expanding outward until it hits pixels\n"
        "  of a specified BOUNDARY COLOR.\n\n"
        "THIS VISUALISER:\n"
        "  Seed X, Seed Y  = center of the square boundary\n"
        "                    AND the starting fill point.\n"
        "  Half-side S     = boundary walls placed at\n"
        "                    cx ± S, cy ± S.\n"
        "  Change S to resize the square on-the-fly.\n\n"
        "HOW IT WORKS (BFS version):\n"
        "  1. Push seed pixel into Queue.\n"
        "  2. Pop pixel, fill it.\n"
        "  3. For each neighbour:\n"
        "       - Is it BOUNDARY COLOR (grey)? -> skip.\n"
        "       - Is it already FILLED?        -> skip.\n"
        "       - Otherwise                    -> enqueue.\n"
        "  4. Repeat until Queue is empty.\n\n"
        "KEY DIFFERENCE FROM FLOOD FILL:\n"
        "  Flood Fill   tests: 'was this pixel visited?'\n"
        "  Boundary Fill tests: 'is this pixel boundary color?'\n\n"
        "  Same result for a closed region, but Boundary Fill\n"
        "  is closer to real raster paint tools: it naturally\n"
        "  halts at any drawn border regardless of shape.\n\n"
        "4-WAY VS 8-WAY:\n"
        "  4-way: cannot cross a single diagonal gap.\n"
        "  8-way: leaks through single-pixel diagonal gaps.\n\n"
        "TIME:  O(N)  SPACE: O(N)  where N = interior pixels\n";
}

std::string BoundaryFill::getName() const { return "Boundary Fill"; }

// ================================================================
//  ScanlineFill
// ================================================================
static const float PI_F = 3.14159265f;

ScanlineFill::ScanlineFill()
    : cx(10), cy(10), polySize(7), numSides(5),
      currentY(0), yMinPoly(0), yMaxPoly(0),
      currentStep(0), totalSteps(0), done(false) {}

void ScanlineFill::generatePolygon() {
    vertices.clear();
    float angleStep = 2.0f * PI_F / (float)numSides;
    float startAngle = -PI_F / 2.0f;      // first vertex at top
    for (int i = 0; i < numSides; i++) {
        float a = startAngle + (float)i * angleStep;
        vertices.push_back(std::make_pair(
            (float)cx + (float)polySize * cosf(a),
            (float)cy + (float)polySize * sinf(a)
        ));
    }
}

void ScanlineFill::buildEdgeTable() {
    edgeTable.clear();
    activeEdges.clear();
    yMinPoly = 1 << 29;
    yMaxPoly = -(1 << 29);

    int n = (int)vertices.size();
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        float fx1 = vertices[i].first,  fy1 = vertices[i].second;
        float fx2 = vertices[j].first,  fy2 = vertices[j].second;
        int   iy1 = (int)roundf(fy1),   iy2 = (int)roundf(fy2);

        if (iy1 == iy2) continue;          // skip horizontal edges

        if (iy1 > iy2) {                   // ensure iy1 < iy2
            std::swap(iy1, iy2);
            std::swap(fx1, fx2);
        }

        SGEdge e;
        e.yMax     = iy2;
        e.x        = fx1;
        e.invSlope = (fx2 - fx1) / (float)(iy2 - iy1);

        edgeTable[iy1].push_back(e);
        yMinPoly = std::min(yMinPoly, iy1);
        yMaxPoly = std::max(yMaxPoly, iy2);
    }
    if (yMinPoly > yMaxPoly) { yMinPoly = cy; yMaxPoly = cy; }
    totalSteps = yMaxPoly - yMinPoly;
}

void ScanlineFill::doOneStep() {
    if (done) return;

    // 1. Add edges from ET[currentY] to AET
    int newCount = 0;
    if (edgeTable.count(currentY)) {
        for (auto& e : edgeTable.at(currentY)) {
            activeEdges.push_back(e);
            newCount++;
        }
    }

    // 2. Remove edges whose yMax <= currentY (not active on this scanline)
    activeEdges.erase(
        std::remove_if(activeEdges.begin(), activeEdges.end(),
            [this](const SGEdge& e){ return e.yMax <= currentY; }),
        activeEdges.end());

    // 3. Sort AET by x (ensures correct pairing)
    std::sort(activeEdges.begin(), activeEdges.end(),
        [](const SGEdge& a, const SGEdge& b){ return a.x < b.x; });

    // 4. Fill horizontal spans (even-odd pairwise rule)
    std::vector<std::pair<int,int>> spans;
    int filled = 0;
    for (int i = 0; i + 1 < (int)activeEdges.size(); i += 2) {
        int xL = (int)ceilf(activeEdges[i].x);
        int xR = (int)floorf(activeEdges[i+1].x);
        if (xL <= xR) {
            spans.push_back({xL, xR});
            for (int x = xL; x <= xR; x++)
                path.push_back({x, currentY, 1.0f, 2});
            filled += xR - xL + 1;
        }
    }

    // 5. Build calc record (capture x BEFORE update)
    std::vector<float> xBefore;
    for (auto& e : activeEdges) xBefore.push_back(e.x);

    // 6. Update x += invSlope for next scanline
    for (auto& e : activeEdges) e.x += e.invSlope;

    // ---- calc panel ----
    lastState.hasCalculation = true;
    lastState.currentStep    = currentStep + 1;
    lastState.totalSteps     = totalSteps;
    lastState.pixelsPerStep  = filled;
    lastState.calcLines.clear();

    lastState.calcLines.push_back("--- Scanline Y = " + ei(currentY)
        + "  (step " + ei(currentStep+1) + "/" + ei(totalSteps) + ") ---");
    lastState.calcLines.push_back("");
    lastState.calcLines.push_back("1) Added from ET[" + ei(currentY) + "]:  "
        + ei(newCount) + " edge(s)");
    lastState.calcLines.push_back("2) AET size (after removal+sort): "
        + ei((int)activeEdges.size()) + " active edge(s)");
    lastState.calcLines.push_back("");

    for (int i = 0; i < (int)activeEdges.size(); i++) {
        std::ostringstream os;
        os << std::fixed << std::setprecision(2);
        os << "   [" << i << "] x=" << xBefore[i]
           << "  yMax=" << activeEdges[i].yMax
           << "  1/slope=" << activeEdges[i].invSlope;
        lastState.calcLines.push_back(os.str());
    }

    lastState.calcLines.push_back("");
    lastState.calcLines.push_back("3) Fill pairs (even-odd rule):");
    if (spans.empty()) {
        lastState.calcLines.push_back("   (No spans — AET empty or odd count)");
    } else {
        for (auto& sp : spans)
            lastState.calcLines.push_back("   x = [" + ei(sp.first) + " .. " + ei(sp.second)
                + "]  -> " + ei(sp.second - sp.first + 1) + " pixels");
    }
    lastState.calcLines.push_back("   Total pixels this scanline: " + ei(filled));
    lastState.calcLines.push_back("");
    lastState.calcLines.push_back("4) Update x += (1/slope) for Y=" + ei(currentY+1) + ":");
    for (int i = 0; i < (int)activeEdges.size(); i++) {
        std::ostringstream os;
        os << std::fixed << std::setprecision(2);
        os << "   [" << i << "] " << xBefore[i]
           << " + " << activeEdges[i].invSlope
           << " = " << activeEdges[i].x;
        lastState.calcLines.push_back(os.str());
    }

    // 7. Advance
    currentStep++;
    currentY++;
    if (currentY > yMaxPoly) {
        done = true;
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("currentY > yMax -> Algorithm COMPLETE!");
    }
}

void ScanlineFill::init(int x1, int y1, int x2, int y2) {
    cx       = x1;
    cy       = y1;
    polySize = std::max(x2, 2);
    numSides = std::max(3, std::min(y2, 10));

    path.clear();
    done        = false;
    currentStep = 0;

    generatePolygon();
    buildEdgeTable();
    currentY = yMinPoly;

    lastState = AlgoState();
    lastState.totalSteps    = totalSteps;
    lastState.pixelsPerStep = 0;
}

void ScanlineFill::step() {
    if (isFinished()) return;
    doOneStep();
}

void ScanlineFill::stepK(int k) {
    for (int i = 0; i < k && !isFinished(); i++) doOneStep();
    lastState.hasCalculation = false;
    lastState.calcLines.clear();
    lastState.calcLines.push_back("Stepped " + ei(k) + " scanlines.  Y now: " + ei(currentY));
}

void ScanlineFill::runToCompletion() {
    while (!isFinished()) doOneStep();
    lastState.hasCalculation = false;
    lastState.calcLines.clear();
}

void ScanlineFill::reset() { init(cx, cy, polySize, numSides); }

bool ScanlineFill::isFinished() const { return done; }

std::vector<Pixel> ScanlineFill::getHighlightedPixels() const { return path; }

AlgoState ScanlineFill::getCurrentState() const { return lastState; }

std::vector<std::string> ScanlineFill::getInitInfo() const {
    std::vector<std::string> info;
    info.push_back("Algorithm : " + getName());
    info.push_back("Polygon   : Regular " + ei(numSides) + "-gon");
    info.push_back("Centre    : (" + ei(cx) + ", " + ei(cy) + ")");
    info.push_back("Radius    : " + ei(polySize));
    info.push_back("Scanlines : Y = [" + ei(yMinPoly) + " .. " + ei(yMaxPoly) + "]");
    info.push_back("Steps     : " + ei(totalSteps) + " scanlines");
    info.push_back("ET entries: " + ei((int)edgeTable.size()) + " Y-bucket(s)");
    return info;
}

std::vector<std::string> ScanlineFill::getCurrentVars() const {
    return {
        "Current Y  : " + ei(currentY),
        "AET size   : " + ei((int)activeEdges.size()),
        "Step       : " + ei(currentStep) + " / " + ei(totalSteps),
        "Total px   : " + ei((int)path.size())
    };
}

std::string ScanlineFill::getTheory() const {
    return
        "Scanline Polygon Fill\n"
        "=====================\n\n"
        "WHAT IS IT?\n"
        "  Fills any polygon by processing horizontal scan-\n"
        "  lines from bottom to top. Uses an Edge Table (ET)\n"
        "  and an Active Edge Table (AET) to find which x-\n"
        "  ranges to fill at each Y.\n\n"
        "DATA STRUCTURES:\n"
        "  Edge Table (ET):\n"
        "    A list of edges grouped by their LOWER y-coord.\n"
        "    Each entry stores: (yMax, xAtYmin, 1/slope).\n"
        "    Horizontal edges are skipped.\n\n"
        "  Active Edge Table (AET):\n"
        "    Edges currently intersecting the scan line.\n"
        "    Sorted by x at every scanline.\n\n"
        "ALGORITHM (per scanline Y):\n"
        "  1. Move edges from ET[Y] into AET.\n"
        "  2. Remove edges from AET where yMax <= Y.\n"
        "  3. Sort AET by current x.\n"
        "  4. Fill between pairs: (AET[0].x, AET[1].x),\n"
        "     (AET[2].x, AET[3].x), etc. (even-odd rule).\n"
        "  5. Update x += 1/slope for each AET edge.\n"
        "  6. Advance Y.\n\n"
        "EVEN-ODD RULE:\n"
        "  A point is INSIDE the polygon if a ray from it\n"
        "  crosses the polygon boundary an ODD number of\n"
        "  times. The pairwise AET fill implements this:\n"
        "  outside -> inside (at AET[0]), inside -> outside\n"
        "  (at AET[1]), etc.\n\n"
        "WHY 1/slope?\n"
        "  Instead of recalculating x = x0 + (dy * dx/dy),\n"
        "  we just add dx/dy (= 1/slope) at each scanline.\n"
        "  This is the same incremental idea as Bresenham.\n\n"
        "COMPLEXITY:\n"
        "  O(N*E) where N = scanlines, E = active edges.\n"
        "  For a regular polygon E is always small (2-N).\n";
}

std::string ScanlineFill::getName() const { return "Scanline Fill"; }
