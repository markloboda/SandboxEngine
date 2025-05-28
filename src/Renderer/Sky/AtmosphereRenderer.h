#pragma once

class AtmosphereRenderer
{
public:
   struct AtmosphereRendererWeather
   {
      vec3 sunDirection;
   };

   AtmosphereRendererWeather Weather;

private:
   RenderPipeline *_renderPipeline{};

public:
   explicit AtmosphereRenderer(Renderer &renderer);
   ~AtmosphereRenderer();

private:
   bool Initialize(Renderer &renderer);
   void Terminate();

public:
   void Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, int profilerIndex);
};
