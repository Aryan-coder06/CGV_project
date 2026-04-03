#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr float kNearPlane = 0.1f;
constexpr float kFarPlane = 750000.0f;
constexpr float kGridSize = 22000.0f;
constexpr int kGridDivisions = 28;
constexpr std::size_t kMaxTrailPoints = 140;
constexpr float kCameraZoomStep = 600.0f;
constexpr float kVelocityVectorScale = 0.18f;
constexpr float kPreviewDistance = 1800.0f;
constexpr float kMaxSimulationSpeed = 8.0f;
constexpr float kMinSimulationSpeed = 0.25f;
constexpr double kHudRefreshSeconds = 0.12;

constexpr double G = 6.6743e-11;   // m^3 kg^-1 s^-2
constexpr float c = 299792458.0f;  // m/s
float initMass = 1.0e22f;
float sizeRatio = 30000.0f;

const char* kWindowTitle = "Gravity Simulator";

const char* vertexShaderSource = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out float lightIntensity;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vec3 worldPos = (model * vec4(aPos, 1.0)).xyz;
    vec3 normal = normalize(aPos);
    vec3 dirToCenter = normalize(-worldPos);
    lightIntensity = max(dot(normal, dirToCenter), 0.15);
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
in float lightIntensity;
out vec4 FragColor;
uniform vec4 objectColor;
uniform bool isGrid;
uniform bool GLOW;
void main() {
    if (isGrid) {
        FragColor = objectColor;
    } else if (GLOW) {
        FragColor = vec4(objectColor.rgb * 2.0, objectColor.a);
    } else {
        float fade = smoothstep(0.0, 10.0, lightIntensity * 10.0);
        FragColor = vec4(objectColor.rgb * fade, objectColor.a);
    }
}
)glsl";

bool running = true;
bool pauseSimulation = false;
bool showGrid = true;
bool showTrails = true;
bool showVelocityVectors = true;
bool showCenterOfMassMarker = true;

float simulationSpeed = 1.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float yaw = -90.0f;
float pitch = -8.0f;
float lastX = static_cast<float>(kWindowWidth) * 0.5f;
float lastY = static_cast<float>(kWindowHeight) * 0.5f;
bool firstMouse = true;

glm::vec3 cameraPos(0.0f, 1800.0f, 7000.0f);
glm::vec3 cameraFront(0.0f, -0.15f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

GLuint activeShaderProgram = 0;
GLuint gridVAO = 0;
GLuint gridVBO = 0;
GLuint lineVAO = 0;
GLuint lineVBO = 0;

std::vector<float> baseGridVertices;
double lastHudUpdate = 0.0;
int selectedObjectIndex = -1;
int customBodyCounter = 1;

struct Object {
    std::string name;
    GLuint VAO = 0;
    GLuint VBO = 0;
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    std::size_t vertexCount = 0;
    glm::vec4 color{1.0f, 0.0f, 0.0f, 1.0f};

    bool initializing = false;
    bool launched = false;
    bool glow = false;

    float mass = initMass;
    float density = 3344.0f;
    float radius = 1.0f;

    std::vector<glm::vec3> trail;

    Object(std::string bodyName,
           glm::vec3 initPosition,
           glm::vec3 initVelocity,
           float initMassValue,
           float initDensity = 3344.0f,
           glm::vec4 bodyColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
           bool isGlowing = false)
        : name(std::move(bodyName)),
          position(initPosition),
          velocity(initVelocity),
          color(bodyColor),
          glow(isGlowing),
          mass(initMassValue),
          density(initDensity) {
        UpdateRadius();
        RebuildMesh();
        trail.push_back(position);
    }

    void UpdateRadius() {
        const float volume = (3.0f * mass / density) / (4.0f * glm::pi<float>());
        radius = std::cbrt(std::max(volume, 0.001f)) / sizeRatio;
    }

    std::vector<float> BuildSphereVertices() const {
        std::vector<float> vertices;
        constexpr int stacks = 14;
        constexpr int sectors = 14;
        vertices.reserve(stacks * sectors * 18);

        for (int i = 0; i <= stacks; ++i) {
            const float theta1 = (static_cast<float>(i) / stacks) * glm::pi<float>();
            const float theta2 = (static_cast<float>(i + 1) / stacks) * glm::pi<float>();

            for (int j = 0; j < sectors; ++j) {
                const float phi1 = (static_cast<float>(j) / sectors) * glm::two_pi<float>();
                const float phi2 = (static_cast<float>(j + 1) / sectors) * glm::two_pi<float>();

                const glm::vec3 v1(radius * std::sin(theta1) * std::cos(phi1),
                                   radius * std::cos(theta1),
                                   radius * std::sin(theta1) * std::sin(phi1));
                const glm::vec3 v2(radius * std::sin(theta1) * std::cos(phi2),
                                   radius * std::cos(theta1),
                                   radius * std::sin(theta1) * std::sin(phi2));
                const glm::vec3 v3(radius * std::sin(theta2) * std::cos(phi1),
                                   radius * std::cos(theta2),
                                   radius * std::sin(theta2) * std::sin(phi1));
                const glm::vec3 v4(radius * std::sin(theta2) * std::cos(phi2),
                                   radius * std::cos(theta2),
                                   radius * std::sin(theta2) * std::sin(phi2));

                vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z});
                vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, v4.x, v4.y, v4.z, v3.x, v3.y, v3.z});
            }
        }

        return vertices;
    }

    void RebuildMesh();

    void RecordTrail(bool force = false) {
        if (initializing) {
            return;
        }

        if (trail.empty() || force) {
            trail.push_back(position);
            return;
        }

        const float minimumStep = std::max(30.0f, radius * 0.3f);
        if (glm::length(position - trail.back()) >= minimumStep) {
            trail.push_back(position);
            if (trail.size() > kMaxTrailPoints) {
                trail.erase(trail.begin());
            }
        }
    }
};

