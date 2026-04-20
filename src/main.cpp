#include "../include/Visualizer/VisualizerEngine.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>

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

    // ================================================================
    //  4. Main Render Loop
    // ================================================================
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw the ImGui algorithm panel
        visualizer.renderUI();

        // ---- Clear entire window ----
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
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