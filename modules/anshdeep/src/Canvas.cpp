#include "Canvas.h"

Canvas::Canvas(int width, int height) : width(width), height(height) {
    buffer.resize(width * height, Color(0, 0, 0, 255).Pack()); // Initialize with black
    backgroundBuffer.resize(width * height, Color(0, 0, 0, 255).Pack());
    shapeMap.resize(width * height, -1);
}

void Canvas::SetPixel(int x, int y, const Color& color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        buffer[y * width + x] = color.Pack();
        if (currentDrawingShape != -1) {
            auto check = [&](int px, int py) {
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    int prev = shapeMap[py * width + px];
                    if (prev != -1 && prev != currentDrawingShape) {
                        collisions.push_back({prev, currentDrawingShape});
                    }
                }
            };
            check(x, y);
            check(x - 1, y);
            check(x + 1, y);
            check(x, y - 1);
            check(x, y + 1);
            shapeMap[y * width + x] = currentDrawingShape;
        }
    }
}

Color Canvas::GetPixel(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        uint32_t c = buffer[y * width + x];
        uint8_t r = c & 0xFF;
        uint8_t g = (c >> 8) & 0xFF;
        uint8_t b = (c >> 16) & 0xFF;
        uint8_t a = (c >> 24) & 0xFF;
        return Color(r, g, b, a);
    }
    return Color(0, 0, 0, 0); // Out of bounds
}

void Canvas::Clear(const Color& color) {
    uint32_t packedColor = color.Pack();
    std::fill(buffer.begin(), buffer.end(), packedColor);
    std::fill(backgroundBuffer.begin(), backgroundBuffer.end(), packedColor);
    std::fill(shapeMap.begin(), shapeMap.end(), -1);
    collisions.clear();
    currentDrawingShape = -1;
    shapes.clear();
}

#include "Algorithms.h"
#include <cmath>

void Canvas::SaveToBackground() {
    backgroundBuffer = buffer;
}

void Canvas::Redraw() {
    // Restore from persistent background (doodles/fills)
    buffer = backgroundBuffer;
    std::fill(shapeMap.begin(), shapeMap.end(), -1);
    collisions.clear();

    for (size_t i = 0; i < shapes.size(); ++i) {
        currentDrawingShape = static_cast<int>(i);
        auto& shape = shapes[i];
        switch (shape.type) {
            case ShapeType::LINE:
                Algorithms::DrawLineSDF(*this, shape.x0, shape.y0, shape.x1, shape.y1, shape.color, shape.thickness, shape.aaType);
                break;
            case ShapeType::CIRCLE: {
                int radius = static_cast<int>(std::sqrt(std::pow(shape.x1 - shape.x0, 2) + std::pow(shape.y1 - shape.y0, 2)));
                Algorithms::DrawCircleSDF(*this, shape.x0, shape.y0, radius, shape.color, shape.isFilled, shape.fillColor, shape.thickness, shape.aaType);
                break;
            }
            case ShapeType::SQUARE:
                Algorithms::DrawSquareSDF(*this, shape.x0, shape.y0, shape.x1, shape.y1, shape.color, shape.isFilled, shape.fillColor, shape.thickness, shape.aaType);
                break;
            case ShapeType::ELLIPSE: {
                int rx = std::abs(shape.x1 - shape.x0);
                int ry = std::abs(shape.y1 - shape.y0);
                Algorithms::DrawEllipseSDF(*this, shape.x0, shape.y0, rx, ry, shape.color, shape.isFilled, shape.fillColor, shape.thickness, shape.aaType);
                break;
            }
            case ShapeType::PENCIL:
            case ShapeType::BRUSH:
            case ShapeType::ERASER: {
                if (shape.points.size() > 0) {
                    Color drawColor = (shape.type == ShapeType::ERASER) ? this->currentBgColor : shape.color;
                    for (size_t i = 1; i < shape.points.size(); ++i) {
                        Algorithms::DrawLineSDF(*this, shape.points[i-1].first, shape.points[i-1].second, shape.points[i].first, shape.points[i].second, drawColor, shape.thickness, shape.aaType);
                    }
                }
                break;
            }
            case ShapeType::FILL: {
                if (shape.points.size() > 0) {
                    for (const auto& p : shape.points) {
                        this->SetPixel(p.first, p.second, shape.color);
                    }
                }
                break;
            }
        }
    }
    currentDrawingShape = -1;

    // Evaluate DSU and groupings
    std::vector<int> parent(shapes.size());
    for (size_t i = 0; i < shapes.size(); ++i) parent[i] = i;
    
    auto find = [&](int i) -> int {
        int root = i;
        while (root != parent[root]) root = parent[root];
        int curr = i;
        while (curr != root) {
            int nxt = parent[curr];
            parent[curr] = root;
            curr = nxt;
        }
        return root;
    };
    
    for (const auto& pair : collisions) {
        int root1 = find(pair.first);
        int root2 = find(pair.second);
        if (root1 != root2) {
            parent[root1] = root2;
        }
    }
    
    for (size_t i = 0; i < shapes.size(); ++i) {
        shapes[i].groupId = find(i);
    }
}