std::vector<Object> objs;

GLFWwindow* StartGLU();
GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource);
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, std::size_t vertexCount);
void UpdateCam(GLuint shaderProgram, const glm::vec3& cameraPosition);
void keyCallback(GLFWwindow* window, int key, int, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int);
void scroll_callback(GLFWwindow*, double, double yoffset);
void mouse_callback(GLFWwindow*, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow*, int width, int height);

glm::vec3 ComputeCenterOfMass();
void UpdateProjection(GLuint shaderProgram, int width, int height);
void ResetCamera();
void ResetScene();
void DestroyScene();
void ClampSelectedObject();
void SelectNextObject(int direction);
void ProcessContinuousInput(GLFWwindow* window);
void HandlePreviewGrowth(GLFWwindow* window);
void StepSimulation();
void PrintControls();
void PrintSceneSummary();
void UpdateWindowTitle(GLFWwindow* window);
bool HasPreviewBody();
glm::vec4 NextCustomBodyColor();

std::vector<float> CreateGridVertices(float size, int divisions, const glm::vec3& center);
std::vector<float> UpdateGridVertices(const std::vector<float>& vertices, const std::vector<Object>& objects);
std::vector<float> BuildTrailVertices(const Object& object);
std::vector<float> BuildVelocityVectorVertices(const Object& object);
std::vector<float> BuildCrossVertices(const glm::vec3& center, float size);
void DrawDynamicLines(GLuint shaderProgram,
                      const std::vector<float>& vertices,
                      GLenum drawMode,
                      const glm::vec4& color,
                      float lineWidth);
void DrawGrid(GLuint shaderProgram, const std::vector<float>& gridVertices, GLint objectColorLoc);

void Object::RebuildMesh() {
    const std::vector<float> vertices = BuildSphereVertices();
    vertexCount = vertices.size();

    if (VAO == 0 || VBO == 0) {
        CreateVBOVAO(VAO, VBO, vertices.data(), vertexCount);
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices.data(), GL_STATIC_DRAW);
}

