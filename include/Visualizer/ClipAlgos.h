#pragma once
#include "IAlgorithm.h"

// Cohen-Sutherland Line Clipping
// Inputs: line endpoints (x1,y1)-(x2,y2); clip window is fixed or user-defined.
// We re-use the init(x1,y1,x2,y2) signature for the line endpoints.
// The clip rectangle is embedded as member variables with sensible defaults.

class CohenSutherland : public IAlgorithm {
public:
  // Outcode bit-flags
  static constexpr int INSIDE = 0;
  static constexpr int LEFT   = 1;
  static constexpr int RIGHT  = 2;
  static constexpr int BOTTOM = 4;
  static constexpr int TOP    = 8;

private:
  // Line endpoints (original and working copies)
  float ox1, oy1, ox2, oy2;   // originals (for reset)
  float x1, y1, x2, y2;       // working endpoints

  // Clip rectangle
  float xMin, xMax, yMin, yMax;

  // Algorithm state
  int  outcode1, outcode2;
  bool accepted;
  bool rejected;
  bool finished;
  int  currentStep;

  // Pixel path:
  //   type 0 = discarded segment pixel (grey)
  //   type 1 = original line pixel     (orange)
  //   type 2 = clipped / accepted pixel(green)
  //   type 3 = boundary pixel          (grey, reused for clip-rect corners)
  std::vector<Pixel> originalPixels;   // full original line rasterised
  std::vector<Pixel> clippedPixels;    // accepted (clipped) segment
  std::vector<Pixel> displayPixels;    // union displayed on grid

  AlgoState lastState;

  // Helpers
  int  computeOutcode(float x, float y) const;
  void rasteriseLine(float ax, float ay, float bx, float by,
                     std::vector<Pixel>& out, int pixType) const;
  void buildDisplay();
  void doClipStep();      // one Cohen-Sutherland iteration

public:
  CohenSutherland();

  void init(int lx1, int ly1, int lx2, int ly2) override;
  void step()            override;
  void stepK(int k)      override;
  void runToCompletion() override;
  void reset()           override;
  bool isFinished() const override;

  std::vector<Pixel>             getHighlightedPixels() const override;
  AlgoState                      getCurrentState()      const override;
  std::vector<std::string>       getInitInfo()          const override;
  std::vector<std::string>       getCurrentVars()       const override;
  std::string                    getTheory()            const override;
  std::string                    getName()              const override;

  bool isClipMode() const override { return true; }

  // Expose clip rectangle for the UI
  float getXMin() const { return xMin; }
  float getXMax() const { return xMax; }
  float getYMin() const { return yMin; }
  float getYMax() const { return yMax; }

  void setClipWindow(int x0, int x1, int y0, int y1) {
    xMin = (float)x0; xMax = (float)x1;
    yMin = (float)y0; yMax = (float)y1;
  }
};

// ================================================================
//  Sutherland-Hodgman Polygon Clipping
//  Clips a convex/concave polygon against a rectangular window.
//  Each of the 4 edges is clipped in sequence — one step per edge.
// ================================================================
class SutherlandHodgman : public IAlgorithm {
public:
  struct Vertex { float x, y; };

  static constexpr int CLIP_LEFT   = 0;
  static constexpr int CLIP_RIGHT  = 1;
  static constexpr int CLIP_BOTTOM = 2;
  static constexpr int CLIP_TOP    = 3;

private:
  // Original polygon parameters (for reset)
  int oCX, oCY, oSize, oSides;

  // Clip rectangle
  float xMin, xMax, yMin, yMax;

  // Polygon states: phases[0]=original, phases[1..4]=after each edge clip
  std::vector<Vertex> phases[5];

  int  currentPhase; // 0 = initial, 4 = done
  bool finished;

  // Display
  std::vector<Pixel> displayPixels;
  AlgoState lastState;

  // Helpers
  std::vector<Vertex> buildRegularPolygon(int cx, int cy, int size, int sides) const;
  std::vector<Vertex> clipAgainstEdge(const std::vector<Vertex>& poly, int edge) const;
  bool     isInside(const Vertex& p, int edge) const;
  Vertex   intersect(const Vertex& a, const Vertex& b, int edge) const;
  void     rasterisePoly(const std::vector<Vertex>& poly,
                         std::vector<Pixel>& out, int pixType) const;
  void     rasteriseLine(float ax, float ay, float bx, float by,
                         std::vector<Pixel>& out, int pixType) const;
  void     buildDisplay();

public:
  SutherlandHodgman();

  // init reuses (x1,y1,x2,y2) as (cx, cy, size, sides)
  void init(int cx, int cy, int size, int sides) override;
  void step()            override;
  void stepK(int k)      override;
  void runToCompletion() override;
  void reset()           override;
  bool isFinished() const override;

  std::vector<Pixel>       getHighlightedPixels() const override;
  AlgoState                getCurrentState()      const override;
  std::vector<std::string> getInitInfo()          const override;
  std::vector<std::string> getCurrentVars()       const override;
  std::string              getTheory()            const override;
  std::string              getName()              const override;

  bool isPolygonClipMode() const override { return true; }

  void setClipWindow(int x0, int x1, int y0, int y1) {
    xMin = (float)x0; xMax = (float)x1;
    yMin = (float)y0; yMax = (float)y1;
  }
  float getXMin() const { return xMin; }
  float getXMax() const { return xMax; }
  float getYMin() const { return yMin; }
  float getYMax() const { return yMax; }

  // For grid rendering: expose polygons
  const std::vector<Vertex>& getOriginalPoly()  const { return phases[0]; }
  const std::vector<Vertex>& getCurrentPoly()   const { return phases[currentPhase]; }
  int getCurrentPhase() const { return currentPhase; }
};
