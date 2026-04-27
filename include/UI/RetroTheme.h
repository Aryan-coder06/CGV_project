#pragma once

#include "imgui.h"

#include <cmath>

namespace RetroTheme {

inline ImU32 NeonCyan(float alpha = 1.0f) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(0.16f, 0.94f, 0.96f, alpha));
}

inline ImU32 NeonAmber(float alpha = 1.0f) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.78f, 0.22f, alpha));
}

inline ImU32 NeonPink(float alpha = 1.0f) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(0.98f, 0.30f, 0.68f, alpha));
}

inline void ApplyRetroTheme(float fontScale = 1.95f, float sizeScale = 1.28f) {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = fontScale;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 18.0f;
    style.ChildRounding = 16.0f;
    style.FrameRounding = 12.0f;
    style.PopupRounding = 12.0f;
    style.GrabRounding = 12.0f;
    style.ScrollbarRounding = 14.0f;
    style.TabRounding = 10.0f;
    style.WindowBorderSize = 1.4f;
    style.ChildBorderSize = 1.2f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(14.0f, 12.0f);
    style.ItemInnerSpacing = ImVec2(10.0f, 8.0f);
    style.WindowPadding = ImVec2(20.0f, 18.0f);
    style.FramePadding = ImVec2(14.0f, 10.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.06f, 0.10f, 0.96f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.09f, 0.14f, 0.94f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.09f, 0.14f, 0.98f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.16f, 0.78f, 0.90f, 0.45f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.09f, 0.12f, 0.20f, 0.92f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.22f, 0.34f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.28f, 0.40f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.07f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.16f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.09f, 0.14f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.12f, 0.28f, 0.38f, 0.90f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.40f, 0.52f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.18f, 0.48f, 0.60f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.28f, 0.38f, 0.98f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.16f, 0.42f, 0.56f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.54f, 0.72f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.11f, 0.18f, 0.94f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.15f, 0.34f, 0.44f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.10f, 0.24f, 0.34f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.78f, 0.28f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.96f, 0.73f, 0.20f, 0.90f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.82f, 0.32f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.65f, 0.80f, 0.35f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.18f, 0.70f, 0.86f, 0.20f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.18f, 0.70f, 0.86f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.18f, 0.70f, 0.86f, 0.85f);
    style.ScaleAllSizes(sizeScale);
}

inline void DrawBackdrop(float timeSeconds) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const ImVec2 display = ImGui::GetIO().DisplaySize;

    dl->AddRectFilled(ImVec2(0, 0), display,
                      ImGui::ColorConvertFloat4ToU32(ImVec4(0.03f, 0.04f, 0.08f, 1.0f)));

    const float sweep = (std::sin(timeSeconds * 0.45f) + 1.0f) * 0.5f;
    const ImVec2 haloCenter(display.x * (0.18f + sweep * 0.16f),
                            display.y * (0.22f + sweep * 0.08f));
    dl->AddCircleFilled(haloCenter, 340.0f,
                        ImGui::ColorConvertFloat4ToU32(ImVec4(0.06f, 0.22f, 0.30f, 0.18f)), 96);
    dl->AddCircleFilled(ImVec2(display.x * 0.82f, display.y * 0.80f), 260.0f,
                        ImGui::ColorConvertFloat4ToU32(ImVec4(0.30f, 0.10f, 0.16f, 0.10f)), 96);

    for (float y = 0.0f; y < display.y; y += 6.0f) {
        dl->AddLine(ImVec2(0.0f, y), ImVec2(display.x, y),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.05f)));
    }

    for (float x = 0.0f; x < display.x; x += 64.0f) {
        dl->AddLine(ImVec2(x, 0.0f), ImVec2(x, display.y),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.16f, 0.94f, 0.96f, 0.05f)));
    }
    for (float y = 0.0f; y < display.y; y += 64.0f) {
        dl->AddLine(ImVec2(0.0f, y), ImVec2(display.x, y),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.78f, 0.22f, 0.04f)));
    }

    for (int i = 0; i < 36; ++i) {
        const float phase = timeSeconds * (0.25f + 0.015f * i) + i * 0.7f;
        const float x = std::fmod(display.x * (0.08f + 0.023f * i) + phase * 28.0f, display.x + 120.0f) - 60.0f;
        const float y = display.y * (0.12f + 0.022f * (i % 14)) + std::sin(phase * 1.3f) * (12.0f + (i % 6) * 4.0f);
        const float radius = 1.4f + (i % 4) * 0.9f + (std::sin(phase * 2.1f) + 1.0f) * 0.4f;
        const ImU32 color = (i % 3 == 0) ? NeonAmber(0.18f) : ((i % 3 == 1) ? NeonCyan(0.16f) : NeonPink(0.12f));
        dl->AddCircleFilled(ImVec2(x, y), radius, color, 12);
    }

    for (int i = 0; i < 10; ++i) {
        const float phase = timeSeconds * (0.35f + i * 0.03f) + i;
        const float x = display.x * (0.10f + 0.085f * i) + std::sin(phase) * 24.0f;
        const float y = display.y * (0.70f + 0.02f * (i % 3)) + std::cos(phase * 1.4f) * 18.0f;
        const float r = 18.0f + std::sin(phase * 1.6f) * 4.0f;
        dl->AddCircle(ImVec2(x, y), r, ImGui::ColorConvertFloat4ToU32(ImVec4(0.16f, 0.94f, 0.96f, 0.08f)), 32, 1.0f);
    }
}