int main() {
    GLFWwindow* window = StartGLU();
    if (window == nullptr) {
        return 1;
    }

    activeShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (activeShaderProgram == 0) {
        glfwTerminate();
        return 1;
    }

    glUseProgram(activeShaderProgram);
    UpdateProjection(activeShaderProgram, kWindowWidth, kWindowHeight);

    float seedLine[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    CreateVBOVAO(gridVAO, gridVBO, seedLine, 6);
    CreateVBOVAO(lineVAO, lineVBO, seedLine, 6);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    PrintControls();
    ResetCamera();
    ResetScene();

    const GLint modelLoc = glGetUniformLocation(activeShaderProgram, "model");
    const GLint objectColorLoc = glGetUniformLocation(activeShaderProgram, "objectColor");

    while (!glfwWindowShouldClose(window) && running) {
        const float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = std::min(currentFrame - lastFrame, 0.05f);
        lastFrame = currentFrame;

        ProcessContinuousInput(window);
        HandlePreviewGrowth(window);
        if (!pauseSimulation) {
            StepSimulation();
        }

        UpdateWindowTitle(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        UpdateCam(activeShaderProgram, cameraPos);

        const glm::vec3 centerOfMass = ComputeCenterOfMass();
        baseGridVertices = CreateGridVertices(
            kGridSize,
            kGridDivisions,
            glm::vec3(centerOfMass.x, centerOfMass.y - 700.0f, centerOfMass.z));

        if (showGrid) {
            const std::vector<float> gridVertices = UpdateGridVertices(baseGridVertices, objs);
            DrawGrid(activeShaderProgram, gridVertices, objectColorLoc);
        }

        if (showTrails) {
            for (const auto& object : objs) {
                if (object.initializing || object.trail.size() < 2) {
                    continue;
                }
                DrawDynamicLines(
                    activeShaderProgram,
                    BuildTrailVertices(object),
                    GL_LINE_STRIP,
                    glm::vec4(object.color.r, object.color.g, object.color.b, 0.85f),
                    2.0f);
            }
        }

        if (showVelocityVectors) {
            for (const auto& object : objs) {
                if (object.initializing || glm::length(object.velocity) < 1.0f) {
                    continue;
                }
                DrawDynamicLines(
                    activeShaderProgram,
                    BuildVelocityVectorVertices(object),
                    GL_LINES,
                    glm::vec4(object.color.r, object.color.g, object.color.b, 1.0f),
                    2.0f);
            }
        }

        if (showCenterOfMassMarker) {
            DrawDynamicLines(
                activeShaderProgram,
                BuildCrossVertices(centerOfMass, 220.0f),
                GL_LINES,
                glm::vec4(1.0f, 0.95f, 0.35f, 1.0f),
                2.0f);
        }

        ClampSelectedObject();
        if (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(objs.size())) {
            const Object& selected = objs[static_cast<std::size_t>(selectedObjectIndex)];
            DrawDynamicLines(
                activeShaderProgram,
                BuildCrossVertices(selected.position, std::max(110.0f, selected.radius * 1.8f)),
                GL_LINES,
                glm::vec4(1.0f, 1.0f, 1.0f, 0.95f),
                1.0f);
        }

        for (const auto& object : objs) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), object.position);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform4f(objectColorLoc, object.color.r, object.color.g, object.color.b, object.color.a);
            glUniform1i(glGetUniformLocation(activeShaderProgram, "isGrid"), 0);
            glUniform1i(glGetUniformLocation(activeShaderProgram, "GLOW"), object.glow ? 1 : 0);
            glBindVertexArray(object.VAO);
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(object.vertexCount / 3));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    DestroyScene();
    if (lineVAO != 0) {
        glDeleteVertexArrays(1, &lineVAO);
    }
    if (lineVBO != 0) {
        glDeleteBuffers(1, &lineVBO);
    }
    if (gridVAO != 0) {
        glDeleteVertexArrays(1, &gridVAO);
    }
    if (gridVBO != 0) {
        glDeleteBuffers(1, &gridVBO);
    }
    glDeleteProgram(activeShaderProgram);
    glfwTerminate();
    return 0;
}

GLFWwindow* StartGLU() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glClearColor(0.03f, 0.04f, 0.07f, 1.0f);

    return window;
}

GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    GLint success = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }

    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }

    const GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, std::size_t vertexCount) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void UpdateCam(GLuint shaderProgram, const glm::vec3& cameraPosition) {
    glUseProgram(shaderProgram);
    const glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    const GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void UpdateProjection(GLuint shaderProgram, int width, int height) {
    const float aspectRatio = static_cast<float>(width) / static_cast<float>(std::max(height, 1));
    const glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, kNearPlane, kFarPlane);
    glUseProgram(shaderProgram);
    const GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
    if (activeShaderProgram != 0) {
        UpdateProjection(activeShaderProgram, width, height);
    }
}

