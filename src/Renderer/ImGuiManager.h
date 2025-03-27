#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_wgpu.h>

class Device;
class RenderPassEncoder;

class ImGuiManager
{
public:
   ImGuiManager(GLFWwindow* window);
   ~ImGuiManager();

   void Configure(Device* device, WGPUTextureFormat renderTargetFormat);
   void Shutdown();

   void NewFrame();
   void EndFrame(RenderPassEncoder* encoder);

   void RenderUI();

private:
   GLFWwindow* _window;
   WGPUTextureFormat _renderTargetFormat = WGPUTextureFormat_Undefined;

   void SetupStyle();
};