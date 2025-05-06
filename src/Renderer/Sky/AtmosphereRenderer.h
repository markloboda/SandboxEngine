#pragma once

class AtmosphereRenderer
{
private:
   RenderPipeline *_renderPipeline;

public:
   AtmosphereRenderer(Renderer *renderer);
   ~AtmosphereRenderer();

private:
   bool Initialize(Renderer *renderer);
   void Terminate();

public:
   void Render(Renderer *renderer, CommandEncoder *encoder, TextureView *surfaceTextureView);
};