void ResetCamera() {
    cameraPos = glm::vec3(0.0f, 1800.0f, 7000.0f);
    yaw = -90.0f;
    pitch = -8.0f;
    const glm::vec3 front(
        std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
        std::sin(glm::radians(pitch)),
        std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)));
    cameraFront = glm::normalize(front);
    firstMouse = true;
    lastX = static_cast<float>(kWindowWidth) * 0.5f;
    lastY = static_cast<float>(kWindowHeight) * 0.5f;
}

void DestroyScene() {
    for (auto& object : objs) {
        if (object.VAO != 0) {
            glDeleteVertexArrays(1, &object.VAO);
        }
        if (object.VBO != 0) {
            glDeleteBuffers(1, &object.VBO);
        }
    }
    objs.clear();
}

void ResetScene() {
    DestroyScene();

    objs.reserve(32);
    objs.emplace_back(
        "Left Orbiter",
        glm::vec3(-5000.0f, 650.0f, -350.0f),
        glm::vec3(0.0f, 0.0f, 1500.0f),
        5.97219f * std::pow(10.0f, 22.0f),
        5515.0f,
        glm::vec4(0.20f, 0.78f, 1.00f, 1.0f));
    objs.emplace_back(
        "Right Orbiter",
        glm::vec3(5000.0f, 650.0f, -350.0f),
        glm::vec3(0.0f, 0.0f, -1500.0f),
        5.97219f * std::pow(10.0f, 22.0f),
        5515.0f,
        glm::vec4(0.15f, 0.95f, 0.70f, 1.0f));
    objs.emplace_back(
        "Core Star",
        glm::vec3(0.0f, 0.0f, -350.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        1.989f * std::pow(10.0f, 25.0f),
        5515.0f,
        glm::vec4(1.00f, 0.84f, 0.25f, 1.0f),
        true);

    for (auto& object : objs) {
        object.RecordTrail(true);
    }

    pauseSimulation = false;
    simulationSpeed = 1.0f;
    customBodyCounter = 1;
    selectedObjectIndex = 2;
    PrintSceneSummary();
}

bool HasPreviewBody() {
    return !objs.empty() && objs.back().initializing;
}

glm::vec4 NextCustomBodyColor() {
    static const std::array<glm::vec4, 5> palette = {
        glm::vec4(0.95f, 0.45f, 0.35f, 1.0f),
        glm::vec4(0.75f, 0.45f, 1.00f, 1.0f),
        glm::vec4(1.00f, 0.65f, 0.20f, 1.0f),
        glm::vec4(0.35f, 0.95f, 0.55f, 1.0f),
        glm::vec4(0.95f, 0.95f, 0.35f, 1.0f),
    };
    return palette[static_cast<std::size_t>((customBodyCounter - 1) % static_cast<int>(palette.size()))];
}

void ClampSelectedObject() {
    if (objs.empty()) {
        selectedObjectIndex = -1;
        return;
    }

    if (selectedObjectIndex >= 0 &&
        selectedObjectIndex < static_cast<int>(objs.size()) &&
        !objs[static_cast<std::size_t>(selectedObjectIndex)].initializing) {
        return;
    }

    for (std::size_t i = 0; i < objs.size(); ++i) {
        if (!objs[i].initializing) {
            selectedObjectIndex = static_cast<int>(i);
            return;
        }
    }

    selectedObjectIndex = -1;
}

void SelectNextObject(int direction) {
    std::vector<int> selectable;
    selectable.reserve(objs.size());

    for (std::size_t i = 0; i < objs.size(); ++i) {
        if (!objs[i].initializing) {
            selectable.push_back(static_cast<int>(i));
        }
    }

    if (selectable.empty()) {
        selectedObjectIndex = -1;
        return;
    }

    const auto current = std::find(selectable.begin(), selectable.end(), selectedObjectIndex);
    if (current == selectable.end()) {
        selectedObjectIndex = selectable.front();
        return;
    }

    const int currentIndex = static_cast<int>(std::distance(selectable.begin(), current));
    const int size = static_cast<int>(selectable.size());
    const int nextIndex = (currentIndex + direction + size) % size;
    selectedObjectIndex = selectable[static_cast<std::size_t>(nextIndex)];
}

void PrintControls() {
    std::cout
        << "\nGravity Simulator controls\n"
        << "----------------------------------------\n"
        << "W/A/S/D, Space, Left Shift : move camera\n"
        << "Mouse / Scroll             : look and zoom\n"
        << "P                          : pause / resume\n"
        << "[ and ]                    : slow down / speed up time\n"
        << "G / T / V / C              : toggle grid, trails, vectors, COM marker\n"
        << "Tab                        : cycle selected body\n"
        << "R                          : reset default scene\n"
        << "H                          : print this help again\n"
        << "Q                          : quit\n"
        << "Left click                 : create a new body in front of the camera\n"
        << "Hold right click           : grow preview body mass while placing\n"
        << "Arrow keys                 : move preview body on X/Z\n"
        << "PageUp / PageDown          : move preview body on Y\n"
        << "Live HUD                   : window title shows state and selected body\n"
        << "----------------------------------------\n";
}

void PrintSceneSummary() {
    std::cout << "\nLoaded scene bodies:\n";
    for (const auto& object : objs) {
        std::cout << "  - " << object.name
                  << " | mass=" << std::scientific << object.mass
                  << " | radius=" << std::fixed << std::setprecision(2) << object.radius
                  << " | speed=" << glm::length(object.velocity)
                  << '\n';
    }
    std::cout.unsetf(std::ios::floatfield);
}

glm::vec3 ComputeCenterOfMass() {
    glm::vec3 centerOfMass(0.0f);
    double totalMass = 0.0;

    for (const auto& object : objs) {
        if (object.initializing) {
            continue;
        }
        centerOfMass += object.position * object.mass;
        totalMass += object.mass;
    }

    if (totalMass <= 0.0) {
        return glm::vec3(0.0f);
    }

    return centerOfMass / static_cast<float>(totalMass);
}

void UpdateWindowTitle(GLFWwindow* window) {
    const double now = glfwGetTime();
    if (now - lastHudUpdate < kHudRefreshSeconds) {
        return;
    }
    lastHudUpdate = now;

    ClampSelectedObject();
    const glm::vec3 centerOfMass = ComputeCenterOfMass();

    std::ostringstream hud;
    hud << kWindowTitle
        << " | " << (pauseSimulation ? "PAUSED" : "RUNNING")
        << " | speed " << std::fixed << std::setprecision(2) << simulationSpeed << "x"
        << " | bodies " << objs.size()
        << " | grid " << (showGrid ? "on" : "off")
        << " | trails " << (showTrails ? "on" : "off")
        << " | vectors " << (showVelocityVectors ? "on" : "off")
        << " | COM " << (showCenterOfMassMarker ? "on" : "off");

    if (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(objs.size())) {
        const Object& selected = objs[static_cast<std::size_t>(selectedObjectIndex)];
        hud << " | selected " << selected.name
            << " | speed " << std::setprecision(1) << glm::length(selected.velocity)
            << " | distCOM " << glm::length(selected.position - centerOfMass);
    }

    if (HasPreviewBody()) {
        const Object& preview = objs.back();
        hud << " | preview mass " << std::scientific << preview.mass;
    }

    glfwSetWindowTitle(window, hud.str().c_str());
}

void ProcessContinuousInput(GLFWwindow* window) {
    const float speedMultiplier = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ? 2.5f : 1.0f;
    const float cameraSpeed = 4200.0f * deltaTime * speedMultiplier;
    const glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraFront * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraFront * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= right * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += right * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cameraPos += cameraUp * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPos -= cameraUp * cameraSpeed;
    }
}

