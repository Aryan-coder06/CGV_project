cmake_minimum_required(VERSION 3.10)
project(CGV_Project)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

set(COMMON_INCLUDE_DIRS
    third_party/imgui
    third_party/imgui/backends
    third_party/glfw/include
    third_party/glad/include
)

set(CGV_MODULE_SOURCES
    src/main.cpp
    src/Core/Shader.cpp
    src/Core/Window.cpp
    src/Canvas.cpp
    src/Renderer.cpp
    src/Algorithms.cpp
    src/GraphApp/GraphGrid.cpp
    src/GraphApp/Pathfinding.cpp
    src/PaintApp/Canvas.cpp
    src/PaintApp/PaintTools.cpp
    src/Visualizer/CircleAlgos.cpp
    src/Visualizer/ClipAlgos.cpp
    src/Visualizer/EllipseAlgos.cpp
    src/Visualizer/FillAlgos.cpp
    src/Visualizer/LineAlgos.cpp
    src/Visualizer/SHAlgos.cpp
    src/Visualizer/TransformAlgos.cpp
    src/Visualizer/VisualizerEngine.cpp
    src/Visualizer/WindowViewportAlgo.cpp
)

set(ARSH_MODULE_SOURCES
    modules/arsh/src/main.cpp
    modules/arsh/src/DSA/GraphAlgos.cpp
    modules/arsh/src/DSA/SearchAlgos.cpp
    modules/arsh/src/DSA/SortAlgos.cpp
    modules/arsh/src/Visualizer/VisualizerEngine.cpp
)

set(PAINT_MODULE_SOURCES
    modules/anshdeep/src/main.cpp
    modules/anshdeep/src/Canvas.cpp
    modules/anshdeep/src/Algorithms.cpp
    modules/anshdeep/src/Renderer.cpp
)

set(LAUNCHER_SOURCES
    src/LauncherMain.cpp
)

add_executable(CGV_Module ${CGV_MODULE_SOURCES} ${IMGUI_SOURCES})
target_include_directories(CGV_Module PRIVATE include ${COMMON_INCLUDE_DIRS})

add_executable(DSA_Visualizer ${ARSH_MODULE_SOURCES} ${IMGUI_SOURCES})
target_include_directories(DSA_Visualizer PRIVATE modules/arsh/include ${COMMON_INCLUDE_DIRS})

add_executable(Paint_Module ${PAINT_MODULE_SOURCES} ${IMGUI_SOURCES})
target_include_directories(Paint_Module PRIVATE modules/anshdeep/src ${COMMON_INCLUDE_DIRS})

add_executable(final_visualize ${LAUNCHER_SOURCES} ${IMGUI_SOURCES})
target_include_directories(final_visualize PRIVATE ${COMMON_INCLUDE_DIRS})

if(WIN32)
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
            "GLFW library not found in third_party/glfw/lib-mingw-w64/.\n"
            "Place glfw3.lib or libglfw3.a in the appropriate folder.")
    endif()

    target_link_libraries(CGV_Module ${GLFW_LIB} opengl32)
    target_link_libraries(DSA_Visualizer ${GLFW_LIB} opengl32)
    target_link_libraries(Paint_Module ${GLFW_LIB} opengl32)
    target_link_libraries(final_visualize ${GLFW_LIB} opengl32)
elseif(APPLE)
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(CGV_Module glfw OpenGL::GL)
    target_link_libraries(DSA_Visualizer glfw OpenGL::GL)
    target_link_libraries(Paint_Module glfw OpenGL::GL)
    target_link_libraries(final_visualize glfw OpenGL::GL)
else()
    target_link_libraries(CGV_Module glfw GLEW GL m dl pthread)
    target_link_libraries(DSA_Visualizer glfw GLEW GL m dl pthread)
    target_link_libraries(Paint_Module glfw GLEW GL m dl pthread)
    target_link_libraries(final_visualize glfw GLEW GL m dl pthread)
endif()
