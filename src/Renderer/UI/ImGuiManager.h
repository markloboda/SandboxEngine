#pragma once

class ImGuiManager
{
public:
   static bool CreateInstance(const Renderer &renderer);
   static void DestroyInstance();
   static ImGuiManager& GetInstance();

private:
   static ImGuiManager* _instance;
   WGPUTextureFormat _renderTargetFormat = WGPUTextureFormat_Undefined;

   // UI renderers
   std::vector<UIRenderer*> _uiRenderers;

   explicit ImGuiManager(const Renderer &renderer);
   ~ImGuiManager();

public:
   void Configure(Renderer &renderer);
   void Shutdown();

   void Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, int profilerIndex);

   void AddUIRenderer(UIRenderer* uiRenderer);
   bool RemoveUIRenderer(const UIRenderer *uiRenderer);

private:
   void SetupStyle();

   void NewFrame();
   void EndFrame(const RenderPassEncoder &encoder);
   void RenderUI() const;
};

