#pragma once

class CloudRenderer;
class GridRenderer;
struct GLFWwindow;

class Renderer
{
public:
   struct RenderStats
   {
      float gridTime = 0.0f;
      float cloudTime = 0.0f;
      float uiTime = 0.0f;
   };

private:
   GLFWwindow* _window;
   Device _device;
   Surface _surface;
   Queue _queue;

   GridRenderer* _gridRenderer;
   CloudRenderer* _cloudRenderer;

   RenderStats _stats;

public:
   Renderer(GLFWwindow* window);
   ~Renderer();

private:
   bool Initialize();
   void Terminate();

public:
   void Render();
   bool ShouldClose() const;

   void OnWindowResize(int width, int height);

   RenderStats GetRenderStats() const { return _stats; }
   CloudRenderer* GetCloudRenderer() const { return _cloudRenderer; }

private:
   void ClearRenderPass(CommandEncoder* encoder, TextureView* surfaceTextureView);
};