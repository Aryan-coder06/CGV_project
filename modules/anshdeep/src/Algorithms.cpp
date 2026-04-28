#include "Algorithms.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include <utility>

// ---------------------------------------------------------
// Helper for Wu's Line Algorithm
// ---------------------------------------------------------
inline float fractionalPart(float x) { return x - std::floor(x); }
inline float reverseFractionalPart(float x) { return 1.0f - fractionalPart(x); }

static void DrawPixelAntiAliased(Canvas& canvas, int x, int y, const Color& color, float intensity) {
    if (x < 0 || x >= canvas.GetWidth() || y < 0 || y >= canvas.GetHeight()) return;

    Color bg = canvas.GetPixel(x, y);

    // Alpha blending
    uint8_t r = static_cast<uint8_t>(color.r * intensity + bg.r * (1.0f - intensity));
    uint8_t g = static_cast<uint8_t>(color.g * intensity + bg.g * (1.0f - intensity));
    uint8_t b = static_cast<uint8_t>(color.b * intensity + bg.b * (1.0f - intensity));

    canvas.SetPixel(x, y, Color(r, g, b, 255));
}

// ---------------------------------------------------------
// Bresenham's Line Algorithm
// ---------------------------------------------------------
void Algorithms::DrawLineBresenham(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    
    // Determine the direction of step
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    
    // Initial error term
    int err = dx - dy;

    while (true) {
        canvas.SetPixel(x0, y0, color);
        
        // Check if we have reached the end point
        if (x0 == x1 && y0 == y1) break;
        
        // Calculate error for the next pixel
        int e2 = 2 * err;
        
        // Adjust error and x-coordinate if we step horizontally
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        
        // Adjust error and y-coordinate if we step vertically
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// ---------------------------------------------------------
// Xiaolin Wu's algorithm for Anti-aliased line drawing
// ---------------------------------------------------------
void Algorithms::DrawLineWu(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

    // Swap coordinates if line is steep to simplify calculation
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    // Always draw from lower x to higher x
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    float gradient = (dx == 0) ? 1.0f : static_cast<float>(dy) / dx;

    // Handle the first endpoint
    int xend = x0;
    int yend = y0 + gradient * (xend - x0);
    float xgap = reverseFractionalPart(x0 + 0.5f);
    int xpxl1 = xend; // will be used in the main loop
    int ypxl1 = yend;

    if (steep) {
        DrawPixelAntiAliased(canvas, ypxl1, xpxl1, color, reverseFractionalPart(yend) * xgap);
        DrawPixelAntiAliased(canvas, ypxl1 + 1, xpxl1, color, fractionalPart(yend) * xgap);
    } else {
        DrawPixelAntiAliased(canvas, xpxl1, ypxl1, color, reverseFractionalPart(yend) * xgap);
        DrawPixelAntiAliased(canvas, xpxl1, ypxl1 + 1, color, fractionalPart(yend) * xgap);
    }
    float intery = yend + gradient; // first y-intersection for the main loop

    // Handle the second endpoint
    xend = x1;
    yend = y1 + gradient * (xend - x1);
    xgap = fractionalPart(x1 + 0.5f);
    int xpxl2 = xend;
    int ypxl2 = yend;

    if (steep) {
        DrawPixelAntiAliased(canvas, ypxl2, xpxl2, color, reverseFractionalPart(yend) * xgap);
        DrawPixelAntiAliased(canvas, ypxl2 + 1, xpxl2, color, fractionalPart(yend) * xgap);
    } else {
        DrawPixelAntiAliased(canvas, xpxl2, ypxl2, color, reverseFractionalPart(yend) * xgap);
        DrawPixelAntiAliased(canvas, xpxl2, ypxl2 + 1, color, fractionalPart(yend) * xgap);
    }

    // Main loop
    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2; ++x) {
            DrawPixelAntiAliased(canvas, static_cast<int>(intery), x, color, reverseFractionalPart(intery));
            DrawPixelAntiAliased(canvas, static_cast<int>(intery) + 1, x, color, fractionalPart(intery));
            intery += gradient;
        }
    } else {
        for (int x = xpxl1 + 1; x < xpxl2; ++x) {
            DrawPixelAntiAliased(canvas, x, static_cast<int>(intery), color, reverseFractionalPart(intery));
            DrawPixelAntiAliased(canvas, x, static_cast<int>(intery) + 1, color, fractionalPart(intery));
            intery += gradient;
        }
    }
}

