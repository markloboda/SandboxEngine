#pragma once

struct AtmosphereUniforms
{
};

class AtmosphereRenderer
{
private:
   BindGroup *_uniformsBindGroup;
   RenderPipeline *_renderPipeline;

   AtmosphereUniforms _uniforms = {};

public:
   AtmosphereRenderer(Renderer *renderer);
   ~AtmosphereRenderer();

private:
   bool Initialize(Renderer *renderer);
   void Terminate();

public:
   void Render(Renderer *renderer, CommandEncoder *encoder, TextureView *surfaceTextureView);
};
