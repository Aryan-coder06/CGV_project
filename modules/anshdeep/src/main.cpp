#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>

#include "Canvas.h"
#include "Algorithms.h"
#include "Renderer.h"
#include "UI/RetroTheme.h"

// Window dimensions
const unsigned int SCR_WIDTH = 1440;
const unsigned int SCR_HEIGHT = 860;

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

void handleShortcutToggles(GLFWwindow* window) {
    static bool aaIncreaseLatch = false;
    static bool aaDecreaseLatch = false;

    const bool ctrlDown =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
    const bool plusPressed =
        glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS;
    const bool minusPressed =
        glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;

    if (ctrlDown && plusPressed && !aaIncreaseLatch) {
        currentAATypeInt = (currentAATypeInt + 1) % IM_ARRAYSIZE(aaItems);
        aaIncreaseLatch = true;
    }
    if (!(ctrlDown && plusPressed)) {
        aaIncreaseLatch = false;
    }

    if (ctrlDown && minusPressed && !aaDecreaseLatch) {
        currentAATypeInt =
            (currentAATypeInt - 1 + IM_ARRAYSIZE(aaItems)) % IM_ARRAYSIZE(aaItems);
        aaDecreaseLatch = true;
    }
    if (!(ctrlDown && minusPressed)) {
        aaDecreaseLatch = false;
    }
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
    RetroTheme::ApplyRetroTheme(1.90f, 1.24f);

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
        handleShortcutToggles(window);

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
        ImGui::SetNextWindowSize(ImVec2((float)windowW, 430.0f));
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin("Tools", nullptr, flags);
        ImGui::SetWindowFontScale(1.10f);
        const ImVec2 toolsPos = ImGui::GetWindowPos();
        const ImVec2 toolsSize = ImGui::GetWindowSize();
        const ImVec2 toolsMax(toolsPos.x + toolsSize.x, toolsPos.y + toolsSize.y);
        RetroTheme::DrawNeonFrame(ImGui::GetWindowDrawList(), toolsPos,
                                  toolsMax,
                                  RetroTheme::NeonCyan(0.92f), (float)glfwGetTime(), 16.0f, 1.5f);
        RetroTheme::DrawCornerAccents(ImGui::GetWindowDrawList(), toolsPos,
                                      toolsMax,
                                      RetroTheme::NeonAmber(0.85f), 22.0f, 2.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.22f, 1.0f));
        ImGui::TextUnformatted("PAINT LAB // RASTER STUDIO");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("live tools, shape rasterization, fill and AA");
        ImGui::Separator();
        
        if (ImGui::Button("Clear", ImVec2(128, 0))) {
            canvas.Clear(GetBGColor());
        }
        
        ImGui::SameLine(0, 20);
        // Tools & Properties
        auto ToolButton = [&](const char* label, Tool tool) {
            bool matches = (currentTool == tool);
            if (matches) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.8f, 1.0f));
            if (ImGui::Button(label, ImVec2(132, 0))) {
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
        ImGui::NewLine();
        ToolButton("Circle", Tool::CIRCLE);
        ImGui::SameLine();
        ToolButton("Ellipse", Tool::ELLIPSE);
        ImGui::SameLine();
        ToolButton("Fill", Tool::FILL_BUCKET);
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.16f, 0.94f, 0.96f, 1.0f));
        ImGui::TextUnformatted("STROKE FILTERS");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("thickness + anti-aliasing");
        ImGui::SetCursorPosX(42.0f);
        ImGui::PushItemWidth(260);
        ImGui::SliderFloat("Thickness", &currentThickness, 1.0f, 50.0f, "%.1f px");
        ImGui::SetCursorPosX(42.0f);
        ImGui::Combo("AA Profile", &currentAATypeInt, aaItems, IM_ARRAYSIZE(aaItems));
        ImGui::SameLine();
        ImGui::TextDisabled("Ctrl +/-");
        ImGui::PopItemWidth();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.22f, 1.0f));
        ImGui::TextUnformatted("COLOR CONTROLS");
        ImGui::PopStyleColor();
        ImGui::SetCursorPosX(42.0f);
        
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

        glClearColor(0.03f, 0.04f, 0.08f, 1.0f);
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
