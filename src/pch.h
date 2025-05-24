#pragma once

// External libraries.
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#pragma warning(push)
#pragma warning(disable: 26495)
#include <imgui/backends/imgui_impl_wgpu.h>
#pragma warning(pop)
#include <imguizmo/imguizmo.h>

#include <GLFW/glfw3.h>

#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
using namespace glm;

// Debug break define.
#if defined(_MSC_VER)
// MSVC (Windows)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__linux__) || defined(__APPLE__)
// Linux and macOS
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#else
#define DEBUG_BREAK() ((void)0)
#endif

// Core components.
#include <Core/Core.h>
#include <Core/Math.h>
#include <Core/Node.h>
#include <Core/Scene.h>

// Helpers.
#include <Application/Input.h>

// Renderer components.
#include <Renderer/Components/BindGroup.h>
#include <Renderer/Components/Buffer.h>
#include <Renderer/Components/CommandBuffer.h>
#include <Renderer/Components/ComputePassEncoder.h>
#include <Renderer/Components/ComputePipeline.h>
#include <Renderer/Components/Device.h>
#include <Renderer/Components/QuerySet.h>
#include <Renderer/Components/Queue.h>
#include <Renderer/Components/RenderPassEncoder.h>
#include <Renderer/Components/RenderPipeline.h>
#include <Renderer/Components/Sampler.h>
#include <Renderer/Components/ShaderModule.h>
#include <Renderer/Components/Surface.h>
#include <Renderer/Components/Queue.h>
#include <Renderer/Components/Texture.h>
#include <Renderer/Components/TextureView.h>
#include <Renderer/Components/CommandEncoder.h>

#include <Renderer/Renderer.h>