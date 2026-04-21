#pragma once
#include "IAlgorithm.h"

// ================================================================
//  WindowViewport — Window-to-Viewport Transformation
//
//  Maps a polygon defined in "world" (window) coordinates to
//  a screen (viewport) coordinate system using the formulas:
//
//    sx = (vxMax - vxMin) / (wxMax - wxMin)
//    sy = (vyMax - vyMin) / (wyMax - wyMin)
//    xv = vxMin + (xw - wxMin) * sx
//    yv = vyMin + (yw - wyMin) * sy
//
//  Steps:
//    Step 1: Compute scale factors sx, sy
//    Step 2: Apply mapping to all vertices
// ================================================================
class WindowViewport : public IAlgorithm {
public:
  struct Vertex { float x, y; };

private:
  // World window bounds
  float wxMin, wxMax, wyMin, wyMax;

  // Viewport (screen) bounds
  float vxMin, vxMax, vyMin, vyMax;

  // Polygon in world coords (original)
  int oCX, oCY, oSize, oSides;

  std::vector<Vertex> worldPoly;    // original vertices in world coords
  std::vector<Vertex> viewPoly;     // mapped vertices in viewport coords

  // Computed scale factors
  float sx, sy;

  int  currentPhase; // 0=initial, 1=scale computed, 2=full mapping done
  bool finished;

  AlgoState lastState;
  std::vector<Pixel> displayPixels;

  // Helpers
  std::vector<Vertex> buildPolygon(int cx, int cy, int size, int sides) const;
  void rasteriseLine(float ax, float ay, float bx, float by,
                     std::vector<Pixel>& out, int pixType) const;
  void rasterisePoly(const std::vector<Vertex>& poly,
                     std::vector<Pixel>& out, int pixType) const;
  void buildDisplay();

public:
  WindowViewport();

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

  bool isWindowViewportMode() const override { return true; }

  // Setters called by the engine before init()
  void setWorldWindow(float x0, float x1, float y0, float y1) {
    wxMin = x0; wxMax = x1; wyMin = y0; wyMax = y1;
  }
  void setViewport(float x0, float x1, float y0, float y1) {
    vxMin = x0; vxMax = x1; vyMin = y0; vyMax = y1;
  }

  // Expose for rendering
  float getWxMin() const { return wxMin; } float getWxMax() const { return wxMax; }
  float getWyMin() const { return wyMin; } float getWyMax() const { return wyMax; }
  float getVxMin() const { return vxMin; } float getVxMax() const { return vxMax; }
  float getVyMin() const { return vyMin; } float getVyMax() const { return vyMax; }
  float getSx()    const { return sx; }
  float getSy()    const { return sy; }

  const std::vector<Vertex>& getWorldPoly()    const { return worldPoly; }
  const std::vector<Vertex>& getViewportPoly() const { return viewPoly; }
  int   getCurrentPhase() const { return currentPhase; }
};
