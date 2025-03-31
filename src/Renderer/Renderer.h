#pragma once

class CloudRenderer;
class GridRenderer;
struct GLFWwindow;

class Renderer
{
private:
   GLFWwindow* _window;
   Device _device;
   Surface _surface;
   Queue _queue;

   GridRenderer* _gridRenderer;
   CloudRenderer* _cloudRenderer;

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

private:
   void ClearRenderPass(CommandEncoder* encoder, TextureView* surfaceTextureView);
};