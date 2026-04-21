cmake_minimum_required(VERSION 3.10)
project(DSA_Visualizer)

set(CMAKE_CXX_STANDARD 17)

#  Include directories
include_directories(
    include
    include/DSA
    third_party/imgui
    third_party/imgui/backends
    third_party/glfw/include    
    third_party/glad/include    
)

#  Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

file(GLOB IMGUI_SOURCES
    "third_party/imgui/*.cpp"
    "third_party/imgui/backends/imgui_impl_glfw.cpp"
    "third_party/imgui/backends/imgui_impl_opengl3.cpp"
)

#  Executable
add_executable(DSA_Visualizer ${SOURCES} ${IMGUI_SOURCES})

#  Platform-specific linking
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

    target_link_libraries(DSA_Visualizer
        ${GLFW_LIB}
        opengl32          
    )

elseif(APPLE)
    # --- macOS ---
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(DSA_Visualizer glfw OpenGL::GL)

else()
    # --- Linux ---
    target_link_libraries(DSA_Visualizer glfw GL m dl pthread)
endif()