// ---------------------------------------------------------
// Midpoint Circle Algorithm
// ---------------------------------------------------------
void Algorithms::DrawCircleMidpoint(Canvas& canvas, int xc, int yc, int radius, const Color& color, bool isFilled, const Color& fillColor) {
    if (radius <= 0) return;
    
    int x = 0;
    int y = radius;
    // Decision parameter to decide which pixel is closer to the true circle
    int d = 1 - radius; // Equivalent to 5/4 - r for integer math

    // Inner lambda to plot 8 symmetrical points
    auto plot8 = [&](int cx, int cy, int px, int py) {
        if (isFilled && fillColor.a > 0) {
            for (int ix = cx - px; ix <= cx + px; ++ix) {
                canvas.SetPixel(ix, cy + py, fillColor);
                canvas.SetPixel(ix, cy - py, fillColor);
            }
            for (int ix = cx - py; ix <= cx + py; ++ix) {
                canvas.SetPixel(ix, cy + px, fillColor);
                canvas.SetPixel(ix, cy - px, fillColor);
            }
        }
        canvas.SetPixel(cx + px, cy + py, color);
        canvas.SetPixel(cx - px, cy + py, color);
        canvas.SetPixel(cx + px, cy - py, color);
        canvas.SetPixel(cx - px, cy - py, color);
        canvas.SetPixel(cx + py, cy + px, color);
        canvas.SetPixel(cx - py, cy + px, color);
        canvas.SetPixel(cx + py, cy - px, color);
        canvas.SetPixel(cx - py, cy - px, color);
    };

    plot8(xc, yc, x, y);

    while (x < y) {
        if (d < 0) {
            // Select E (East) pixel
            d += 2 * x + 3;
        } else {
            // Select SE (South-East) pixel
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        plot8(xc, yc, x, y);
    }
}

// ---------------------------------------------------------
// Ellipse Drawing Algorithm (Midpoint)
// ---------------------------------------------------------
void Algorithms::DrawEllipse(Canvas& canvas, int xc, int yc, int rx, int ry, const Color& color, bool isFilled, const Color& fillColor) {
    if (rx <= 0 || ry <= 0) return;

    float dx, dy, d1, d2, x, y;
    x = 0;
    y = ry;

    // Initial decision parameter for region 1
    d1 = (ry * ry) - (rx * rx * ry) + (0.25f * rx * rx);
    dx = 2 * ry * ry * x;
    dy = 2 * rx * rx * y;

    auto plot4 = [&](int cx, int cy, int px, int py) {
        if (isFilled && fillColor.a > 0) {
            for (int ix = cx - px; ix <= cx + px; ++ix) {
                canvas.SetPixel(ix, cy + py, fillColor);
                canvas.SetPixel(ix, cy - py, fillColor);
            }
        }
        canvas.SetPixel(cx + px, cy + py, color);
        canvas.SetPixel(cx - px, cy + py, color);
        canvas.SetPixel(cx + px, cy - py, color);
        canvas.SetPixel(cx - px, cy - py, color);
    };

    // For region 1
    while (dx < dy) {
        plot4(xc, yc, x, y);
        if (d1 < 0) {
            x++;
            dx = dx + (2 * ry * ry);
            d1 = d1 + dx + (ry * ry);
        } else {
            x++;
            y--;
            dx = dx + (2 * ry * ry);
            dy = dy - (2 * rx * rx);
            d1 = d1 + dx - dy + (ry * ry);
        }
    }

    // Decision parameter for region 2
    d2 = ((ry * ry) * ((x + 0.5f) * (x + 0.5f))) + ((rx * rx) * ((y - 1) * (y - 1))) - (rx * rx * ry * ry);

    // For region 2
    while (y >= 0) {
        plot4(xc, yc, x, y);
        if (d2 > 0) {
            y--;
            dy = dy - (2 * rx * rx);
            d2 = d2 + (rx * rx) - dy;
        } else {
            y--;
            x++;
            dx = dx + (2 * ry * ry);
            dy = dy - (2 * rx * rx);
            d2 = d2 + dx - dy + (rx * rx);
        }
    }
}

// ---------------------------------------------------------
// Square Drawing (defined by center or bounding box)
// Let's implement it as a rectangle/square bounding box from (x0, y0) to (x1, y1)
// ---------------------------------------------------------
void Algorithms::DrawSquare(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color, bool isFilled, const Color& fillColor) {
    // Determine the side length for a square if we want it strictly square,
    // or just draw a rectangle as requested by "Square" in common paint parlance.
    // Given the prompt asks for "Square", I'll make it a square by taking the max delta.
    
    int dx = x1 - x0;
    int dy = y1 - y0;
    int side = std::max(std::abs(dx), std::abs(dy));
    
    int realX1 = x0 + (dx >= 0 ? side : -side);
    int realY1 = y0 + (dy >= 0 ? side : -side);

    if (isFilled && fillColor.a > 0) {
        int minX = std::min(x0, realX1);
        int maxX = std::max(x0, realX1);
        int minY = std::min(y0, realY1);
        int maxY = std::max(y0, realY1);
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                canvas.SetPixel(x, y, fillColor);
            }
        }
    }

    Algorithms::DrawLineBresenham(canvas, x0, y0, realX1, y0, color);
    Algorithms::DrawLineBresenham(canvas, realX1, y0, realX1, realY1, color);
    Algorithms::DrawLineBresenham(canvas, realX1, realY1, x0, realY1, color);
    Algorithms::DrawLineBresenham(canvas, x0, realY1, x0, y0, color);
}