inline void DrawNeonFrame(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                          ImU32 color, float timeSeconds, float rounding = 18.0f,
                          float thickness = 1.6f) {
    const float pulse = 0.55f + 0.45f * std::sin(timeSeconds * 2.6f);
    ImVec4 base = ImGui::ColorConvertU32ToFloat4(color);

    for (int i = 0; i < 3; ++i) {
        const float expand = 3.0f + i * 3.0f;
        const float alpha = (0.11f - i * 0.025f) * pulse;
        dl->AddRect(ImVec2(min.x - expand, min.y - expand),
                    ImVec2(max.x + expand, max.y + expand),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(base.x, base.y, base.z, alpha)),
                    rounding + expand, 0, thickness + i);
    }

    dl->AddRect(min, max, color, rounding, 0, thickness);

    const float sweep = std::fmod(timeSeconds * 140.0f, (max.x - min.x) + 120.0f);
    const ImVec2 sweepMin(min.x + sweep - 80.0f, min.y);
    const ImVec2 sweepMax(min.x + sweep, max.y);
    dl->PushClipRect(min, max, true);
    dl->AddRectFilledMultiColor(
        sweepMin, sweepMax,
        ImGui::ColorConvertFloat4ToU32(ImVec4(base.x, base.y, base.z, 0.0f)),
        ImGui::ColorConvertFloat4ToU32(ImVec4(base.x, base.y, base.z, 0.22f)),
        ImGui::ColorConvertFloat4ToU32(ImVec4(base.x, base.y, base.z, 0.22f)),
        ImGui::ColorConvertFloat4ToU32(ImVec4(base.x, base.y, base.z, 0.0f)));
    dl->PopClipRect();
}

inline void DrawCornerAccents(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                              ImU32 color, float len = 26.0f, float thickness = 3.0f) {
    dl->AddLine(min, ImVec2(min.x + len, min.y), color, thickness);
    dl->AddLine(min, ImVec2(min.x, min.y + len), color, thickness);
    dl->AddLine(ImVec2(max.x - len, min.y), ImVec2(max.x, min.y), color, thickness);
    dl->AddLine(ImVec2(max.x, min.y), ImVec2(max.x, min.y + len), color, thickness);
    dl->AddLine(ImVec2(min.x, max.y - len), ImVec2(min.x, max.y), color, thickness);
    dl->AddLine(ImVec2(min.x, max.y), ImVec2(min.x + len, max.y), color, thickness);
    dl->AddLine(ImVec2(max.x - len, max.y), max, color, thickness);
    dl->AddLine(ImVec2(max.x, max.y - len), max, color, thickness);
}

inline void DrawPanelAmbient(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                             float timeSeconds, ImU32 accent, float alphaScale = 1.0f) {
    const float w = max.x - min.x;
    const float h = max.y - min.y;
    const ImVec4 accentF = ImGui::ColorConvertU32ToFloat4(accent);

    dl->AddRectFilledMultiColor(
        min, max,
        ImGui::ColorConvertFloat4ToU32(ImVec4(accentF.x, accentF.y, accentF.z, 0.06f * alphaScale)),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.02f, 0.03f, 0.05f, 0.00f)),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.02f, 0.03f, 0.05f, 0.00f)),
        ImGui::ColorConvertFloat4ToU32(ImVec4(accentF.x, accentF.y, accentF.z, 0.04f * alphaScale)));

    for (int i = 0; i < 12; ++i) {
        const float phase = timeSeconds * (0.55f + 0.04f * i) + i * 0.8f;
        const float x = min.x + std::fmod(w * (0.12f + 0.071f * i) + phase * 22.0f, w + 80.0f);
        const float y = min.y + h * (0.14f + 0.061f * (i % 7)) + std::sin(phase) * (6.0f + i);
        dl->AddCircleFilled(ImVec2(x, y), 1.2f + (i % 3), ImGui::ColorConvertFloat4ToU32(ImVec4(accentF.x, accentF.y, accentF.z, 0.18f * alphaScale)), 10);
    }

    for (int i = 0; i < 6; ++i) {
        const float phase = timeSeconds * (0.8f + i * 0.08f) + i;
        const float cx = min.x + w * (0.18f + 0.13f * i);
        const float cy = min.y + h * (0.18f + 0.10f * (i % 3));
        const float rx = 18.0f + 8.0f * i + std::sin(phase) * 3.0f;
        const float ry = rx * 0.48f;
        dl->AddEllipse(ImVec2(cx, cy), ImVec2(rx, ry),
                       ImGui::ColorConvertFloat4ToU32(ImVec4(accentF.x, accentF.y, accentF.z, 0.12f * alphaScale)),
                       32, 0.0f, 1.0f);
    }

    for (int i = 0; i < 8; ++i) {
        const float phase = timeSeconds * 0.9f + i;
        const float y = min.y + 18.0f + i * 14.0f + std::sin(phase) * 2.5f;
        dl->AddLine(ImVec2(min.x + 12.0f, y), ImVec2(max.x - 12.0f, y),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(accentF.x, accentF.y, accentF.z, 0.03f * alphaScale)));
    }
}

}  // namespace RetroTheme
