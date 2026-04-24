#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>

#include "Canvas.h"
#include "Algorithms.h"
#include "Renderer.h"

// Window dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Application state
enum class Tool { LINE, CIRCLE, SQUARE, ELLIPSE, FILL_BUCKET, PENCIL, BRUSH, ERASER, MOVE };
Tool currentTool = Tool::PENCIL;
float currentColor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // RGBA Draw color
float backgroundColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // RGBA Back color

Color GetCurrentColor() {
    return Color(static_cast<uint8_t>(currentColor[0] * 255.0f), static_cast<uint8_t>(currentColor[1] * 255.0f), static_cast<uint8_t>(currentColor[2] * 255.0f), static_cast<uint8_t>(currentColor[3] * 255.0f));
}
Color GetBGColor() {
    return Color(static_cast<uint8_t>(backgroundColor[0] * 255.0f), static_cast<uint8_t>(backgroundColor[1] * 255.0f), static_cast<uint8_t>(backgroundColor[2] * 255.0f), static_cast<uint8_t>(backgroundColor[3] * 255.0f));
}

float currentThickness = 1.0f;
int currentAATypeInt = 0;
const char* aaItems[] = { "None", "Linear Edge", "Conical", "Gaussian" };
bool isDrawing = false;
int lastMouseX = -1;
int lastMouseY = -1;
int startMouseX = -1;
int startMouseY = -1;
int selectedShapeIdx = -1;
std::vector<int> draggingShapeIndices;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // ------------------------------------------------------------------
    // GLFW Init
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2D Paint Application - Software Rasterizer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ------------------------------------------------------------------
    // GLEW Init
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // ------------------------------------------------------------------
    // ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.FontGlobalScale = 1.8f;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.35f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ------------------------------------------------------------------
    // App Components Init
    Canvas canvas(SCR_WIDTH, SCR_HEIGHT);
    canvas.Clear(GetBGColor());
    canvas.currentBgColor = GetBGColor();
    
    Renderer renderer;
    renderer.Initialize();

    std::unique_ptr<Canvas> backupCanvas = nullptr;
    Shape tempFreehandShape;

    // ------------------------------------------------------------------
    // Main Render Loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        int windowW, windowH;
        glfwGetWindowSize(window, &windowW, &windowH);

        // Process Mouse Input for Canvas
        if (!io.WantCaptureMouse) { // Don't draw if mouse is over ImGui windows
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                double mx, my;
                glfwGetCursorPos(window, &mx, &my);
                
                // Map screen coordinates to internal canvas constraints, accounting for arbitrary window sizing natively
                int mouseX = static_cast<int>((mx / windowW) * SCR_WIDTH);
                int mouseY = static_cast<int>((my / windowH) * SCR_HEIGHT);
                
                if (!isDrawing) {
                    isDrawing = true;
                    lastMouseX = mouseX;
                    lastMouseY = mouseY;
                    startMouseX = mouseX;
                    startMouseY = mouseY;
                    
                    // Take snapshot for preview-based drawing
                    backupCanvas = std::make_unique<Canvas>(canvas);
                    
                    if (currentTool == Tool::FILL_BUCKET) {
                        int hitIdx = Algorithms::FindShapeAt(canvas, mouseX, mouseY);
                        if (hitIdx != -1) {
                            int targetGroup = canvas.GetShapes()[hitIdx].groupId;
                            for (auto& hitShape : canvas.GetShapes()) {
                                if (hitShape.groupId == targetGroup) {
                                    if (hitShape.type == ShapeType::CIRCLE || hitShape.type == ShapeType::SQUARE || hitShape.type == ShapeType::ELLIPSE) {
                                        hitShape.isFilled = true;
                                        hitShape.fillColor = GetCurrentColor();
                                    } else {
                                        hitShape.color = GetCurrentColor();
                                    }
                                }
                            }
                            canvas.Redraw();
                        } else {
                            std::vector<std::pair<int, int>> fillPoints;
                            Algorithms::FloodFill(canvas, mouseX, mouseY, GetCurrentColor(), &fillPoints);
                            if (!fillPoints.empty()) {
                                Shape fillShape;
                                fillShape.type = ShapeType::FILL;
                                fillShape.color = GetCurrentColor();
                                fillShape.points = std::move(fillPoints);
                                canvas.AddShape(fillShape);
                            }
                            canvas.Redraw();
                        }
                    } else if (currentTool == Tool::MOVE) {
                        selectedShapeIdx = Algorithms::FindShapeAt(canvas, mouseX, mouseY);
                        draggingShapeIndices.clear();
                        if (selectedShapeIdx != -1) {
                            int group = canvas.GetShapes()[selectedShapeIdx].groupId;
                            for (size_t i = 0; i < canvas.GetShapes().size(); ++i) {
                                if (canvas.GetShapes()[i].groupId == group) {
                                    draggingShapeIndices.push_back(i);
                                }
                            }
                            canvas.Redraw();
                        }
                    } else if (currentTool == Tool::PENCIL || currentTool == Tool::BRUSH || currentTool == Tool::ERASER) {
                        tempFreehandShape = Shape();
                        if (currentTool == Tool::PENCIL) tempFreehandShape.type = ShapeType::PENCIL;
                        else if (currentTool == Tool::BRUSH) tempFreehandShape.type = ShapeType::BRUSH;
                        else tempFreehandShape.type = ShapeType::ERASER;
                        tempFreehandShape.color = GetCurrentColor();
                        tempFreehandShape.thickness = currentThickness;
                        tempFreehandShape.aaType = static_cast<AAType>(currentAATypeInt);
                        tempFreehandShape.points.push_back({mouseX, mouseY});
                    }
                } else {
                    // Dragging
                    if (currentTool == Tool::MOVE) {
                        if (!draggingShapeIndices.empty()) {
                            int dx = mouseX - lastMouseX;
                            int dy = mouseY - lastMouseY;
                            
                            for (int idx : draggingShapeIndices) {
                                auto& shape = canvas.GetShapes()[idx];
                                shape.x0 += dx;
                                shape.y0 += dy;
                                shape.x1 += dx;
                                shape.y1 += dy;
                                if (shape.type == ShapeType::PENCIL || shape.type == ShapeType::BRUSH || shape.type == ShapeType::FILL || shape.type == ShapeType::ERASER) {
                                    for (auto& p : shape.points) {
                                        p.first += dx;
                                        p.second += dy;
                                    }
                                }
                            }
                            canvas.Redraw();
                            
                            // Visual feedback for selection: Draw a dashed bounding box around all moved shapes
                            int minX = 999999, minY = 999999, maxX = -999999, maxY = -999999;
                            for (int idx : draggingShapeIndices) {
                                auto& shape = canvas.GetShapes()[idx];
                                int bx_min, by_min, bx_max, by_max;
                                shape.GetBoundingBox(bx_min, by_min, bx_max, by_max);
                                minX = std::min(minX, bx_min);
                                minY = std::min(minY, by_min);
                                maxX = std::max(maxX, bx_max);
                                maxY = std::max(maxY, by_max);
                            }
                            if (minX <= maxX && minY <= maxY) {
                                Algorithms::DrawDashedBox(canvas, minX - 2, minY - 2, maxX + 2, maxY + 2, Color(255, 255, 0));
                            }
                        }
                    } else if (currentTool == Tool::CIRCLE) {
                        canvas = *backupCanvas;
                        int radius = static_cast<int>(std::sqrt(std::pow(mouseX - startMouseX, 2) + std::pow(mouseY - startMouseY, 2)));
                        Algorithms::DrawCircleSDF(canvas, startMouseX, startMouseY, radius, GetCurrentColor(), false, Color(), currentThickness, static_cast<AAType>(currentAATypeInt));
                    } else if (currentTool == Tool::SQUARE) {
                        canvas = *backupCanvas;
                        Algorithms::DrawSquareSDF(canvas, startMouseX, startMouseY, mouseX, mouseY, GetCurrentColor(), false, Color(), currentThickness, static_cast<AAType>(currentAATypeInt));
                    } else if (currentTool == Tool::ELLIPSE) {
                        canvas = *backupCanvas;
                        int rx = std::abs(mouseX - startMouseX);
                        int ry = std::abs(mouseY - startMouseY);
                        Algorithms::DrawEllipseSDF(canvas, startMouseX, startMouseY, rx, ry, GetCurrentColor(), false, Color(), currentThickness, static_cast<AAType>(currentAATypeInt));
                    } else if (currentTool == Tool::PENCIL || currentTool == Tool::BRUSH || currentTool == Tool::ERASER) {
                        auto& pts = tempFreehandShape.points;
                        if (!pts.empty()) {
                            int lastX = pts.back().first;
                            int lastY = pts.back().second;
                            if (lastX != mouseX || lastY != mouseY) {
                                pts.push_back({mouseX, mouseY});
                                Color drawColor = (currentTool == Tool::ERASER) ? GetBGColor() : GetCurrentColor();
                                Algorithms::DrawLineSDF(canvas, lastX, lastY, mouseX, mouseY, drawColor, currentThickness, static_cast<AAType>(currentAATypeInt));
                            }
                        }
                    } else if (currentTool == Tool::LINE) {
                        canvas = *backupCanvas;
                        Algorithms::DrawLineSDF(canvas, startMouseX, startMouseY, mouseX, mouseY, GetCurrentColor(), currentThickness, static_cast<AAType>(currentAATypeInt));
                    }
                    lastMouseX = mouseX;
                    lastMouseY = mouseY;
                }
            } else {
                if (isDrawing) {
                    if (isDrawing && backupCanvas) {
                        double mx, my;
                        glfwGetCursorPos(window, &mx, &my);
                        int mouseX = static_cast<int>((mx / windowW) * SCR_WIDTH);
                        int mouseY = static_cast<int>((my / windowH) * SCR_HEIGHT);

                        if (currentTool == Tool::CIRCLE || currentTool == Tool::SQUARE || currentTool == Tool::ELLIPSE || currentTool == Tool::LINE) {
                            Shape s;
                            if (currentTool == Tool::CIRCLE) s.type = ShapeType::CIRCLE;
                            else if (currentTool == Tool::SQUARE) s.type = ShapeType::SQUARE;
                            else if (currentTool == Tool::ELLIPSE) s.type = ShapeType::ELLIPSE;
                            else if (currentTool == Tool::LINE) s.type = ShapeType::LINE;
                            
                            s.x0 = startMouseX;
                            s.y0 = startMouseY;
                            s.x1 = mouseX;
                            s.y1 = mouseY;
                            s.color = GetCurrentColor();
                            s.thickness = currentThickness;
                            s.aaType = static_cast<AAType>(currentAATypeInt);
                            canvas.AddShape(s);
                            canvas.Redraw();
                        } else if (currentTool == Tool::PENCIL || currentTool == Tool::BRUSH || currentTool == Tool::ERASER) {
                            canvas.AddShape(tempFreehandShape);
                            tempFreehandShape = Shape();
                            canvas.Redraw();
                        }
                    }
                    isDrawing = false;
                    if (currentTool == Tool::MOVE) {
                        canvas.Redraw(); // Final redraw to remove selection highlight
                    }
                    selectedShapeIdx = -1;
                    backupCanvas.reset(); // Free backup once finalized click
                }
            }
        } else {
            // Also cancel drawing if they dragged onto the toolbar
            isDrawing = false;
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ----------------------------------------------------------
        // MS-Paint Style Floating Toolbar natively docked to Top
        // ----------------------------------------------------------
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)windowW, 140.0f));
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin("Tools", nullptr, flags);
        ImGui::SetWindowFontScale(1.1f);
        
        if (ImGui::Button("Clear", ImVec2(110, 0))) {
            canvas.Clear(GetBGColor());
        }
        
        ImGui::SameLine(0, 20);
        // Tools & Properties
        auto ToolButton = [&](const char* label, Tool tool) {
            bool matches = (currentTool == tool);
            if (matches) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.8f, 1.0f));
            if (ImGui::Button(label, ImVec2(120, 0))) {
                currentTool = tool;
            }
            if (matches) ImGui::PopStyleColor();
        };

        ToolButton("Move", Tool::MOVE);
        ImGui::SameLine();
        ToolButton("Pencil", Tool::PENCIL);
        ImGui::SameLine();
        ToolButton("Brush", Tool::BRUSH);
        ImGui::SameLine();
        ToolButton("Eraser", Tool::ERASER);
        ImGui::SameLine();
        ToolButton("Line", Tool::LINE);
        ImGui::SameLine();
        ToolButton("Square", Tool::SQUARE);
        ImGui::SameLine();
        ToolButton("Circle", Tool::CIRCLE);
        ImGui::SameLine();
        ToolButton("Ellipse", Tool::ELLIPSE);
        ImGui::SameLine();
        ToolButton("Fill", Tool::FILL_BUCKET);
        
        ImGui::Spacing();
        ImGui::PushItemWidth(220);
        ImGui::SliderFloat("Thickness", &currentThickness, 1.0f, 50.0f, "%.1f");
        ImGui::SameLine();
        ImGui::Combo("AA Profile", &currentAATypeInt, aaItems, IM_ARRAYSIZE(aaItems));
        ImGui::PopItemWidth();
        ImGui::Spacing();
        
        // Color Pickers
        ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoOptions;
        
        ImGui::Text("Draw Color");
        ImGui::SameLine();
        ImGui::ColorEdit4("##Draw", currentColor, colorFlags);
        
        ImGui::SameLine();
        ImGui::Text("BG Color");
        ImGui::SameLine();
        if (ImGui::ColorEdit4("##BG", backgroundColor, colorFlags)) {
            canvas.currentBgColor = GetBGColor();
        }

        ImGui::End();

        // ----------------------------------------------------------
        // Render OpenGL Frame
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Upload our CPU canvas to texture and draw the quad
        renderer.Render(canvas);

        // Render ImGui over the canvas
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