// ---------------------------------------------------------
// Basic Flood Fill Algorithm
// ---------------------------------------------------------
void Algorithms::FloodFill(Canvas& canvas, int startX, int startY, const Color& newColor, std::vector<std::pair<int, int>>* outPoints) {
    // Check out of bounds
    if (startX < 0 || startX >= canvas.GetWidth() || startY < 0 || startY >= canvas.GetHeight()) return;

    Color targetColor = canvas.GetPixel(startX, startY);

    // If target is the same as new color, do nothing
    if (targetColor.Pack() == newColor.Pack()) return;

    // Use a queue for iterative BFS flood fill (to avoid stack overflow recursion)
    std::queue<std::pair<int, int>> q;
    q.push({startX, startY});

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        if (x < 0 || x >= canvas.GetWidth() || y < 0 || y >= canvas.GetHeight()) continue;

        if (canvas.GetPixel(x, y).Pack() == targetColor.Pack()) {
            canvas.SetPixel(x, y, newColor);
            if (outPoints) outPoints->push_back({x, y});
            
            // Add neighboring pixels
            q.push({x + 1, y});
            q.push({x - 1, y});
            q.push({x, y + 1});
            q.push({x, y - 1});
        }
    }
}

int Algorithms::FindShapeAt(Canvas& canvas, int x, int y) {
    auto& shapes = canvas.GetShapes();
    // Search backwards to select the topmost shape
    for (int i = static_cast<int>(shapes.size()) - 1; i >= 0; --i) {
        const auto& s = shapes[i];
        if (s.type == ShapeType::LINE) {
            // Check distance to line segment
            float dx = static_cast<float>(s.x1 - s.x0);
            float dy = static_cast<float>(s.y1 - s.y0);
            float l2 = dx*dx + dy*dy;
            if (l2 == 0) continue;
            float t = ((x - s.x0) * dx + (y - s.y0) * dy) / l2;
            t = std::max(0.0f, std::min(1.0f, t));
            float dist = std::sqrt(std::pow(x - (s.x0 + t * dx), 2) + std::pow(y - (s.y0 + t * dy), 2));
            if (dist < 8.0f) return i;
        } else if (s.type == ShapeType::CIRCLE) {
            float radius = std::sqrt(std::pow(s.x1 - s.x0, 2) + std::pow(s.y1 - s.y0, 2));
            float dist = std::sqrt(std::pow(x - s.x0, 2) + std::pow(y - s.y0, 2));
            if (dist <= radius + 5.0f) return i; // Area check
        } else if (s.type == ShapeType::SQUARE) {
            int sdx = s.x1 - s.x0;
            int sdy = s.y1 - s.y0;
            int side = std::max(std::abs(sdx), std::abs(sdy));
            int rx1 = s.x0 + (sdx >= 0 ? side : -side);
            int ry1 = s.y0 + (sdy >= 0 ? side : -side);
            int minX = std::min(s.x0, rx1);
            int maxX = std::max(s.x0, rx1);
            int minY = std::min(s.y0, ry1);
            int maxY = std::max(s.y0, ry1);
            if (x >= minX - 5 && x <= maxX + 5 && y >= minY - 5 && y <= maxY + 5) {
                return i; // Area check
            }
        } else if (s.type == ShapeType::ELLIPSE) {
            int rx = std::abs(s.x1 - s.x0);
            int ry = std::abs(s.y1 - s.y0);
            if (rx == 0 || ry == 0) continue;
            // Ellipse area check: (x-xc)^2/rx^2 + (y-yc)^2/ry^2 <= 1
            float val = std::pow(x - s.x0, 2) / std::pow(rx, 2) + std::pow(y - s.y0, 2) / std::pow(ry, 2);
            if (val <= 1.1f) return i; // Allow slight margin
        } else if (s.type == ShapeType::BEZIER) {
            if (s.points.empty()) continue;
            int cx = s.points[0].first;
            int cy = s.points[0].second;
            auto getBezierPoint = [&](float t) {
                float px = (1.0f - t) * (1.0f - t) * s.x0 + 2.0f * (1.0f - t) * t * cx + t * t * s.x1;
                float py = (1.0f - t) * (1.0f - t) * s.y0 + 2.0f * (1.0f - t) * t * cy + t * t * s.y1;
                return std::make_pair(px, py);
            };
            int segments = 20;
            auto prev = getBezierPoint(0.0f);
            for (int j = 1; j <= segments; ++j) {
                float t = j / static_cast<float>(segments);
                auto curr = getBezierPoint(t);
                float dx = curr.first - prev.first;
                float dy = curr.second - prev.second;
                float l2 = dx*dx + dy*dy;
                if (l2 > 0) {
                    float tt = ((x - prev.first) * dx + (y - prev.second) * dy) / l2;
                    tt = std::max(0.0f, std::min(1.0f, tt));
                    float dist = std::sqrt(std::pow(x - (prev.first + tt * dx), 2) + std::pow(y - (prev.second + tt * dy), 2));
                    if (dist < 8.0f) return i;
                }
                prev = curr;
            }
        } else if (s.type == ShapeType::PENCIL || s.type == ShapeType::BRUSH) {
            if (s.points.empty()) continue;
            for (size_t j = 0; j < s.points.size() - 1; ++j) {
                float px0 = static_cast<float>(s.points[j].first);
                float py0 = static_cast<float>(s.points[j].second);
                float px1 = static_cast<float>(s.points[j+1].first);
                float py1 = static_cast<float>(s.points[j+1].second);
                float dx = px1 - px0;
                float dy = py1 - py0;
                float l2 = dx*dx + dy*dy;
                if (l2 == 0) {
                    float dist = std::sqrt(std::pow(x - px0, 2) + std::pow(y - py0, 2));
                    if (dist < 8.0f) return i;
                    continue;
                }
                float t = ((x - px0) * dx + (y - py0) * dy) / l2;
                t = std::max(0.0f, std::min(1.0f, t));
                float dist = std::sqrt(std::pow(x - (px0 + t * dx), 2) + std::pow(y - (py0 + t * dy), 2));
                if (dist < 8.0f) return i;
            }
        }
    }
    return -1;
}

