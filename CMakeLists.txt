cmake_minimum_required(VERSION 3.20)
project(VolumetricClouds)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)

# Define path variables
set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_ROOT ${REPO_ROOT}/src)
set(SHADER_ROOT ${REPO_ROOT}/shaders)
set(LIBS_ROOT ${REPO_ROOT}/libs)
set(BIN_ROOT ${REPO_ROOT}/bin)

# Dependencies
add_subdirectory(${LIBS_ROOT}/glfw)
add_subdirectory(${LIBS_ROOT}/glm)
add_subdirectory(${LIBS_ROOT}/webgpu)
add_subdirectory(${LIBS_ROOT}/glfw3webgpu)
add_subdirectory(${LIBS_ROOT}/imgui)

# Source files
set(SOURCE_FILES
   ${SRC_ROOT}/Application/Application.cpp
   ${SRC_ROOT}/Application/Editor.cpp
   ${SRC_ROOT}/Application/Input.cpp

   ${SRC_ROOT}/Renderer/Clouds/CloudRenderer.cpp
   ${SRC_ROOT}/Renderer/UI/ImGuiManager.cpp
   ${SRC_ROOT}/Renderer/BindGroup.cpp
   ${SRC_ROOT}/Renderer/Buffer.cpp
   ${SRC_ROOT}/Renderer/CommandBuffer.cpp
   ${SRC_ROOT}/Renderer/CommandEncoder.cpp
   ${SRC_ROOT}/Renderer/Device.cpp
   ${SRC_ROOT}/Renderer/GridRenderer.cpp
   ${SRC_ROOT}/Renderer/Queue.cpp
   ${SRC_ROOT}/Renderer/Renderer.cpp
   ${SRC_ROOT}/Renderer/RenderPassEncoder.cpp
   ${SRC_ROOT}/Renderer/Surface.cpp
   ${SRC_ROOT}/Renderer/TextureView.cpp

   ${SRC_ROOT}/Utils/FileReader.cpp
   ${SRC_ROOT}/Utils/FreeCamera.cpp

   ${SRC_ROOT}/main.cpp

   ${SRC_ROOT}/pch.cpp
)

# Group source files
source_group(TREE ${REPO_ROOT} FILES ${SOURCE_FILES})

# Set compiler flags
# if (MSVC)
#     add_compile_options(VolumetricClouds PRIVATE /W4 /WX)
# else()
#     add_compile_options(VolumetricClouds PRIVATE -Wall -Wextra -pedantic)
# endif()

if (XCODE)
    add_compile_options(VolumetricClouds PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
    )
endif()

# Compile shader files
# Function to compile GLSL shaders
function(compile_shader SHADER_PATH)
   get_filename_component(SHADER_NAME ${SHADER_PATH} NAME_WE)
   set(SPIRV_OUTPUT ${BIN_ROOT}/shaders/${SHADER_NAME}.spv)

   add_custom_command(
      OUTPUT ${SPIRV_OUTPUT}
      COMMAND ${GLSLANG_VALIDATOR} -V ${SHADER_PATH} -o ${SPIRV_OUTPUT}
      DEPENDS ${SHADER_PATH}
      COMMENT "Compiling shader: ${SHADER_PATH}"
   )

   set(${SHADER_NAME}_SPIRV ${SPIRV_OUTPUT} PARENT_SCOPE)
endfunction()
# Find glslangValidator
find_program(GLSLANG_VALIDATOR NAMES glslangValidator HINTS $ENV{VULKAN_SDK}/Bin)
if(NOT GLSLANG_VALIDATOR)
   message(FATAL_ERROR "glslangValidator not found! Install Vulkan SDK or glslang-tools.")
endif()
# Compile shaders
file(GLOB SHADER_FILES "${SHADER_ROOT}/*.vert" "${SHADER_ROOT}/*.frag")
foreach(SHADER ${SHADER_FILES})
   compile_shader(${SHADER})
endforeach()

# Executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${SHADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_ROOT})
target_link_libraries(${PROJECT_NAME} PRIVATE webgpu glfw glm glfw3webgpu imgui)

# Enable precompiled headers
target_precompile_headers(${PROJECT_NAME} PRIVATE ${SRC_ROOT}/pch.h)

# Copy webgpu binaries
target_copy_webgpu_binaries(${PROJECT_NAME})