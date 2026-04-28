#include "UI/RetroTheme.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr float kPi = 3.14159265358979323846f;

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
};

struct Mat4 {
    float m[16];
};

struct VertexPC {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
};

struct PipelineState {
    Vec3 translate {0.0f, 0.0f, 0.0f};
    Vec3 rotateDeg {24.0f, 36.0f, 0.0f};
    Vec3 scale {1.0f, 1.0f, 1.0f};
    float cameraYawDeg = 28.0f;
    float cameraPitchDeg = 18.0f;
    float cameraDistance = 7.4f;
    float fovDeg = 52.0f;
    float nearPlane = 0.7f;
    float farPlane = 20.0f;
    bool showGrid = true;
    bool showAxes = true;
    bool showSolid = true;
    bool showWireframe = true;
    bool showCameraRay = true;
    bool autoSpin = true;
    bool animateCamera = true;
    bool pulseScale = true;
    int activeShape = 0;
    int stageFocus = 2;
    int selectedVertex = 0;
};

enum StageIndex {
    StageObject = 0,
    StageWorld = 1,
    StageView = 2,
    StageClip = 3,
    StageScreen = 4
};

struct ShapeData {
    const char* name;
    std::vector<Vec3> vertices;
    std::vector<std::array<int, 3>> faces;
    std::vector<std::array<int, 2>> edges;
};

struct RenderStageData {
    std::vector<Vec3> vertices;
    std::vector<std::array<int, 2>> edges;
};

bool IsInsideClipVolume(const Vec4& clip) {
    const float w = std::fabs(clip.w);
    return std::fabs(clip.x) <= w &&
           std::fabs(clip.y) <= w &&
           std::fabs(clip.z) <= w;
}

Mat4 Identity() {
    Mat4 out {};
    out.m[0] = out.m[5] = out.m[10] = out.m[15] = 1.0f;
    return out;
}

Mat4 Multiply(const Mat4& a, const Mat4& b) {
    Mat4 out {};
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a.m[k * 4 + row] * b.m[col * 4 + k];
            }
            out.m[col * 4 + row] = sum;
        }
    }
    return out;
}

Vec4 Multiply(const Mat4& m, const Vec4& v) {
    return {
        m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w,
        m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w,
        m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w,
        m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w
    };
}

Mat4 Translate(const Vec3& t) {
    Mat4 out = Identity();
    out.m[12] = t.x;
    out.m[13] = t.y;
    out.m[14] = t.z;
    return out;
}

Mat4 Scale(const Vec3& s) {
    Mat4 out {};
    out.m[0] = s.x;
    out.m[5] = s.y;
    out.m[10] = s.z;
    out.m[15] = 1.0f;
    return out;
}

Mat4 RotateX(float radians) {
    Mat4 out = Identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    out.m[5] = c;
    out.m[6] = s;
    out.m[9] = -s;
    out.m[10] = c;
    return out;
}

Mat4 RotateY(float radians) {
    Mat4 out = Identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    out.m[0] = c;
    out.m[2] = -s;
    out.m[8] = s;
    out.m[10] = c;
    return out;
}

Mat4 RotateZ(float radians) {
    Mat4 out = Identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    out.m[0] = c;
    out.m[1] = s;
    out.m[4] = -s;
    out.m[5] = c;
    return out;
}

Vec3 Sub(const Vec3& a, const Vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

float Dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 Cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 Normalize(const Vec3& v) {
    const float len = std::sqrt(std::max(1e-8f, Dot(v, v)));
    return {v.x / len, v.y / len, v.z / len};
}

Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    const Vec3 f = Normalize(Sub(center, eye));
    const Vec3 s = Normalize(Cross(f, up));
    const Vec3 u = Cross(s, f);

    Mat4 out = Identity();
    out.m[0] = s.x;
    out.m[1] = u.x;
    out.m[2] = -f.x;
    out.m[4] = s.y;
    out.m[5] = u.y;
    out.m[6] = -f.y;
    out.m[8] = s.z;
    out.m[9] = u.z;
    out.m[10] = -f.z;
    out.m[12] = -Dot(s, eye);
    out.m[13] = -Dot(u, eye);
    out.m[14] = Dot(f, eye);
    return out;
}

Mat4 Perspective(float fovDeg, float aspect, float nearPlane, float farPlane) {
    const float f = 1.0f / std::tan((fovDeg * kPi / 180.0f) * 0.5f);
    Mat4 out {};
    out.m[0] = f / aspect;
    out.m[5] = f;
    out.m[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    out.m[11] = -1.0f;
    out.m[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
    return out;
}

std::string FormatVec4(const Vec4& v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3)
        << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return oss.str();
}

std::string FormatVec3(const Vec3& v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3)
        << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return oss.str();
}

std::string FormatMat4(const Mat4& m) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    for (int row = 0; row < 4; ++row) {
        oss << "[ ";
        for (int col = 0; col < 4; ++col) {
            oss << std::setw(6) << m.m[col * 4 + row] << ' ';
        }
        oss << "]";
        if (row < 3) {
            oss << '\n';
        }
    }
    return oss.str();
}

GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, '\0');
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        std::cerr << "Shader compile failed:\n" << log << '\n';
    }
    return shader;
}

GLuint CreateProgram(const char* vs, const char* fs) {
    const GLuint vert = CompileShader(GL_VERTEX_SHADER, vs);
    const GLuint frag = CompileShader(GL_FRAGMENT_SHADER, fs);
    const GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, '\0');
        glGetProgramInfoLog(program, logLen, nullptr, log.data());
        std::cerr << "Program link failed:\n" << log << '\n';
    }
    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

std::array<ShapeData, 2> BuildShapes() {
    ShapeData cube;
    cube.name = "Cube";
    cube.vertices = {
        Vec3{-1.0f, -1.0f, -1.0f}, Vec3{ 1.0f, -1.0f, -1.0f},
        Vec3{ 1.0f,  1.0f, -1.0f}, Vec3{-1.0f,  1.0f, -1.0f},
        Vec3{-1.0f, -1.0f,  1.0f}, Vec3{ 1.0f, -1.0f,  1.0f},
        Vec3{ 1.0f,  1.0f,  1.0f}, Vec3{-1.0f,  1.0f,  1.0f}
    };
    cube.faces = {
        std::array<int, 3>{0, 1, 2}, {2, 3, 0},
        {4, 5, 6}, {6, 7, 4},
        {0, 4, 7}, {7, 3, 0},
        {1, 5, 6}, {6, 2, 1},
        {3, 2, 6}, {6, 7, 3},
        {0, 1, 5}, {5, 4, 0}
    };
    cube.edges = {
        std::array<int, 2>{0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    ShapeData pyramid;
    pyramid.name = "Pyramid";
    pyramid.vertices = {
        Vec3{-1.2f, -1.0f, -1.2f}, Vec3{1.2f, -1.0f, -1.2f},
        Vec3{1.2f, -1.0f, 1.2f}, Vec3{-1.2f, -1.0f, 1.2f},
        Vec3{0.0f, 1.3f, 0.0f}
    };
    pyramid.faces = {
        std::array<int, 3>{0, 1, 2}, {2, 3, 0},
        {0, 1, 4}, {1, 2, 4}, {2, 3, 4}, {3, 0, 4}
    };
    pyramid.edges = {
        std::array<int, 2>{0,1}, {1,2}, {2,3}, {3,0},
        {0,4}, {1,4}, {2,4}, {3,4}
    };

    return {cube, pyramid};
}

Vec3 ToVec3(const Vec4& v) {
    return {v.x, v.y, v.z};
}

std::vector<Vec3> ComputeFaceNormals(const ShapeData& shape,
                                     const std::vector<Vec3>& verts) {
    std::vector<Vec3> normals;
    normals.reserve(shape.faces.size());
    for (const auto& face : shape.faces) {
        const Vec3 a = verts[face[0]];
        const Vec3 b = verts[face[1]];
        const Vec3 c = verts[face[2]];
        normals.push_back(Normalize(Cross(Sub(b, a), Sub(c, a))));
    }
    return normals;
}

RenderStageData BuildObjectStage(const ShapeData& shape) {
    RenderStageData out;
    out.vertices = shape.vertices;
    out.edges = shape.edges;
    return out;
}

RenderStageData BuildWorldStage(const ShapeData& shape,
                                const std::vector<Vec4>& worldPoints) {
    RenderStageData out;
    out.edges = shape.edges;
    out.vertices.reserve(worldPoints.size());
    for (const Vec4& v : worldPoints) {
        out.vertices.push_back(ToVec3(v));
    }
    return out;
}

RenderStageData BuildViewStage(const ShapeData& shape,
                               const std::vector<Vec4>& viewPoints) {
    RenderStageData out;
    out.edges = shape.edges;
    out.vertices.reserve(viewPoints.size());
    for (const Vec4& v : viewPoints) {
        out.vertices.push_back(ToVec3(v));
    }
    return out;
}

RenderStageData BuildClipStage(const ShapeData& shape,
                               const std::vector<Vec4>& clipPoints) {
    RenderStageData out;
    out.edges = shape.edges;
    out.vertices.reserve(clipPoints.size());
    for (const Vec4& v : clipPoints) {
        const float safeW = std::max(0.001f, std::fabs(v.w));
        out.vertices.push_back({v.x / safeW, v.y / safeW, v.z / safeW});
    }
    return out;
}

RenderStageData BuildScreenStage(const ShapeData& shape,
                                 const std::vector<Vec4>& screenPoints,
                                 float width,
                                 float height) {
    RenderStageData out;
    out.edges = shape.edges;
    out.vertices.reserve(screenPoints.size());
    const float invW = width > 0.0f ? 2.0f / width : 0.0f;
    const float invH = height > 0.0f ? 2.0f / height : 0.0f;
    for (const Vec4& v : screenPoints) {
        out.vertices.push_back({
            -1.0f + v.x * invW,
            1.0f - v.y * invH,
            std::clamp(v.z, -1.0f, 1.0f)
        });
    }
    return out;
}

void PushLine(std::vector<VertexPC>& verts, const Vec3& a, const Vec3& b,
              float r, float g, float bl) {
    verts.push_back({a.x, a.y, a.z, r, g, bl});
    verts.push_back({b.x, b.y, b.z, r, g, bl});
}

void PushTriangle(std::vector<VertexPC>& verts, const Vec3& a, const Vec3& b, const Vec3& c,
                  float r, float g, float bl) {
    verts.push_back({a.x, a.y, a.z, r, g, bl});
    verts.push_back({b.x, b.y, b.z, r, g, bl});
    verts.push_back({c.x, c.y, c.z, r, g, bl});
}

void DrawMatrixBlock(const char* title, const Mat4& mat, ImU32 accent) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(accent));
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.86f, 0.92f, 1.0f));
    ImGui::TextUnformatted(FormatMat4(mat).c_str());
    ImGui::PopStyleColor();
}