// ---------------------------------------------------------
// SDF Rendering Implementation
// ---------------------------------------------------------
void Algorithms::SetPixelAntiAliased(Canvas& canvas, int x, int y, const Color& color, float alpha) {
    if (x < 0 || x >= canvas.GetWidth() || y < 0 || y >= canvas.GetHeight() || alpha <= 0.0f) return;
    if (alpha >= 1.0f) {
        canvas.SetPixel(x, y, color);
        return;
    }
    Color bgColor = canvas.GetPixel(x, y);
    uint8_t r = static_cast<uint8_t>((color.r * alpha) + (bgColor.r * (1.0f - alpha)));
    uint8_t g = static_cast<uint8_t>((color.g * alpha) + (bgColor.g * (1.0f - alpha)));
    uint8_t b = static_cast<uint8_t>((color.b * alpha) + (bgColor.b * (1.0f - alpha)));
    canvas.SetPixel(x, y, Color(r, g, b, 255));
}

float Algorithms::GetAlphaSDF(float dist, float thickness, AAType aaType) {
    float halfT = thickness / 2.0f;
    if (dist > halfT + 1.0f) return 0.0f;

    switch (aaType) {
        case AAType::NONE:
            return dist <= halfT ? 1.0f : 0.0f;
        case AAType::EDGE_LINEAR:
            if (dist <= halfT - 0.5f) return 1.0f;
            if (dist >= halfT + 0.5f) return 0.0f;
            return 0.5f - (dist - halfT);
        case AAType::CONICAL:
            if (dist >= halfT) return 0.0f;
            return 1.0f - (dist / halfT);
        case AAType::GAUSSIAN:
            if (dist >= halfT) return 0.0f;
            float sigma = halfT / 3.0f;
            if (sigma == 0.0f) return 1.0f;
            return std::exp(-(dist * dist) / (2.0f * sigma * sigma));
    }
    return 1.0f;
}