void HandlePreviewGrowth(GLFWwindow* window) {
    if (!HasPreviewBody()) {
        return;
    }

    Object& preview = objs.back();
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        preview.mass *= 1.0f + (0.9f * deltaTime);
        preview.UpdateRadius();
        preview.RebuildMesh();
    }
}

void StepSimulation() {
    std::vector<glm::vec3> accelerations(objs.size(), glm::vec3(0.0f));

    for (std::size_t i = 0; i < objs.size(); ++i) {
        if (objs[i].initializing) {
            continue;
        }

        for (std::size_t j = i + 1; j < objs.size(); ++j) {
            if (objs[j].initializing) {
                continue;
            }

            const glm::vec3 delta = objs[j].position - objs[i].position;
            const float distance = glm::length(delta);
            if (distance <= 1.0f) {
                continue;
            }

            const float collisionDistance = objs[i].radius + objs[j].radius;
            if (distance <= collisionDistance) {
                objs[i].velocity *= -0.2f;
                objs[j].velocity *= -0.2f;
                continue;
            }

            const glm::vec3 direction = delta / distance;
            const float distanceMeters = distance * 1000.0f;
            const double force = (G * objs[i].mass * objs[j].mass) / (distanceMeters * distanceMeters);

            accelerations[i] += direction * static_cast<float>(force / objs[i].mass);
            accelerations[j] -= direction * static_cast<float>(force / objs[j].mass);
        }
    }

    const float accelerationStep = simulationSpeed / 96.0f;
    const float movementStep = simulationSpeed / 94.0f;

    for (std::size_t i = 0; i < objs.size(); ++i) {
        if (objs[i].initializing) {
            continue;
        }
        objs[i].velocity += accelerations[i] * accelerationStep;
    }

    for (auto& object : objs) {
        if (object.initializing) {
            continue;
        }
        object.position += object.velocity * movementStep;
        object.RecordTrail();
    }
}

