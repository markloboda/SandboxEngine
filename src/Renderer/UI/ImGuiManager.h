#pragma once
class Device;
class RenderPassEncoder;
class UIRenderer;

class ImGuiManager
{
public:
   static bool CreateInstance(Renderer* renderer);
   static void DestroyInstance();
   static ImGuiManager& GetInstance();

private:
   static ImGuiManager* _instance;
   WGPUTextureFormat _renderTargetFormat = WGPUTextureFormat_Undefined;

   // UI renderers
   std::vector<UIRenderer*> _uiRenderers;

   ImGuiManager(Renderer* renderer);
   ~ImGuiManager();

public:
   void Configure(Renderer* renderer);
   void Shutdown();

   void Render(Renderer* renderer, CommandEncoder* encoder, TextureView* surfaceTextureView);

   void AddUIRenderer(UIRenderer* uiRenderer);
   bool RemoveUIRenderer(UIRenderer* uiRenderer);

private:
   void SetupStyle();

   void NewFrame();
   void EndFrame(RenderPassEncoder* encoder);
   void RenderUI();
};