void Algorithms::DrawLineSDF(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color, float thickness, AAType aaType) {
    int pad = static_cast<int>(thickness / 2.0f) + 2;
    int minX = std::min(x0, x1) - pad;
    int maxX = std::max(x0, x1) + pad;
    int minY = std::min(y0, y1) - pad;
    int maxY = std::max(y0, y1) + pad;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float l2 = dx * dx + dy * dy;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float dist;
            if (l2 == 0.0f) {
                dist = std::sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
            } else {
                float t = std::max(0.0f, std::min(1.0f, ((x - x0) * dx + (y - y0) * dy) / l2));
                float projX = x0 + t * dx;
                float projY = y0 + t * dy;
                dist = std::sqrt((x - projX) * (x - projX) + (y - projY) * (y - projY));
            }
            float alpha = GetAlphaSDF(dist, thickness, aaType);
            if (alpha > 0.0f) SetPixelAntiAliased(canvas, x, y, color, alpha);
        }
    }
}

void Algorithms::DrawCircleSDF(Canvas& canvas, int xc, int yc, int radius, const Color& color, bool isFilled, const Color& fillColor, float thickness, AAType aaType) {
    int pad = static_cast<int>(thickness / 2.0f) + 2;
    int minX = xc - radius - pad;
    int maxX = xc + radius + pad;
    int minY = yc - radius - pad;
    int maxY = yc + radius + pad;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float dCenter = std::sqrt((x - xc) * (x - xc) + (y - yc) * (y - yc));
            float distToEdge = std::abs(dCenter - radius);
            float alpha = GetAlphaSDF(distToEdge, thickness, aaType);
            
            if (alpha > 0.0f) {
                if (isFilled && dCenter < radius) SetPixelAntiAliased(canvas, x, y, fillColor, 1.0f);
                SetPixelAntiAliased(canvas, x, y, color, alpha);
            } else if (isFilled && dCenter < radius) {
                SetPixelAntiAliased(canvas, x, y, fillColor, 1.0f);
            }
        }
    }
}

void Algorithms::DrawEllipseSDF(Canvas& canvas, int xc, int yc, int rx, int ry, const Color& color, bool isFilled, const Color& fillColor, float thickness, AAType aaType) {
    if (rx == 0 || ry == 0) return;
    int pad = static_cast<int>(thickness / 2.0f) + 2;
    int minX = xc - rx - pad;
    int maxX = xc + rx + pad;
    int minY = yc - ry - pad;
    int maxY = yc + ry + pad;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float dx = x - xc;
            float dy = y - yc;
            float theta = std::atan2(dy * rx, dx * ry);
            float ex = rx * std::cos(theta);
            float ey = ry * std::sin(theta);
            
            float distCenterToEdge = std::sqrt(ex*ex + ey*ey);
            float distCenterToPoint = std::sqrt(dx*dx + dy*dy);
            float dist = std::abs(distCenterToPoint - distCenterToEdge);
            float alpha = GetAlphaSDF(dist, thickness, aaType);
            bool isInside = distCenterToPoint < distCenterToEdge;
            
            if (alpha > 0.0f) {
                if (isFilled && isInside) SetPixelAntiAliased(canvas, x, y, fillColor, 1.0f);
                SetPixelAntiAliased(canvas, x, y, color, alpha);
            } else if (isFilled && isInside) {
                SetPixelAntiAliased(canvas, x, y, fillColor, 1.0f);
            }
        }
    }
}