void keyCallback(GLFWwindow* window, int key, int, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_P:
                pauseSimulation = !pauseSimulation;
                break;
            case GLFW_KEY_G:
                showGrid = !showGrid;
                break;
            case GLFW_KEY_T:
                showTrails = !showTrails;
                break;
            case GLFW_KEY_V:
                showVelocityVectors = !showVelocityVectors;
                break;
            case GLFW_KEY_C:
                showCenterOfMassMarker = !showCenterOfMassMarker;
                break;
            case GLFW_KEY_R:
                ResetCamera();
                ResetScene();
                break;
            case GLFW_KEY_H:
                PrintControls();
                break;
            case GLFW_KEY_Q:
                running = false;
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_TAB:
                SelectNextObject((mods & GLFW_MOD_SHIFT) ? -1 : 1);
                break;
            case GLFW_KEY_LEFT_BRACKET:
            case GLFW_KEY_MINUS:
                simulationSpeed = std::max(kMinSimulationSpeed, simulationSpeed * 0.5f);
                break;
            case GLFW_KEY_RIGHT_BRACKET:
            case GLFW_KEY_EQUAL:
                simulationSpeed = std::min(kMaxSimulationSpeed, simulationSpeed * 2.0f);
                break;
            default:
                break;
        }
    }

    if (!HasPreviewBody()) {
        return;
    }

    Object& preview = objs.back();
    const float moveStep = std::max(80.0f, preview.radius * 0.4f);

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_LEFT:
                preview.position.x -= moveStep;
                break;
            case GLFW_KEY_RIGHT:
                preview.position.x += moveStep;
                break;
            case GLFW_KEY_UP:
                preview.position.z -= moveStep;
                break;
            case GLFW_KEY_DOWN:
                preview.position.z += moveStep;
                break;
            case GLFW_KEY_PAGE_UP:
                preview.position.y += moveStep;
                break;
            case GLFW_KEY_PAGE_DOWN:
                preview.position.y -= moveStep;
                break;
            default:
                break;
        }
    }
}

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    constexpr float sensitivity = 0.08f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    pitch = std::clamp(pitch, -89.0f, 89.0f);

    const glm::vec3 front(
        std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
        std::sin(glm::radians(pitch)),
        std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)));
    cameraFront = glm::normalize(front);
}

void mouseButtonCallback(GLFWwindow*, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
    }

    if (action == GLFW_PRESS && !HasPreviewBody()) {
        const glm::vec3 spawnPosition = cameraPos + cameraFront * kPreviewDistance;
        objs.emplace_back(
            "Custom Body " + std::to_string(customBodyCounter),
            spawnPosition,
            glm::vec3(0.0f),
            initMass,
            3344.0f,
            NextCustomBodyColor());
        objs.back().initializing = true;
        selectedObjectIndex = static_cast<int>(objs.size() - 1);
        ++customBodyCounter;
    } else if (action == GLFW_RELEASE && HasPreviewBody()) {
        objs.back().initializing = false;
        objs.back().launched = true;
        objs.back().RecordTrail(true);
        ClampSelectedObject();
    }
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    cameraPos += cameraFront * static_cast<float>(yoffset) * kCameraZoomStep;
}

