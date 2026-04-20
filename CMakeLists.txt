cmake_minimum_required(VERSION 3.10)
project(CGV_Project)

set(CMAKE_CXX_STANDARD 17)

# ================================================================
#  Include directories
# ================================================================
include_directories(
    include
    third_party/imgui
    third_party/imgui/backends
    third_party/glfw/include     # Windows: GLFW headers from third_party
    third_party/glad/include     # GLAD (if used)
)

# ================================================================
#  Source files
# ================================================================
file(GLOB_RECURSE SOURCES "src/*.cpp")

file(GLOB IMGUI_SOURCES
    "third_party/imgui/*.cpp"
    "third_party/imgui/backends/imgui_impl_glfw.cpp"
    "third_party/imgui/backends/imgui_impl_opengl3.cpp"
)

# ================================================================
#  Executable
# ================================================================
add_executable(CGV_Project ${SOURCES} ${IMGUI_SOURCES})

# ================================================================
#  Platform-specific linking
# ================================================================
if(WIN32)
    # --- Windows (MinGW or MSVC) ---
    # Look for the pre-built GLFW library inside third_party
    find_library(GLFW_LIB
        NAMES glfw3 glfw
        PATHS
            "${CMAKE_SOURCE_DIR}/third_party/glfw/lib-mingw-w64"
            "${CMAKE_SOURCE_DIR}/third_party/glfw/lib-vc2022"
            "${CMAKE_SOURCE_DIR}/third_party/glfw/lib-vc2019"
            "${CMAKE_SOURCE_DIR}/third_party/glfw/lib"
    )

    if(NOT GLFW_LIB)
        message(FATAL_ERROR
            "GLFW library not found in third_party/glfw/lib-mingw-w64/\n"
            "Download GLFW from https://www.glfw.org/download.html\n"
            "and place libglfw3.a (MinGW) or glfw3.lib (MSVC) in the correct sub-folder.")
    endif()

    target_link_libraries(CGV_Project
        ${GLFW_LIB}
        opengl32          # Windows OpenGL
    )

elseif(APPLE)
    # --- macOS ---
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(CGV_Project glfw OpenGL::GL)

else()
    # --- Linux ---
    target_link_libraries(CGV_Project glfw GL m dl pthread)
endif()