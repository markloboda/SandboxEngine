#pragma once

// External libraries.
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#pragma warning(push)
#pragma warning(disable: 26495)
#include <imgui/backends/imgui_impl_wgpu.h>
#pragma warning(pop)

#include <GLFW/glfw3.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
using namespace glm;

// Core components.
#include <Core/Core.h>
#include <Core/Node.h>
#include <Core/Scene.h>

// Helpers.
#include <Application/Input.h>

// Renderer components.
#include <Renderer/Components/BindGroup.h>
#include <Renderer/Components/Buffer.h>
#include <Renderer/Components/CommandBuffer.h>
#include <Renderer/Components/CommandEncoder.h>
#include <Renderer/Components/ComputePassEncoder.h>
#include <Renderer/Components/ComputePipeline.h>
#include <Renderer/Components/Device.h>
#include <Renderer/Components/Queue.h>
#include <Renderer/Components/RenderPassEncoder.h>
#include <Renderer/Components/RenderPipeline.h>
#include <Renderer/Components/Sampler.h>
#include <Renderer/Components/ShaderModule.h>
#include <Renderer/Components/Surface.h>
#include <Renderer/Components/Queue.h>
#include <Renderer/Components/Texture.h>
#include <Renderer/Components/TextureView.h>