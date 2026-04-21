#include "../include/Visualizer/VisualizerEngine.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>

enum class AppSection {
    Home,
    PaintApp,
    AlgorithmVisualizer,
    GraphVisualizer
};

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
        1280, 720, "CGV Algorithm Visualizer — Section 2", nullptr, nullptr);

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = 1.75f;

    // Dark theme
    ImGui::StyleColorsDark();

    // Tune colours
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding   = 0.0f;
    style.FrameRounding    = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.Colors[ImGuiCol_WindowBg]  = ImVec4(0.08f, 0.08f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_FrameBg]   = ImVec4(0.15f, 0.15f, 0.22f, 1.0f);
    style.Colors[ImGuiCol_Header]    = ImVec4(0.2f,  0.4f,  0.7f,  1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.5f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_Tab]           = ImVec4(0.12f, 0.12f, 0.18f, 1.0f);
    style.Colors[ImGuiCol_TabActive]     = ImVec4(0.2f,  0.4f,  0.7f,  1.0f);
    style.Colors[ImGuiCol_TabHovered]    = ImVec4(0.25f, 0.5f,  0.85f, 1.0f);
    style.Colors[ImGuiCol_Separator]     = ImVec4(0.3f,  0.3f,  0.4f,  1.0f);
    style.Colors[ImGuiCol_Button]        = ImVec4(0.2f,  0.4f,  0.6f,  1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f,  0.5f,  0.8f,  1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // ================================================================
    //  3. Create Visualizer (Section 2)
    //     TODO: teammates add PaintApp and GraphApp here
    // ================================================================
    VisualizerEngine visualizer;

    AppSection activeSection = AppSection::Home;

    auto renderHomePage = [&activeSection]() {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGui::Begin("CGV Home", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        ImGui::SetCursorPosY(40.0f);
        ImGui::SetWindowFontScale(1.3f);
        ImGui::Text("CGV Project");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Text("Choose a mode to start:");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = std::max(420.0f, ImGui::GetContentRegionAvail().x * 0.38f);
        ImVec2 buttonSize(buttonWidth, 70.0f);
        if (ImGui::Button("1) Paint App", buttonSize)) {
            activeSection = AppSection::PaintApp;
        }
        ImGui::TextDisabled("Interactive drawing workspace (coming soon)");
        ImGui::Spacing();

        if (ImGui::Button("2) Algorithm Visualizer", buttonSize)) {
            activeSection = AppSection::AlgorithmVisualizer;
        }
        ImGui::TextDisabled("Current implemented module");
        ImGui::Spacing();

        if (ImGui::Button("3) Graph Visualizer", buttonSize)) {
            activeSection = AppSection::GraphVisualizer;
        }
        ImGui::TextDisabled("Pathfinding and graph animations (coming soon)");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextWrapped(
            "Tip: Press ESC in any mode to come back to this home page.");
        ImGui::End();
    };

    auto renderPlaceholderSection = [&](const char* title, const char* subtitle) {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGui::Begin(title, nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        if (ImGui::Button("<- Back to Home")) {
            activeSection = AppSection::Home;
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::SetWindowFontScale(1.2f);
        ImGui::Text("%s", title);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::TextWrapped("%s", subtitle);
        ImGui::Spacing();
        ImGui::TextDisabled("This module is not implemented yet in this branch.");
        ImGui::End();
    };

    // ================================================================
    //  4. Main Render Loop
    // ================================================================
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            activeSection = AppSection::Home;
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        glViewport(0, 0, fbWidth, fbHeight);
        if (activeSection == AppSection::AlgorithmVisualizer) {
            glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
        } else if (activeSection == AppSection::PaintApp) {
            glClearColor(0.08f, 0.11f, 0.12f, 1.0f);
        } else if (activeSection == AppSection::GraphVisualizer) {
            glClearColor(0.08f, 0.09f, 0.14f, 1.0f);
        } else {
            glClearColor(0.07f, 0.07f, 0.10f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT);

        if (activeSection == AppSection::Home) {
            renderHomePage();
        } else if (activeSection == AppSection::AlgorithmVisualizer) {
            if (ImGui::Begin("Navigation")) {
                if (ImGui::Button("<- Back to Home")) {
                    activeSection = AppSection::Home;
                }
            }
            ImGui::End();
            visualizer.renderUI();
            visualizer.renderGrid(fbWidth, fbHeight);
        } else if (activeSection == AppSection::PaintApp) {
            renderPlaceholderSection("Paint App", "Home page added successfully. "
                                                  "Paint module wiring will be done next.");
        } else if (activeSection == AppSection::GraphVisualizer) {
            renderPlaceholderSection("Graph Visualizer",
                                     "Home page added successfully. Graph module wiring "
                                     "will be done next.");
        }

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
