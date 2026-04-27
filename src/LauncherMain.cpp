#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../include/UI/RetroTheme.h"
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
    RetroTheme::ApplyRetroTheme(1.95f, 1.26f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::string status;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd))) {
            io.FontGlobalScale += 0.1f;
            if (io.FontGlobalScale > 3.0f) io.FontGlobalScale = 3.0f;
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract))) {
            io.FontGlobalScale -= 0.1f;
            if (io.FontGlobalScale < 0.5f) io.FontGlobalScale = 0.5f;
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_0) || ImGui::IsKeyPressed(ImGuiKey_Keypad0))) {
            io.FontGlobalScale = 1.0f;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        const float now = (float)glfwGetTime();
        RetroTheme::DrawBackdrop(now);

        int fbWidth = 0;
        int fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
        glClearColor(0.03f, 0.04f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGui::Begin("Launcher", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize);

        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float outerPad = 28.0f;
        ImGui::SetCursorScreenPos(ImVec2(outerPad, outerPad));
        ImGui::BeginChild("##HomeCard",
                          ImVec2(display.x - outerPad * 2.0f,
                                 display.y - outerPad * 2.0f),
                          true, ImGuiWindowFlags_NoScrollbar);

        ImDrawList* card = ImGui::GetWindowDrawList();
        const ImVec2 cardMin = ImGui::GetWindowPos();
        const ImVec2 cardSize = ImGui::GetWindowSize();
        const ImVec2 cardMax(cardMin.x + cardSize.x, cardMin.y + cardSize.y);
        RetroTheme::DrawNeonFrame(card, cardMin, cardMax, RetroTheme::NeonCyan(0.95f), now, 22.0f, 1.7f);
        RetroTheme::DrawCornerAccents(card, cardMin, cardMax, RetroTheme::NeonAmber(0.90f), 28.0f, 3.0f);
        RetroTheme::DrawPanelAmbient(card, cardMin, cardMax, now, RetroTheme::NeonCyan(0.9f), 0.9f);
        card->AddRectFilledMultiColor(
            ImVec2(cardMin.x + 18.0f, cardMin.y + 18.0f),
            ImVec2(cardMax.x - 18.0f, cardMin.y + 150.0f),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.05f, 0.12f, 0.18f, 0.85f)),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.03f, 0.05f, 0.08f, 0.15f)),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.03f, 0.05f, 0.08f, 0.05f)),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.05f, 0.12f, 0.18f, 0.75f)));
        for (int i = 0; i < 14; ++i) {
            const float phase = now * (0.7f + i * 0.03f) + i * 0.6f;
            const float px = cardMin.x + 70.0f + i * 120.0f + std::sin(phase) * 22.0f;
            const float py = cardMin.y + 84.0f + std::cos(phase * 1.7f) * 16.0f;
            card->AddCircleFilled(ImVec2(px, py), 2.0f + (i % 3),
                                  (i % 2 == 0) ? RetroTheme::NeonCyan(0.20f) : RetroTheme::NeonAmber(0.18f), 12);
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.22f, 1.0f));
        ImGui::SetWindowFontScale(1.20f);
        ImGui::TextUnformatted("FINAL VISUALIZER");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("retro suite // paint, CGV, DSA, 3D pipeline");
        ImGui::Spacing();

        ImGui::SetWindowFontScale(1.60f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.16f, 0.94f, 0.96f, 1.0f));
        ImGui::TextWrapped("One launch surface for the entire visualization stack.");
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        ImGui::PushItemWidth(100);
        ImGui::SliderFloat("Scale", &io.FontGlobalScale, 0.5f, 3.0f, "%.1f");
        ImGui::PopItemWidth();
        ImGui::TextWrapped(
            "Launch the interactive paint module, the computer graphics visualizer, "
            "the DSA suite, or the 3D graphics pipeline lab from one clean control surface.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        struct ModuleCard {
            const char* title;
            const char* subtitle;
            const char* summaryA;
            const char* summaryB;
            const char* binary;
            ImU32 color;
        };

        const ModuleCard cards[] = {
            {"1) PAINT APP",
             "Raster drawing, line work, fill, anti-aliasing",
             "Software raster sketching",
             "Thickness, AA, fill, move, scale",
             "Paint_Module",
             RetroTheme::NeonAmber(0.95f)},
            {"2) CGV VISUALIZER",
             "Lines, circles, fill, clipping, transform, viewport",
             "Core computer graphics algorithms",
             "DDA, Bresenham, clipping, transforms",
             "CGV_Module",
             RetroTheme::NeonCyan(0.95f)},
            {"3) DSA VISUALIZER",
             "Sorting, searching, graph algorithms with original UI",
             "Step-by-step algorithm behavior",
             "Sorting, searching, traversal flow",
             "DSA_Visualizer",
             RetroTheme::NeonPink(0.95f)},
            {"4) 3D PIPELINE LAB",
             "Model, view, projection, clipping, NDC, viewport",
             "How 3D becomes screen pixels",
             "Matrices, camera, projection, depth",
             "Pipeline3D_Module",
             RetroTheme::NeonCyan(0.92f)},
        };

        const float availableW = ImGui::GetContentRegionAvail().x;
        const float gap = 26.0f;
        const int columns = 2;
        const float cardW = (availableW - gap * (columns - 1)) / columns;
        const float cardH = 360.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.0f, 20.0f));
        for (int i = 0; i < 4; ++i) {
            ImGui::PushID(i);
            ImGui::BeginChild("##ModuleCard", ImVec2(cardW, cardH), true, ImGuiWindowFlags_NoScrollbar);
            ImDrawList* moduleDl = ImGui::GetWindowDrawList();
            const ImVec2 min = ImGui::GetWindowPos();
            const ImVec2 size = ImGui::GetWindowSize();
            const ImVec2 max(min.x + size.x, min.y + size.y);
            const float localTime = now + i * 0.8f;
            RetroTheme::DrawNeonFrame(moduleDl, min, max, cards[i].color, now + i * 0.4f, 16.0f, 1.3f);
            RetroTheme::DrawPanelAmbient(moduleDl, min, max, localTime, cards[i].color, 0.82f);
            moduleDl->AddRectFilledMultiColor(
                min, ImVec2(max.x, min.y + 8.0f),
                cards[i].color,
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.02f, 0.03f, 0.05f, 0.0f)),
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.02f, 0.03f, 0.05f, 0.0f)),
                cards[i].color);
            const ImVec2 orbCenter(max.x - 56.0f, min.y + 48.0f);
            moduleDl->AddCircleFilled(orbCenter, 18.0f, cards[i].color, 20);
            moduleDl->AddCircle(orbCenter, 26.0f + std::sin(localTime * 2.1f) * 4.0f,
                                ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 0.16f)), 28, 1.2f);
            moduleDl->AddCircle(orbCenter, 38.0f + std::cos(localTime * 1.6f) * 5.0f,
                                ImGui::ColorConvertFloat4ToU32(ImVec4(0.16f, 0.94f, 0.96f, 0.10f)), 28, 1.0f);
            for (int bar = 0; bar < 6; ++bar) {
                const float amp = 10.0f + std::sin(localTime * 3.0f + bar * 0.7f) * 8.0f;
                const float bx0 = min.x + 24.0f + bar * 12.0f;
                const float by1 = max.y - 80.0f;
                moduleDl->AddRectFilled(ImVec2(bx0, by1 - amp), ImVec2(bx0 + 7.0f, by1),
                                        ImGui::ColorConvertFloat4ToU32(ImVec4(0.22f, 0.70f, 0.90f, 0.35f)), 3.0f);
            }

            ImGui::SetWindowFontScale(1.12f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(cards[i].color));
            ImGui::TextUnformatted(cards[i].title);
            ImGui::PopStyleColor();
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Spacing();
            ImGui::BeginChild("##CardSummary", ImVec2(0.0f, 60.0f), false, ImGuiWindowFlags_NoScrollbar);
            ImGui::TextWrapped("%s", cards[i].subtitle);
            ImGui::EndChild();

            ImGui::BeginChild("##Facts", ImVec2(0.0f, 92.0f), false, ImGuiWindowFlags_NoScrollbar);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.82f, 0.84f, 0.90f, 1.0f));
            ImGui::BulletText("%s", cards[i].summaryA);
            ImGui::BulletText("%s", cards[i].summaryB);
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::SetCursorPosY(cardH - 108.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.36f, 0.52f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.48f, 0.66f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.58f, 0.78f, 1.0f));
            if (ImGui::Button("OPEN STUDIO", ImVec2(-1.0f, 56.0f))) {
                launchBinary(cards[i].binary, status);
            }
            ImGui::PopStyleColor(3);
            ImGui::SetCursorPosY(cardH - 42.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.58f, 0.62f, 0.72f, 1.0f));
            ImGui::TextUnformatted("click to launch");
            ImGui::PopStyleColor();
            ImGui::EndChild();
            if (i % columns != columns - 1) {
                ImGui::SameLine(0.0f, gap);
            } else if (i < 3) {
                ImGui::Spacing();
            }
            ImGui::PopID();
        }
        ImGui::PopStyleVar();

        ImGui::EndChild();
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