std::vector<float> CreateGridVertices(float size, int divisions, const glm::vec3& center) {
    std::vector<float> vertices;
    const float step = size / divisions;
    const float halfSize = size * 0.5f;

    vertices.reserve(static_cast<std::size_t>(divisions * divisions * 12));

    for (int zStep = 0; zStep <= divisions; ++zStep) {
        const float z = center.z - halfSize + zStep * step;
        for (int xStep = 0; xStep < divisions; ++xStep) {
            const float xStart = center.x - halfSize + xStep * step;
            const float xEnd = xStart + step;
            vertices.insert(vertices.end(), {xStart, center.y, z, xEnd, center.y, z});
        }
    }

    for (int xStep = 0; xStep <= divisions; ++xStep) {
        const float x = center.x - halfSize + xStep * step;
        for (int zStep = 0; zStep < divisions; ++zStep) {
            const float zStart = center.z - halfSize + zStep * step;
            const float zEnd = zStart + step;
            vertices.insert(vertices.end(), {x, center.y, zStart, x, center.y, zEnd});
        }
    }

    return vertices;
}

std::vector<float> UpdateGridVertices(const std::vector<float>& vertices, const std::vector<Object>& objects) {
    std::vector<float> warped = vertices;

    for (std::size_t i = 0; i < warped.size(); i += 3) {
        const glm::vec3 vertexPos(warped[i], warped[i + 1], warped[i + 2]);
        float bend = 0.0f;

        for (const auto& object : objects) {
            if (object.initializing) {
                continue;
            }

            const float distance = std::max(glm::length(object.position - vertexPos), 100.0f);
            const float massInfluence = std::sqrt(std::max(object.mass / 1.0e22f, 0.1f));
            bend += std::min(420.0f, massInfluence * 240.0f / (distance / 1400.0f + 1.0f));
        }

        warped[i + 1] -= bend;
    }

    return warped;
}

std::vector<float> BuildTrailVertices(const Object& object) {
    std::vector<float> vertices;
    vertices.reserve(object.trail.size() * 3);
    for (const glm::vec3& point : object.trail) {
        vertices.push_back(point.x);
        vertices.push_back(point.y);
        vertices.push_back(point.z);
    }
    return vertices;
}

std::vector<float> BuildVelocityVectorVertices(const Object& object) {
    const glm::vec3 endPoint = object.position + object.velocity * kVelocityVectorScale;
    return {
        object.position.x, object.position.y, object.position.z,
        endPoint.x, endPoint.y, endPoint.z,
    };
}

std::vector<float> BuildCrossVertices(const glm::vec3& center, float size) {
    return {
        center.x - size, center.y, center.z, center.x + size, center.y, center.z,
        center.x, center.y - size, center.z, center.x, center.y + size, center.z,
        center.x, center.y, center.z - size, center.x, center.y, center.z + size,
    };
}

void DrawDynamicLines(GLuint shaderProgram,
                      const std::vector<float>& vertices,
                      GLenum drawMode,
                      const glm::vec4& color,
                      float lineWidth) {
    if (vertices.size() < 6) {
        return;
    }

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 0);
    glUniform4f(glGetUniformLocation(shaderProgram, "objectColor"), color.r, color.g, color.b, color.a);

    const glm::mat4 identity(1.0f);
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "model"),
        1,
        GL_FALSE,
        glm::value_ptr(identity));

    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(lineVAO);
    glLineWidth(lineWidth);
    glDrawArrays(drawMode, 0, static_cast<GLsizei>(vertices.size() / 3));
    glLineWidth(1.0f);
    glBindVertexArray(0);
}

void DrawGrid(GLuint shaderProgram, const std::vector<float>& gridVertices, GLint objectColorLoc) {
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 0);
    glUniform4f(objectColorLoc, 0.78f, 0.82f, 0.95f, 0.40f);

    const glm::mat4 identity(1.0f);
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "model"),
        1,
        GL_FALSE,
        glm::value_ptr(identity));

    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW);
    glBindVertexArray(gridVAO);
    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(gridVertices.size() / 3));
    glBindVertexArray(0);
}

}  // namespace
