#pragma once

class CloudRenderer
{
private:
   struct ShaderParamsUniform
   {
      float time;
      vec2 resolution;
      vec3 cameraPos;
   };

private:
   Device* _device;

   RenderPipeline* _pipeline;
   BindGroup* _bindGroup;
   Buffer* _uniformBuffer;
   Buffer* _vertexBuffer;
   Texture* _noiseTexture;
   TextureView* _noiseTextureView;
   Sampler* _sampler;

   ShaderParamsUniform _shaderParams;

public:
   CloudRenderer(Device* device);
   ~CloudRenderer();

   [[nodiscard]] bool Initialize();
   void Terminate();
   void Render(CommandEncoder* encoder, TextureView* surfaceTextureView);
};