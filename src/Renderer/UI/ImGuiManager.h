#pragma once
class Device;
class RenderPassEncoder;
class UIRenderer;

class ImGuiManager
{
public:
   static bool CreateInstance(GLFWwindow* window);
   static ImGuiManager& GetInstance();
   static void DestroyInstance();

private:
   static ImGuiManager* _instance;
   GLFWwindow* _window;
   WGPUTextureFormat _renderTargetFormat = WGPUTextureFormat_Undefined;

   // UI renderers
   std::vector<UIRenderer*> _uiRenderers;

   ImGuiManager(GLFWwindow* window);
   ~ImGuiManager();

public:
   void Configure(Device* device, WGPUTextureFormat renderTargetFormat);
   void Shutdown();

   void Render(CommandEncoder* encoder, TextureView* surfaceTextureView);

   void AddUIRenderer(UIRenderer* uiRenderer);
   bool RemoveUIRenderer(UIRenderer* uiRenderer);

private:
   void SetupStyle();

   void NewFrame();
   void EndFrame(RenderPassEncoder* encoder);
   void RenderUI();
};

