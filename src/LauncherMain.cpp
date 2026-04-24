#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

std::string launchBinary(const std::string& name, std::string& status) {
    namespace fs = std::filesystem;

    const fs::path cwd = fs::current_path();
    const fs::path candidates[] = {
        cwd / "build-local" / name,
        cwd / "build" / name,
        cwd / name
    };

    for (const auto& candidate : candidates) {
        if (fs::exists(candidate)) {
            std::string cmd = "\"" + candidate.string() + "\" &";
            int code = std::system(cmd.c_str());
            status = code == 0 ? "Launched: " + candidate.string()
                               : "Failed to launch: " + candidate.string();
            return candidate.string();
        }
    }

    status = "Binary not found: " + name;
    return {};
}

}  // namespace

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window =
        glfwCreateWindow(1280, 720, "Final Visualizer Launcher", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = 1.8f;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.11f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.14f, 0.30f, 0.50f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.45f, 0.74f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.10f, 0.58f, 0.88f, 1.0f);
    style.ScaleAllSizes(1.35f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::string status;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int fbWidth = 0;
        int fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGui::Begin("Launcher", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize);

        ImGui::SetWindowFontScale(1.15f);
        ImGui::TextUnformatted("FINAL VISUALIZER");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextWrapped(
            "Launch Khatri's CGV module or Arsh's DSA module without changing "
            "their original internal UI and algorithm flow.");
        ImGui::Spacing();

        const ImVec2 buttonSize(760.0f, 118.0f);

        if (ImGui::Button("1) Open Paint App", buttonSize)) {
            launchBinary("Paint_Module", status);
        }
        ImGui::TextDisabled("Anshdeep branch paint module.");
        ImGui::Spacing();

        if (ImGui::Button("2) Open CGV Algorithm Visualizer", buttonSize)) {
            launchBinary("CGV_Module", status);
        }
        ImGui::TextDisabled("Khatri branch visualizer.");
        ImGui::Spacing();

        if (ImGui::Button("3) Open DSA Algorithm Visualizer", buttonSize)) {
            launchBinary("DSA_Visualizer", status);
        }
        ImGui::TextDisabled("Arsh branch visualizer with original UI and algorithms.");
        ImGui::Spacing();

        ImGui::Separator();
        if (!status.empty()) {
            ImGui::TextWrapped("%s", status.c_str());
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