void DrawStageDiagram(const char* label,
                      const std::vector<Vec4>& points,
                      const std::vector<std::array<int, 2>>& edges,
                      StageIndex stage,
                      int selectedVertex,
                      ImU32 accent) {
    ImGui::BeginChild(label, ImVec2(0.0f, 160.0f), true);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const ImVec2 p1(p0.x + avail.x, p0.y + avail.y);
    RetroTheme::DrawPanelAmbient(dl, ImGui::GetWindowPos(),
                                 ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x,
                                        ImGui::GetWindowPos().y + ImGui::GetWindowSize().y),
                                 (float)ImGui::GetTime() * 0.9f, accent, 0.45f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(accent));
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::Spacing();

    const float xMin = p0.x + 16.0f;
    const float xMax = p1.x - 16.0f;
    const float yMin = p0.y + 46.0f;
    const float yMax = p1.y - 16.0f;
    const float cx = (xMin + xMax) * 0.5f;
    const float cy = (yMin + yMax) * 0.5f;
    const float scale = std::min(xMax - xMin, yMax - yMin) * 0.22f;

    auto project = [&](const Vec4& v) {
        Vec3 q {};
        if (stage == StageClip) {
            const float safeW = std::max(0.001f, std::fabs(v.w));
            q = {v.x / safeW, v.y / safeW, v.z / safeW};
        } else if (stage == StageScreen) {
            q = {v.x, v.y, v.z};
        } else {
            q = {v.x, v.y, v.z};
        }
        return ImVec2(cx + q.x * scale, cy - q.y * scale);
    };

    dl->AddRect(ImVec2(xMin, yMin), ImVec2(xMax, yMax), accent, 10.0f, 0, 1.0f);
    dl->AddLine(ImVec2(cx, yMin), ImVec2(cx, yMax), IM_COL32(90, 110, 130, 80), 1.0f);
    dl->AddLine(ImVec2(xMin, cy), ImVec2(xMax, cy), IM_COL32(90, 110, 130, 80), 1.0f);

    for (const auto& edge : edges) {
        const ImVec2 a = project(points[edge[0]]);
        const ImVec2 b = project(points[edge[1]]);
        dl->AddLine(a, b, IM_COL32(145, 220, 255, 170), 2.0f);
    }
    for (int i = 0; i < (int)points.size(); ++i) {
        const ImVec2 p = project(points[i]);
        const bool active = i == selectedVertex;
        dl->AddCircleFilled(p, active ? 6.0f : 4.0f,
                            active ? RetroTheme::NeonAmber(0.95f) : IM_COL32(235, 245, 255, 210), 12);
        dl->AddText(ImVec2(p.x + 8.0f, p.y - 8.0f), IM_COL32(220, 228, 240, 220), std::to_string(i).c_str());
    }
    ImGui::EndChild();
}

}  // namespace

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1680, 980, "3D Graphics Pipeline Visualizer", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    RetroTheme::ApplyRetroTheme(1.90f, 1.22f);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    static const char* kVertSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;
        uniform mat4 uMVP;
        out vec3 vColor;
        void main() {
            gl_Position = uMVP * vec4(aPos, 1.0);
            vColor = aColor;
        }
    )";

    static const char* kFragSrc = R"(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vColor, 1.0);
        }
    )";

    const GLuint program = CreateProgram(kVertSrc, kFragSrc);
    const GLint mvpLoc = glGetUniformLocation(program, "uMVP");

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPC) * 16384, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPC), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPC), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    const auto shapes = BuildShapes();
    PipelineState state;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd))) {
            io.FontGlobalScale = std::min(3.0f, io.FontGlobalScale + 0.1f);
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract))) {
            io.FontGlobalScale = std::max(0.7f, io.FontGlobalScale - 0.1f);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImVec2 viewportCanvasMin {};
        ImVec2 viewportCanvasAvail {};
        Vec3 eyeForOverlay {};

        const float now = (float)glfwGetTime();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.015f, 0.020f, 0.045f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
        ImGui::Begin("PipelineLab", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoBringToFrontOnFocus |
                     ImGuiWindowFlags_NoBackground);

        const ImVec2 display = io.DisplaySize;
        const float pad = 24.0f;
        const float sidebarW = std::clamp(display.x * 0.34f, 520.0f, 700.0f);
        const float headerH = 146.0f;

        ImGui::SetCursorScreenPos(ImVec2(pad, pad));
        ImGui::BeginChild("##StageSidebar", ImVec2(sidebarW, display.y - pad * 2.0f), true);
        ImDrawList* sidebarDl = ImGui::GetWindowDrawList();
        const ImVec2 sidebarMin = ImGui::GetWindowPos();
        const ImVec2 sidebarMax(sidebarMin.x + ImGui::GetWindowSize().x,
                                sidebarMin.y + ImGui::GetWindowSize().y);
        RetroTheme::DrawNeonFrame(sidebarDl, sidebarMin, sidebarMax, RetroTheme::NeonCyan(0.95f), now, 18.0f, 1.6f);
        RetroTheme::DrawPanelAmbient(sidebarDl, sidebarMin, sidebarMax, now, RetroTheme::NeonCyan(0.95f), 0.9f);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.22f, 1.0f));
        ImGui::SetWindowFontScale(1.15f);
        ImGui::TextUnformatted("HOLOGRAPHIC 3D LAB");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("model  ->  world  ->  view  ->  clip  ->  screen");

        ImGui::SetWindowFontScale(1.34f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.16f, 0.94f, 0.96f, 1.0f));
        ImGui::TextWrapped("Trace one vertex through the complete graphics pipeline.");
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::TextWrapped("This module lets you orbit the camera, distort the projection, pulse the model, and inspect how clipping and screen mapping react in real time.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const char* shapesList[] = {"Cube", "Pyramid"};
        ImGui::Combo("Shape", &state.activeShape, shapesList, IM_ARRAYSIZE(shapesList));
        ImGui::SliderInt("Focus Stage", &state.stageFocus, 0, 4, state.stageFocus == 0 ? "Object" :
                                                              state.stageFocus == 1 ? "World" :
                                                              state.stageFocus == 2 ? "View" :
                                                              state.stageFocus == 3 ? "Clip" : "Screen");
        ImGui::SliderInt("Tracked Vertex", &state.selectedVertex, 0, (int)shapes[state.activeShape].vertices.size() - 1);

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.22f, 1.0f));
        ImGui::TextUnformatted("MODEL TRANSFORM");
        ImGui::PopStyleColor();
        ImGui::SliderFloat3("Translate", &state.translate.x, -3.5f, 3.5f, "%.2f");
        ImGui::SliderFloat3("Scale", &state.scale.x, 0.35f, 2.40f, "%.2f");
        ImGui::SliderFloat3("Rotate", &state.rotateDeg.x, -180.0f, 180.0f, "%.0f deg");

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.16f, 0.94f, 0.96f, 1.0f));
        ImGui::TextUnformatted("CAMERA + PROJECTION");
        ImGui::PopStyleColor();
        ImGui::SliderFloat("Yaw", &state.cameraYawDeg, -180.0f, 180.0f, "%.0f deg");
        ImGui::SliderFloat("Pitch", &state.cameraPitchDeg, -70.0f, 70.0f, "%.0f deg");
        ImGui::SliderFloat("Distance", &state.cameraDistance, 3.0f, 12.0f, "%.2f");
        ImGui::SliderFloat("FOV", &state.fovDeg, 28.0f, 100.0f, "%.0f deg");
        ImGui::SliderFloat("Near", &state.nearPlane, 0.15f, 3.0f, "%.2f");
        ImGui::SliderFloat("Far", &state.farPlane, 6.0f, 35.0f, "%.1f");

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.98f, 0.30f, 0.68f, 1.0f));
        ImGui::TextUnformatted("INTERACTIVE LAYERS");
        ImGui::PopStyleColor();
        ImGui::Checkbox("Auto spin model", &state.autoSpin);
        ImGui::SameLine();
        ImGui::Checkbox("Orbit camera", &state.animateCamera);
        ImGui::Checkbox("Pulse scale", &state.pulseScale);
        ImGui::SameLine();
        ImGui::Checkbox("Show grid", &state.showGrid);
        ImGui::Checkbox("Show axes", &state.showAxes);
        ImGui::SameLine();
        ImGui::Checkbox("Solid mesh", &state.showSolid);
        ImGui::Checkbox("Wireframe edges", &state.showWireframe);
        ImGui::SameLine();
        ImGui::Checkbox("Camera ray", &state.showCameraRay);

        const float orbitOffset = state.animateCamera ? std::sin(now * 0.45f) * 22.0f : 0.0f;
        const float spinOffset = state.autoSpin ? now * 30.0f : 0.0f;
        const float pulse = state.pulseScale ? (1.0f + std::sin(now * 1.4f) * 0.08f) : 1.0f;
        const Vec3 effectiveScale {
            state.scale.x * pulse,
            state.scale.y * pulse,
            state.scale.z * pulse
        };
        const Vec3 effectiveRotateDeg {
            state.rotateDeg.x,
            state.rotateDeg.y + spinOffset,
            state.rotateDeg.z
        };

        const ShapeData& shape = shapes[state.activeShape];
        const Vec3 target {0.0f, 0.0f, 0.0f};
        const float yaw = (state.cameraYawDeg + orbitOffset) * kPi / 180.0f;
        const float pitch = state.cameraPitchDeg * kPi / 180.0f;
        const Vec3 eye {
            std::cos(yaw) * std::cos(pitch) * state.cameraDistance,
            std::sin(pitch) * state.cameraDistance,
            std::sin(yaw) * std::cos(pitch) * state.cameraDistance
        };
        Mat4 model = Multiply(Translate(state.translate),
                              Multiply(RotateZ(effectiveRotateDeg.z * kPi / 180.0f),
                                       Multiply(RotateY(effectiveRotateDeg.y * kPi / 180.0f),
                                                Multiply(RotateX(effectiveRotateDeg.x * kPi / 180.0f),
                                                         Scale(effectiveScale)))));
        Mat4 view = LookAt(eye, target, {0.0f, 1.0f, 0.0f});
        const float viewportW = display.x - sidebarW - pad * 3.0f;
        const float viewportH = display.y - headerH - pad * 2.0f;
        Mat4 proj = Perspective(state.fovDeg, viewportW / viewportH, state.nearPlane, state.farPlane);
        Mat4 vp = Multiply(proj, view);

        std::vector<Vec4> objectPoints;
        std::vector<Vec4> worldPoints;
        std::vector<Vec4> viewPoints;
        std::vector<Vec4> clipPoints;
        std::vector<Vec4> screenPoints;
        std::vector<bool> insideClip;
        objectPoints.reserve(shape.vertices.size());
        worldPoints.reserve(shape.vertices.size());
        viewPoints.reserve(shape.vertices.size());
        clipPoints.reserve(shape.vertices.size());
        screenPoints.reserve(shape.vertices.size());
        insideClip.reserve(shape.vertices.size());

        for (const Vec3& v : shape.vertices) {
            const Vec4 object {v.x, v.y, v.z, 1.0f};
            const Vec4 world = Multiply(model, object);
            const Vec4 cam = Multiply(view, world);
            const Vec4 clip = Multiply(proj, cam);
            const float invW = 1.0f / std::max(0.0001f, std::fabs(clip.w));
            const Vec4 ndc {clip.x * invW, clip.y * invW, clip.z * invW, 1.0f};
            const Vec4 screen {
                (ndc.x * 0.5f + 0.5f) * viewportW,
                (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportH,
                ndc.z,
                1.0f
            };
            objectPoints.push_back(object);
            worldPoints.push_back(world);
            viewPoints.push_back(cam);
            clipPoints.push_back(clip);
            screenPoints.push_back(screen);
            insideClip.push_back(IsInsideClipVolume(clip));
        }

        ImGui::Spacing();
        DrawStageDiagram("Object Space", objectPoints, shape.edges, StageObject, state.selectedVertex, RetroTheme::NeonAmber(0.88f));
        DrawStageDiagram("View / Clip Space", clipPoints, shape.edges, StageClip, state.selectedVertex, RetroTheme::NeonCyan(0.88f));
        DrawStageDiagram("Screen Mapping", screenPoints, shape.edges, StageScreen, state.selectedVertex, RetroTheme::NeonPink(0.88f));

        ImGui::Spacing();
        ImGui::BeginChild("##Inspector", ImVec2(0.0f, 280.0f), true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.22f, 1.0f));
        ImGui::TextUnformatted("VERTEX TRACE");
        ImGui::PopStyleColor();
        ImGui::TextWrapped("Tracked vertex %d moves through every stage below. If clip-space w changes sign or grows tiny, perspective division becomes unstable and clipping matters immediately.", state.selectedVertex);
        ImGui::Separator();
        ImGui::BulletText("Object: %s", FormatVec4(objectPoints[state.selectedVertex]).c_str());
        ImGui::BulletText("World:  %s", FormatVec4(worldPoints[state.selectedVertex]).c_str());
        ImGui::BulletText("View:   %s", FormatVec4(viewPoints[state.selectedVertex]).c_str());
        ImGui::BulletText("Clip:   %s", FormatVec4(clipPoints[state.selectedVertex]).c_str());
        ImGui::BulletText("Screen: (%.1f px, %.1f px, %.3f)",
                          screenPoints[state.selectedVertex].x,
                          screenPoints[state.selectedVertex].y,
                          screenPoints[state.selectedVertex].z);
        ImGui::BulletText("Clip Test: %s", insideClip[state.selectedVertex] ? "inside clip volume" : "outside clip volume");
        ImGui::Spacing();
        DrawMatrixBlock("Model Matrix", model, RetroTheme::NeonAmber(0.9f));
        ImGui::Spacing();
        DrawMatrixBlock("View Matrix", view, RetroTheme::NeonCyan(0.9f));
        ImGui::Spacing();
        DrawMatrixBlock("Projection Matrix", proj, RetroTheme::NeonPink(0.9f));
        ImGui::EndChild();

        ImGui::EndChild();

        const ImVec2 viewportMin(sidebarW + pad * 2.0f, pad);
        const ImVec2 viewportSize(display.x - sidebarW - pad * 3.0f, display.y - pad * 2.0f);
        ImGui::SetCursorScreenPos(viewportMin);
        ImGui::BeginChild("##ViewportPanel", viewportSize, true,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
        ImDrawList* viewDl = ImGui::GetWindowDrawList();
        const ImVec2 panelMin = ImGui::GetWindowPos();
        const ImVec2 panelMax(panelMin.x + ImGui::GetWindowSize().x,
                              panelMin.y + ImGui::GetWindowSize().y);
        RetroTheme::DrawNeonFrame(viewDl, panelMin, panelMax, RetroTheme::NeonPink(0.92f), now + 0.35f, 18.0f, 1.45f);
        RetroTheme::DrawPanelAmbient(viewDl, panelMin, panelMax, now, RetroTheme::NeonPink(0.78f), 0.18f);

        ImGui::SetWindowFontScale(1.14f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.16f, 0.94f, 0.96f, 1.0f));
        ImGui::TextUnformatted("LIVE 3D VIEWPORT");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("camera orbit + stage-aware overlays");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.82f, 0.86f, 0.92f, 1.0f));
        const char* stageMessage[] = {
            "Object stage: geometry is still in local model coordinates.",
            "World stage: transforms place the shape into scene space.",
            "View stage: camera basis re-expresses coordinates around the eye.",
            "Clip stage: projection reshapes the frustum before perspective divide.",
            "Screen stage: NDC becomes pixel coordinates for the viewport."
        };
        ImGui::TextWrapped("%s", stageMessage[state.stageFocus]);
        ImGui::PopStyleColor();

        const ImVec2 canvasMin = ImGui::GetCursorScreenPos();
        const ImVec2 canvasAvail = ImGui::GetContentRegionAvail();
        viewportCanvasMin = canvasMin;
        viewportCanvasAvail = canvasAvail;
        eyeForOverlay = eye;

        std::vector<VertexPC> lineVerts;
        std::vector<VertexPC> triVerts;
        std::vector<VertexPC> pointVerts;
        lineVerts.reserve(4096);
        triVerts.reserve(4096);
        pointVerts.reserve(256);

        if (state.showGrid) {
            for (int i = -10; i <= 10; ++i) {
                const float z = (float)i;
                PushLine(lineVerts, {-10.0f, -1.2f, z}, {10.0f, -1.2f, z}, 0.18f, 0.34f, 0.46f);
                PushLine(lineVerts, {(float)i, -1.2f, -10.0f}, {(float)i, -1.2f, 10.0f}, 0.18f, 0.34f, 0.46f);
            }
        }
        if (state.showAxes) {
            PushLine(lineVerts, {0.0f, 0.0f, 0.0f}, {2.6f, 0.0f, 0.0f}, 1.0f, 0.24f, 0.28f);
            PushLine(lineVerts, {0.0f, 0.0f, 0.0f}, {0.0f, 2.6f, 0.0f}, 0.28f, 1.0f, 0.45f);
            PushLine(lineVerts, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 2.6f}, 0.28f, 0.56f, 1.0f);
        }
        if (state.showCameraRay) {
            PushLine(lineVerts, eye, target, 0.98f, 0.76f, 0.18f);
        }

        RenderStageData objectStage = BuildObjectStage(shape);
        RenderStageData worldStage = BuildWorldStage(shape, worldPoints);
        RenderStageData viewStage = BuildViewStage(shape, viewPoints);
        RenderStageData clipStage = BuildClipStage(shape, clipPoints);
        RenderStageData screenStage = BuildScreenStage(shape, screenPoints, viewportW, viewportH);

        const RenderStageData* focusedStage = &worldStage;
        Mat4 stageMVP = vp;
        bool stageUsesSolidFaces = state.showSolid;
        bool stageUses3DGrid = state.showGrid;
        bool stageUsesAxes = state.showAxes;
        bool stageUsesCameraRay = state.showCameraRay;

        if (state.stageFocus == StageObject) {
            focusedStage = &objectStage;
            const Vec3 debugEye {4.4f, 3.0f, 4.6f};
            stageMVP = Multiply(proj, LookAt(debugEye, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
            stageUsesCameraRay = false;
        } else if (state.stageFocus == StageWorld) {
            focusedStage = &worldStage;
            const Vec3 debugEye {5.0f, 3.2f, 5.4f};
            stageMVP = Multiply(proj, LookAt(debugEye, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
        } else if (state.stageFocus == StageView) {
            focusedStage = &viewStage;
            const Vec3 debugEye {3.2f, 2.2f, 5.6f};
            stageMVP = Multiply(proj, LookAt(debugEye, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
            stageUsesCameraRay = false;
        } else if (state.stageFocus == StageClip) {
            focusedStage = &clipStage;
            stageMVP = Identity();
            stageUses3DGrid = false;
        } else if (state.stageFocus == StageScreen) {
            focusedStage = &screenStage;
            stageMVP = Identity();
            stageUses3DGrid = false;
        }

        if (stageUses3DGrid) {
            for (int i = -10; i <= 10; ++i) {
                const float z = (float)i;
                PushLine(lineVerts, {-10.0f, -1.2f, z}, {10.0f, -1.2f, z}, 0.18f, 0.34f, 0.46f);
                PushLine(lineVerts, {(float)i, -1.2f, -10.0f}, {(float)i, -1.2f, 10.0f}, 0.18f, 0.34f, 0.46f);
            }
        } else if (state.stageFocus == StageClip) {
            const float cube = 1.0f;
            const std::array<Vec3, 8> clipBox = {{
                {-cube, -cube, -cube}, { cube, -cube, -cube}, { cube,  cube, -cube}, {-cube,  cube, -cube},
                {-cube, -cube,  cube}, { cube, -cube,  cube}, { cube,  cube,  cube}, {-cube,  cube,  cube}
            }};
            const std::array<std::array<int, 2>, 12> clipEdges = {{
                {{0,1}}, {{1,2}}, {{2,3}}, {{3,0}},
                {{4,5}}, {{5,6}}, {{6,7}}, {{7,4}},
                {{0,4}}, {{1,5}}, {{2,6}}, {{3,7}}
            }};
            for (const auto& edge : clipEdges) {
                PushLine(lineVerts, clipBox[edge[0]], clipBox[edge[1]], 0.34f, 0.82f, 1.0f);
            }
            if (state.showGrid) {
                for (int i = -1; i <= 1; ++i) {
                    const float t = i * 0.5f;
                    PushLine(lineVerts, {-1.0f, t, -1.0f}, {1.0f, t, -1.0f}, 0.20f, 0.45f, 0.66f);
                    PushLine(lineVerts, {-1.0f, t,  1.0f}, {1.0f, t,  1.0f}, 0.20f, 0.45f, 0.66f);
                    PushLine(lineVerts, {t, -1.0f, -1.0f}, {t, 1.0f, -1.0f}, 0.20f, 0.45f, 0.66f);
                    PushLine(lineVerts, {t, -1.0f,  1.0f}, {t, 1.0f,  1.0f}, 0.20f, 0.45f, 0.66f);
                    PushLine(lineVerts, {-1.0f, -1.0f, t}, {-1.0f, 1.0f, t}, 0.20f, 0.45f, 0.66f);
                    PushLine(lineVerts, { 1.0f, -1.0f, t}, { 1.0f, 1.0f, t}, 0.20f, 0.45f, 0.66f);
                }
            }
        } else if (state.stageFocus == StageScreen) {
            PushLine(lineVerts, {-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, 0.34f, 0.82f, 1.0f);
            PushLine(lineVerts, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, 0.34f, 0.82f, 1.0f);
            PushLine(lineVerts, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, 0.34f, 0.82f, 1.0f);
            PushLine(lineVerts, {-1.0f, 1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}, 0.34f, 0.82f, 1.0f);
            if (state.showAxes) {
                PushLine(lineVerts, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 0.22f, 0.48f, 0.72f);
                PushLine(lineVerts, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.22f, 0.48f, 0.72f);
            }
            if (state.showGrid) {
                for (int i = -4; i <= 4; ++i) {
                    const float t = i / 4.0f;
                    PushLine(lineVerts, {-1.0f, t, 0.0f}, {1.0f, t, 0.0f}, 0.18f, 0.34f, 0.46f);
                    PushLine(lineVerts, {t, -1.0f, 0.0f}, {t, 1.0f, 0.0f}, 0.18f, 0.34f, 0.46f);
                }
            }
        }

        const std::vector<Vec3>& focusedVerts = focusedStage->vertices;

        if (stageUsesAxes && state.stageFocus != StageScreen) {
            PushLine(lineVerts, {0.0f, 0.0f, 0.0f}, {2.6f, 0.0f, 0.0f}, 1.0f, 0.24f, 0.28f);
            PushLine(lineVerts, {0.0f, 0.0f, 0.0f}, {0.0f, 2.6f, 0.0f}, 0.28f, 1.0f, 0.45f);
            PushLine(lineVerts, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 2.6f}, 0.28f, 0.56f, 1.0f);
        }
        if (stageUsesCameraRay) {
            if (state.stageFocus == StageView) {
                PushLine(lineVerts, {0.0f, 0.0f, 0.0f}, focusedVerts[state.selectedVertex], 0.98f, 0.76f, 0.18f);
            } else if (state.stageFocus == StageClip || state.stageFocus == StageScreen) {
                PushLine(lineVerts, {0.0f, 0.0f, 0.0f}, focusedVerts[state.selectedVertex], 0.98f, 0.76f, 0.18f);
            } else {
                PushLine(lineVerts, eye, target, 0.98f, 0.76f, 0.18f);
            }
        }

        if (stageUsesSolidFaces) {
            const auto faceNormals = ComputeFaceNormals(shape, focusedVerts);
            for (size_t i = 0; i < shape.faces.size(); ++i) {
                const auto& f = shape.faces[i];
                const Vec3& n = faceNormals[i];
                const float light = std::clamp(0.35f + Dot(n, Normalize(Vec3{0.4f, 0.8f, 0.3f})) * 0.55f, 0.18f, 1.0f);
                const bool clippedFace =
                    !insideClip[f[0]] || !insideClip[f[1]] || !insideClip[f[2]];
                const float tint = (state.stageFocus == StageClip && clippedFace) ? 0.55f : 1.0f;
                PushTriangle(triVerts, focusedVerts[f[0]], focusedVerts[f[1]], focusedVerts[f[2]],
                             std::min(1.0f, (0.22f + light * 0.62f) * tint),
                             std::min(1.0f, (0.34f + light * 0.54f) * tint),
                             std::min(1.0f, (0.52f + light * 0.42f) * tint));
            }
        }
        if (state.showWireframe) {
            for (const auto& edge : focusedStage->edges) {
                const bool active = edge[0] == state.selectedVertex || edge[1] == state.selectedVertex;
                PushLine(lineVerts, focusedVerts[edge[0]], focusedVerts[edge[1]],
                         1.0f,
                         active ? 0.90f : 0.98f,
                         active ? 0.16f : 1.0f);
            }
        }
        for (size_t i = 0; i < focusedVerts.size(); ++i) {
                const bool active = (int)i == state.selectedVertex;
                const bool clipped = state.stageFocus == StageClip && !insideClip[(int)i];
                pointVerts.push_back({
                    focusedVerts[i].x, focusedVerts[i].y, focusedVerts[i].z,
                    clipped ? 1.0f : (active ? 1.0f : 0.95f),
                    clipped ? 0.32f : (active ? 0.88f : 0.97f),
                    clipped ? 0.32f : (active ? 0.14f : 1.0f)
                });
        }

        ImDrawList* overlay = ImGui::GetWindowDrawList();
        const ImVec2 contentStart = canvasMin;
        const ImVec2 contentEnd(canvasMin.x + canvasAvail.x, canvasMin.y + canvasAvail.y);
        overlay->AddRect(contentStart, contentEnd, RetroTheme::NeonCyan(0.22f), 12.0f, 0, 1.0f);
        const char* overlayTitle[] = {
            "scene: object coordinates",
            "scene: world-transformed object",
            "scene: camera/view coordinates",
            "scene: clip volume after perspective divide",
            "scene: final screen-space mapping"
        };
        overlay->AddText(ImVec2(contentStart.x + 18.0f, contentStart.y + 18.0f),
                         RetroTheme::NeonAmber(0.95f), overlayTitle[state.stageFocus]);
        overlay->AddText(ImVec2(contentStart.x + 18.0f, contentStart.y + 46.0f),
                         IM_COL32(220, 228, 240, 220),
                         ("eye " + FormatVec3(eye)).c_str());
        ImGui::Dummy(canvasAvail);

        ImGui::EndChild();
        ImGui::End();

        int fbWidth = 0;
        int fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        const float scaleX = io.DisplaySize.x > 0.0f ? (float)fbWidth / io.DisplaySize.x : 1.0f;
        const float scaleY = io.DisplaySize.y > 0.0f ? (float)fbHeight / io.DisplaySize.y : 1.0f;
        const int vpX = std::max(0, (int)(viewportCanvasMin.x * scaleX));
        const int vpW = std::max(1, (int)(viewportCanvasAvail.x * scaleX));
        const int vpH = std::max(1, (int)(viewportCanvasAvail.y * scaleY));
        const int vpY = std::max(0, fbHeight - (int)((viewportCanvasMin.y + viewportCanvasAvail.y) * scaleY));
        
        glEnable(GL_SCISSOR_TEST);
        glViewport(vpX, vpY, vpW, vpH);
        glScissor(vpX, vpY, vpW, vpH);
        glClearColor(0.02f, 0.03f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(vao);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, stageMVP.m);

        if (!triVerts.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(triVerts.size() * sizeof(VertexPC)), triVerts.data());
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)triVerts.size());
        }
        if (!lineVerts.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(lineVerts.size() * sizeof(VertexPC)), lineVerts.data());
            glDrawArrays(GL_LINES, 0, (GLsizei)lineVerts.size());
        }
        if (!pointVerts.empty()) {
            glPointSize(12.0f);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(pointVerts.size() * sizeof(VertexPC)), pointVerts.data());
            glDrawArrays(GL_POINTS, 0, (GLsizei)pointVerts.size());
        }

        glBindVertexArray(0);
        glUseProgram(0);
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, fbWidth, fbHeight);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
