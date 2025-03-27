#pragma once
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