void Algorithms::DrawSquareSDF(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color, bool isFilled, const Color& fillColor, float thickness, AAType aaType) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int side = std::max(std::abs(dx), std::abs(dy));
    int realX1 = x0 + (dx >= 0 ? side : -side);
    int realY1 = y0 + (dy >= 0 ? side : -side);
    
    int rMinX = std::min(x0, realX1);
    int rMaxX = std::max(x0, realX1);
    int rMinY = std::min(y0, realY1);
    int rMaxY = std::max(y0, realY1);

    int pad = static_cast<int>(thickness / 2.0f) + 2;
    int minX = rMinX - pad;
    int maxX = rMaxX + pad;
    int minY = rMinY - pad;
    int maxY = rMaxY + pad;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float dX = std::max(0.0f, std::max((float)(rMinX - x), (float)(x - rMaxX)));
            float dY = std::max(0.0f, std::max((float)(rMinY - y), (float)(y - rMaxY)));
            float dist;
            if (x >= rMinX && x <= rMaxX && y >= rMinY && y <= rMaxY) {
                dist = std::min({ (float)(x - rMinX), (float)(rMaxX - x), (float)(y - rMinY), (float)(rMaxY - y) });
                float alpha = GetAlphaSDF(dist, thickness, aaType);
                if (alpha > 0.0f) {
                    if (isFilled) SetPixelAntiAliased(canvas, x, y, fillColor, 1.0f);
                    SetPixelAntiAliased(canvas, x, y, color, alpha);
                } else if (isFilled) {
                    SetPixelAntiAliased(canvas, x, y, fillColor, 1.0f);
                }
            } else {
                dist = std::sqrt(dX * dX + dY * dY);
                float alpha = GetAlphaSDF(dist, thickness, aaType);
                if (alpha > 0.0f) SetPixelAntiAliased(canvas, x, y, color, alpha);
            }
        }
    }
}

void Algorithms::DrawBezierSDF(Canvas& canvas, int x0, int y0, int x1, int y1, int cx, int cy, const Color& color, float thickness, AAType aaType) {
    auto getBezierPoint = [&](float t) {
        float x = (1.0f - t) * (1.0f - t) * x0 + 2.0f * (1.0f - t) * t * cx + t * t * x1;
        float y = (1.0f - t) * (1.0f - t) * y0 + 2.0f * (1.0f - t) * t * cy + t * t * y1;
        return std::make_pair(x, y);
    };

    int segments = 50;
    auto prev = getBezierPoint(0.0f);
    for (int i = 1; i <= segments; ++i) {
        float t = i / static_cast<float>(segments);
        auto curr = getBezierPoint(t);
        DrawLineSDF(canvas, static_cast<int>(prev.first), static_cast<int>(prev.second), 
                           static_cast<int>(curr.first), static_cast<int>(curr.second), 
                           color, thickness, aaType);
        prev = curr;
    }
}

// ---------------------------------------------------------
// Dashed Bounding Box for selections
// ---------------------------------------------------------
void Algorithms::DrawDashedBox(Canvas& canvas, int x0, int y0, int x1, int y1, const Color& color, int dashLength) {
    // Top Edge
    for (int x = x0; x <= x1; ++x) {
        if ((x / dashLength) % 2 == 0) canvas.SetPixel(x, y0, color);
    }
    // Bottom Edge
    for (int x = x0; x <= x1; ++x) {
        if ((x / dashLength) % 2 == 0) canvas.SetPixel(x, y1, color);
    }
    // Left Edge
    for (int y = y0; y <= y1; ++y) {
        if ((y / dashLength) % 2 == 0) canvas.SetPixel(x0, y, color);
    }
    // Right Edge
    for (int y = y0; y <= y1; ++y) {
        if ((y / dashLength) % 2 == 0) canvas.SetPixel(x1, y, color);
    }
}
