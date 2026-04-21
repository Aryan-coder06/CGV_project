#pragma once
#include "IAlgorithm.h"
#include <cmath>

// ================================================================
//  Transform2D — 2D Transformations: Translation, Rotation, Scaling
//  Applied step-by-step to a regular polygon.
//  Step 1 = Translation, Step 2 = Rotation, Step 3 = Scaling
// ================================================================
class Transform2D : public IAlgorithm {
public:
  struct Vertex { float x, y; };

private:
  // Polygon definition
  int oCX, oCY, oSize, oSides;

  // Transform parameters
  float tx, ty;           // Translation
  float rotAngleDeg;      // Rotation angle in degrees
  float pivotX, pivotY;   // Pivot point for rotation & scaling
  float sx, sy;           // Scale factors

  // phases[0]=original, [1]=after translate, [2]=after rotate, [3]=after scale
  std::vector<Vertex> phases[4];

  int  currentPhase; // 0..3
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

  void doTranslation();
  void doRotation();
  void doScaling();

public:
  Transform2D();

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

  bool isTransformMode() const override { return true; }

  // Setters for transform parameters
  void setTranslation(float dx, float dy)           { tx = dx; ty = dy; }
  void setRotation(float angleDeg, float px, float py) {
    rotAngleDeg = angleDeg; pivotX = px; pivotY = py;
  }
  void setScaling(float scx, float scy, float px, float py) {
    sx = scx; sy = scy; pivotX = px; pivotY = py;
  }

  // Expose polygons for filled rendering
  const std::vector<Vertex>& getOriginalPoly()  const { return phases[0]; }
  const std::vector<Vertex>& getCurrentPoly()   const { return phases[currentPhase]; }
  const std::vector<Vertex>& getPhase(int i)    const { return phases[i]; }
  int getCurrentPhase() const { return currentPhase; }

  float getTx()  const { return tx; }
  float getTy()  const { return ty; }
  float getAngle() const { return rotAngleDeg; }
  float getSx()  const { return sx; }
  float getSy()  const { return sy; }
};
