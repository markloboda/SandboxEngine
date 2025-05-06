#pragma once

class AtmosphereRenderer;
class CloudRenderer;
class GridRenderer;
struct GLFWwindow;

class Renderer
{
public:
   struct RenderStats
   {
      float atmosphereTime = 0.0f;
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
   AtmosphereRenderer *_atmosphereRenderer;
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

   void UploadTextureData(Texture* texture, const void* data, size_t dataSize, const WGPUExtent3D* writeSize);
   void UploadBufferData(Buffer* buffer, const void* data, size_t size);

   GLFWwindow* GetWindow() const { return _window; }
   Device* GetDevice() { return &_device; }
   Surface* GetSurface() { return &_surface; }
   Queue* GetQueue() { return &_queue; }

   CloudRenderer* GetCloudRenderer() const { return _cloudRenderer; }
   GridRenderer* GetGridRenderer() const { return _gridRenderer; }

   RenderStats GetRenderStats() const { return _stats; }

private:
   void ClearRenderPass(CommandEncoder* encoder, TextureView* surfaceTextureView);
};