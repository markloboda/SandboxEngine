#pragma once

struct GridUniforms
{
   alignas(16) mat4x4 view;
   alignas(16) mat4x4 proj;
   float gridSpacing;
   uint32_t numHorizontal;
   uint32_t numVertical;
};

static_assert(sizeof(GridUniforms) == 144, "GridUniforms size mismatch");

class GridRenderer
{
private:
   GLFWwindow* _window;
   Device* _device;

   Buffer* _vertexBuffer;
   Buffer* _uniformBuffer;

   RenderPipeline* _renderPipeline;

   GridUniforms _uniformsData;

   BindGroup* _uniformsBindGroup;

   GridUniforms _uniforms = {
      .gridSpacing = 1.0f,
      .numHorizontal = 500,
      .numVertical = 500
   };

public:
   GridRenderer(Device* device);
   ~GridRenderer();

private:
   bool Initialize();
   void Terminate();

public:
   void Render(CommandEncoder* encoder, TextureView* surfaceTextureView);
};