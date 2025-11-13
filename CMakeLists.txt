cmake_minimum_required(VERSION 3.15)
project(PlantGrow VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find packages
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Option to enable/disable USD support
option(USE_USD "Enable USD export support" ON)

# Option to enable/disable OpenVDB (for later phases)
option(USE_OPENVDB "Enable OpenVDB junction blending" OFF)

# Option to build GUI application
option(BUILD_GUI "Build GUI application" ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/external/json/include)
include_directories(${CMAKE_SOURCE_DIR}/external/glm)

# GLM (header-only, will be added as submodule or downloaded)
# JSON (header-only, will be added as submodule or downloaded)

# USD support (optional)
if(USE_USD)
    find_package(pxr QUIET)
    if(pxr_FOUND)
        add_definitions(-DUSE_USD)
        message(STATUS "USD support enabled")
    else()
        message(WARNING "USD not found, will use fallback export")
    endif()
endif()

# OpenVDB support (for Phase 7)
if(USE_OPENVDB)
    find_package(OpenVDB QUIET)
    if(OpenVDB_FOUND)
        add_definitions(-DUSE_OPENVDB)
        message(STATUS "OpenVDB support enabled")
    endif()
endif()

# Source files
set(CORE_SOURCES
    src/core/lsystem.cpp
    src/core/tree.cpp
    src/core/branch.cpp
    src/core/config.cpp
    src/core/tropism.cpp
    src/core/resources.cpp
)

set(EXPORT_SOURCES
    src/export/usd_exporter.cpp
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${EXPORT_SOURCES}
    src/main.cpp
)

# Main executable
add_executable(plantgrow ${ALL_SOURCES})

# Link libraries
target_link_libraries(plantgrow PRIVATE)

if(pxr_FOUND)
    target_link_libraries(plantgrow PRIVATE ${PXR_LIBRARIES})
    target_include_directories(plantgrow PRIVATE ${PXR_INCLUDE_DIRS})
endif()

# Python bindings (for future phases)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)
    if(pybind11_FOUND)
        pybind11_add_module(pyplantgrow src/bindings/python_bindings.cpp)
        target_link_libraries(pyplantgrow PRIVATE ${CORE_SOURCES} ${EXPORT_SOURCES})
    endif()
endif()

# Installation
install(TARGETS plantgrow DESTINATION bin)

# GUI Application
if(BUILD_GUI)
    message(STATUS "Building GUI application")

    # Find GUI dependencies
    find_package(OpenGL QUIET)
    find_package(glfw3 QUIET)

    if(NOT OpenGL_FOUND)
        message(WARNING "OpenGL not found. GUI build disabled.")
        message(STATUS "Note: OpenGL is required for GUI. On macOS it should be available by default.")
        set(BUILD_GUI OFF)
    endif()

    if(NOT glfw3_FOUND)
        message(WARNING "GLFW not found. Install with: brew install glfw")
        message(WARNING "GUI build disabled")
        set(BUILD_GUI OFF)
    endif()

    if(BUILD_GUI)
        # ImGui sources (will be added to external/)
        set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
        set(IMGUI_SOURCES
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_demo.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
            ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        )

        # GUI sources
        set(GUI_SOURCES
            src/gui/camera.cpp
            src/gui/renderer.cpp
            src/gui/viewer.cpp
            src/gui/main_gui.cpp
        )

        # GUI executable
        add_executable(plantgrow_gui
            ${CORE_SOURCES}
            ${EXPORT_SOURCES}
            ${GUI_SOURCES}
            ${IMGUI_SOURCES}
        )

        # Include ImGui
        target_include_directories(plantgrow_gui PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)

        # Link GUI libraries
        target_link_libraries(plantgrow_gui PRIVATE
            OpenGL::GL
            glfw
        )

        if(pxr_FOUND)
            target_link_libraries(plantgrow_gui PRIVATE ${PXR_LIBRARIES})
            target_include_directories(plantgrow_gui PRIVATE ${PXR_INCLUDE_DIRS})
        endif()

        # Installation
        install(TARGETS plantgrow_gui DESTINATION bin)

        message(STATUS "GUI application will be built as: plantgrow_gui")
    endif()
endif()
