#pragma once
#include "Utils/Profiler.h"

struct GLFWwindow;
class Profiler;

class AtmosphereRenderer;
class CloudRenderer;
class GridRenderer;
class ClothRenderer;

class Renderer
{
public:
   struct GPURenderStats
   {
      float totalTime = 0.0f;
      float clearTime = 0.0f;
      float atmosphereTime = 0.0f;
      float gridTime = 0.0f;
      float clothTime = 0.0f;
      float cloudTime = 0.0f;
      float uiTime = 0.0f;
   };

private:
   GLFWwindow *_window;
   Device _device;
   Surface _surface;
   Queue _queue;

   GridRenderer *_gridRenderer;
   AtmosphereRenderer *_atmosphereRenderer;
   CloudRenderer *_cloudRenderer;
   ClothRenderer *_clothRenderer;

   Profiler *_profiler;
   GPURenderStats _gpuRenderStats;

public:
   explicit Renderer(GLFWwindow *window);
   ~Renderer();

private:
   bool Initialize();
   void InitializeTimestampResources();
   void Terminate() const;

public:
   void Update(float dt) const;

   void Render();
   [[nodiscard]] bool ShouldClose() const;

   void OnWindowResize(int width, int height);

   void UploadTextureData(const Texture &texture, const void *data, size_t dataSize, const WGPUExtent3D *writeSize) const;
   void UploadBufferData(const Buffer &buffer, const void *data, size_t size) const;

   [[nodiscard]] GLFWwindow *GetWindow() const { return _window; }
   Device &GetDevice() { return _device; }
   Surface &GetSurface() { return _surface; }
   Queue &GetQueue() { return _queue; }

   [[nodiscard]] CloudRenderer &GetCloudRenderer() const { return *_cloudRenderer; }
   [[nodiscard]] GridRenderer &GetGridRenderer() const { return *_gridRenderer; }

   [[nodiscard]] Profiler &GetProfiler() const { return *_profiler; }
   [[nodiscard]] const GPURenderStats &GetGPURenderStats() const { return _gpuRenderStats; }

private:
   void ClearRenderPass(const CommandEncoder &encoder, const TextureView &surfaceTextureView, int profilerIndex) const;

   void FetchProfileResults();
};