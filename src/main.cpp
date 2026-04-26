#include "../include/Visualizer/VisualizerEngine.h"
#include "../include/UI/RetroTheme.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>

int main() {
    // ================================================================
    //  1. Initialise GLFW
    // ================================================================
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(
        1440, 860, "CGV Algorithm Visualizer — Section 2", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);   // vsync

    // ================================================================
    //  2. Initialise Dear ImGui
    // ================================================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    RetroTheme::ApplyRetroTheme(1.95f, 1.26f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // ================================================================
    //  3. Create Visualizer (Section 2)
    //     TODO: teammates add PaintApp and GraphApp here
    // ================================================================
    VisualizerEngine visualizer;

    // ================================================================
    //  4. Main Render Loop
    // ================================================================
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Handle Font Scaling via Keyboard Shortcuts
        if (io.KeyCtrl) {
            if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
                io.FontGlobalScale = std::min(io.FontGlobalScale + 0.1f, 3.0f);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
                io.FontGlobalScale = std::max(io.FontGlobalScale - 0.1f, 0.5f);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_0) || ImGui::IsKeyPressed(ImGuiKey_Keypad0)) {
                io.FontGlobalScale = 1.0f;
            }
        }

        // Draw the ImGui algorithm panel
        visualizer.renderUI();

        // ---- Clear entire window ----
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.03f, 0.04f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ---- Draw the pixel grid in the right-hand viewport ----
        visualizer.renderGrid(fbWidth, fbHeight);

        // ---- Render ImGui on top (full viewport) ----
        glViewport(0, 0, fbWidth, fbHeight);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ================================================================
    //  5. Cleanup
    // ================================================================
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
