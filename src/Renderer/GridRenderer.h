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
   BindGroup *_uniformsBindGroup;
   Buffer *_uniformBuffer;
   Buffer *_vertexBuffer;

   RenderPipeline *_renderPipeline;

   GridUniforms _uniforms = {
      .view = mat4x4(),
      .proj = mat4x4(),
      .gridSpacing = 1.0f,
      .numHorizontal = 500,
      .numVertical = 500
   };

public:
   explicit GridRenderer(Renderer &renderer);
   ~GridRenderer();

private:
   bool Initialize(Renderer &renderer);
   void Terminate();

public:
   void Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, int profilerIndex);
};
