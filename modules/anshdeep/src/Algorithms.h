#pragma once

#include "Canvas.h"

// Math and drawing algorithms
class Algorithms {
public:
    // Bresenham's Line Drawing Algorithm (handles all 8 octants)
    static void DrawLineBresenham(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color);

    // Xiaolin Wu's anti-aliased line algorithm
    static void DrawLineWu(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color);

    // Midpoint Circle Algorithm
    static void DrawCircleMidpoint(Canvas& canvas, int xc, int yc, int radius, const Color& color, bool isFilled = false, const Color& fillColor = Color());

    // Ellipse Algorithm
    static void DrawEllipse(Canvas& canvas, int xc, int yc, int rx, int ry, const Color& color, bool isFilled = false, const Color& fillColor = Color());

    // Square Algorithm (drawn from start point to mouse point)
    static void DrawSquare(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color, bool isFilled = false, const Color& fillColor = Color());

    // Basic Flood Fill Algorithm
    static void FloodFill(Canvas& canvas, int x, int y, const Color& newColor, std::vector<std::pair<int, int>>* outPoints = nullptr);

    // SDF Rendering Helpers (for thickness and advanced AA)
    static void SetPixelAntiAliased(Canvas& canvas, int x, int y, const Color& color, float alpha);
    static float GetAlphaSDF(float distance, float thickness, AAType aaType);

    // SDF rendering primitives
    static void DrawLineSDF(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color, float thickness, AAType aaType);
    static void DrawCircleSDF(Canvas& canvas, int xc, int yc, int radius, const Color& color, bool isFilled, const Color& fillColor, float thickness, AAType aaType);
    static void DrawEllipseSDF(Canvas& canvas, int xc, int yc, int rx, int ry, const Color& color, bool isFilled, const Color& fillColor, float thickness, AAType aaType);
    static void DrawSquareSDF(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color, bool isFilled, const Color& fillColor, float thickness, AAType aaType);

    // Hit testing for shapes
    static int FindShapeAt(Canvas& canvas, int x, int y);

    // Dashed Bounding Box for selections
    static void DrawDashedBox(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color, int dashLength = 5);
};
