cmake_minimum_required(VERSION 3.15)
project(ImGuizmo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ImGuizmo STATIC
        ImGuizmo.cpp
)

# Add ImGuizmo include directory
target_include_directories(ImGuizmo PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Assume the parent project already added ImGui — tell ImGuizmo where to find it
target_include_directories(ImGuizmo PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../imgui
)

# Enable GLM-style math operators
target_compile_definitions(ImGuizmo PUBLIC IMGUI_DEFINE_MATH_OPERATORS)