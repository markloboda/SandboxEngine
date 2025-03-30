#pragma once

// External libraries.
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_wgpu.h>

#include <GLFW/glfw3.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
using namespace glm;

// Core components.
#include <Core/Core.h>

// Helpers.
#include <Application/Input.h>

// Renderer components.
#include <Renderer/BindGroup.h>
#include <Renderer/Buffer.h>
#include <Renderer/CommandBuffer.h>
#include <Renderer/CommandEncoder.h>
#include <Renderer/Device.h>
#include <Renderer/Queue.h>
#include <Renderer/RenderPassEncoder.h>
#include <Renderer/ShaderModule.h>
#include <Renderer/Surface.h>
#include <Renderer/Queue.h>
#include <Renderer/TextureView.h>