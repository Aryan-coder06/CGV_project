#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <cmath>
#include <algorithm>

struct Color {
    uint8_t r, g, b, a;
    
    // Default constructor (black, fully opaque)
    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    
    // Pack into a single 32-bit unsigned int (ABGR order for typical little-endian OpenGL RGBA)
    // Adjust if needed depending on GL_RGBA format.
    uint32_t Pack() const {
        return (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(g) << 8) | uint32_t(r);
    }

    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

enum class AAType { NONE, EDGE_LINEAR, CONICAL, GAUSSIAN };

enum class ShapeType { LINE, CIRCLE, SQUARE, ELLIPSE, PENCIL, BRUSH, FILL, ERASER };

struct Shape {
    ShapeType type;
    int x0, y0, x1, y1; // Coordinates (start/end or center/radius)
    Color color;
    bool isFilled = false;
    Color fillColor;
    int groupId = -1;
    float thickness = 1.0f;
    AAType aaType = AAType::NONE;

    std::vector<std::pair<int, int>> points; // For freehand paths

    void GetBoundingBox(int& minX, int& minY, int& maxX, int& maxY) const {
        if (type == ShapeType::PENCIL || type == ShapeType::BRUSH || type == ShapeType::FILL || type == ShapeType::ERASER) {
            if (points.empty()) {
                minX = minY = maxX = maxY = 0;
                return;
            }
            minX = maxX = points[0].first;
            minY = maxY = points[0].second;
            for (const auto& p : points) {
                minX = std::min(minX, p.first);
                minY = std::min(minY, p.second);
                maxX = std::max(maxX, p.first);
                maxY = std::max(maxY, p.second);
            }
        } else if (type == ShapeType::CIRCLE) {
            int radius = static_cast<int>(std::sqrt(std::pow(x1 - x0, 2) + std::pow(y1 - y0, 2)));
            minX = x0 - radius;
            minY = y0 - radius;
            maxX = x0 + radius;
            maxY = y0 + radius;
        } else if (type == ShapeType::ELLIPSE) {
            int rx = std::abs(x1 - x0);
            int ry = std::abs(y1 - y0);
            minX = x0 - rx;
            minY = y0 - ry;
            maxX = x0 + rx;
            maxY = y0 + ry;
        } else if (type == ShapeType::SQUARE) {
            int sdx = x1 - x0;
            int sdy = y1 - y0;
            int side = std::max(std::abs(sdx), std::abs(sdy));
            int rx1 = x0 + (sdx >= 0 ? side : -side);
            int ry1 = y0 + (sdy >= 0 ? side : -side);
            minX = std::min(x0, rx1);
            minY = std::min(y0, ry1);
            maxX = std::max(x0, rx1);
            maxY = std::max(y0, ry1);
        } else {
            minX = std::min(x0, x1);
            minY = std::min(y0, y1);
            maxX = std::max(x0, x1);
            maxY = std::max(y0, y1);
        }
    }
};

class Canvas {
public:
    Color currentBgColor = Color(0, 0, 0, 255);

    Canvas(int width, int height);
    ~Canvas() = default;

    void SetPixel(int x, int y, const Color& color);
    Color GetPixel(int x, int y) const;
    void Clear(const Color& color);

    void AddShape(const Shape& shape) { shapes.push_back(shape); }
    std::vector<Shape>& GetShapes() { return shapes; }
    void Redraw();
    void SaveToBackground(); // New: Copy current buffer to background

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    const std::vector<uint32_t>& GetBuffer() const { return buffer; }

private:
    int width;
    int height;
    std::vector<uint32_t> buffer;
    std::vector<uint32_t> backgroundBuffer; // New: Persistent layer for doodles
    std::vector<Shape> shapes;

    std::vector<int> shapeMap;
    std::vector<std::pair<int, int>> collisions;
    int currentDrawingShape = -1;
};
