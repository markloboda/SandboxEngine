cmake_minimum_required(VERSION 3.20)
project(VolumetricClouds)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_WARNING_AS_ERROR ON)

# Define path variables
set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_ROOT ${REPO_ROOT}/src)
set(SHADER_ROOT ${REPO_ROOT}/shaders)
set(LIBS_ROOT ${REPO_ROOT}/libs)
set(BIN_ROOT ${CMAKE_BINARY_DIR})

# Dependencies
add_subdirectory(${LIBS_ROOT}/FastNoiseLite)
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

  ${SRC_ROOT}/Renderer/Clouds/CloudModel.cpp
  ${SRC_ROOT}/Renderer/Clouds/CloudRenderer.cpp
  ${SRC_ROOT}/Renderer/Components/BindGroup.cpp
  ${SRC_ROOT}/Renderer/Components/Buffer.cpp
  ${SRC_ROOT}/Renderer/Components/CommandBuffer.cpp
  ${SRC_ROOT}/Renderer/Components/CommandEncoder.cpp
  ${SRC_ROOT}/Renderer/Components/ComputePassEncoder.cpp
  ${SRC_ROOT}/Renderer/Components/ComputePipeline.cpp
  ${SRC_ROOT}/Renderer/Components/Device.cpp
  ${SRC_ROOT}/Renderer/Components/Queue.cpp
  ${SRC_ROOT}/Renderer/Components/RenderPassEncoder.cpp
  ${SRC_ROOT}/Renderer/Components/RenderPipeline.cpp
  ${SRC_ROOT}/Renderer/Components/Sampler.cpp
  ${SRC_ROOT}/Renderer/Components/ShaderModule.cpp
  ${SRC_ROOT}/Renderer/Components/Surface.cpp
  ${SRC_ROOT}/Renderer/Components/Texture.cpp
  ${SRC_ROOT}/Renderer/Components/TextureView.cpp
  ${SRC_ROOT}/Renderer/UI/ImGuiManager.cpp
  ${SRC_ROOT}/Renderer/GridRenderer.cpp
  ${SRC_ROOT}/Renderer/Renderer.cpp

  ${SRC_ROOT}/Utils/FileReader.cpp
  ${SRC_ROOT}/Utils/FreeCamera.cpp

  ${SRC_ROOT}/main.cpp
  ${SRC_ROOT}/pch.cpp
)

# Group source files
source_group(TREE ${REPO_ROOT} FILES ${SOURCE_FILES})

# Set compiler flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
if (MSVC)
  add_compile_options(/W4 /WX)
  add_compile_options(/Zi /Od)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
  add_compile_options(-g -O0)
endif()
endif()

# Compile shader files
function(compile_shader SHADER_PATH)
  get_filename_component(SHADER_NAME ${SHADER_PATH} NAME)
  set(SPIRV_OUTPUT "${BIN_ROOT}/shaders/${SHADER_NAME}")
  
  # Create output directory
  file(MAKE_DIRECTORY "${BIN_ROOT}/shaders")

  add_custom_command(
   OUTPUT ${SPIRV_OUTPUT}
   COMMAND ${GLSLANG_VALIDATOR} -V "${SHADER_PATH}" -o "${SPIRV_OUTPUT}"
   DEPENDS "${SHADER_PATH}"
   COMMENT "Compiling shader: ${SHADER_NAME}"
   VERBATIM
  )
  
  # Add to global list of shader outputs
  set(SHADER_SPIRV_FILES ${SHADER_SPIRV_FILES} ${SPIRV_OUTPUT} PARENT_SCOPE)
endfunction()

# Find glslangValidator
find_program(GLSLANG_VALIDATOR 
  NAMES glslangValidator
  HINTS "$ENV{VULKAN_SDK}/Bin"
  REQUIRED
)

# Compile all shaders
file(GLOB SHADER_FILES 
  "${SHADER_ROOT}/*.vert" 
  "${SHADER_ROOT}/*.frag"
  "${SHADER_ROOT}/*.comp"
)

foreach(SHADER ${SHADER_FILES})
  compile_shader(${SHADER})
endforeach()

# Create custom target that depends on all shader outputs
add_custom_target(Shaders ALL DEPENDS ${SHADER_SPIRV_FILES})

# Create executable with shader outputs as dependencies
add_executable(${PROJECT_NAME} 
  ${SOURCE_FILES} 
  ${SHADER_FILES}
  ${SHADER_SPIRV_FILES}  # Explicitly list SPIR-V outputs
)

target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_ROOT})
target_link_libraries(${PROJECT_NAME} PRIVATE webgpu glfw glm glfw3webgpu imgui FastNoiseLite)

# Enable precompiled headers
target_precompile_headers(${PROJECT_NAME} PRIVATE ${SRC_ROOT}/pch.h)

# Copy webgpu binaries
target_copy_webgpu_binaries(${PROJECT_NAME})