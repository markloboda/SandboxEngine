#pragma once

class CloudRenderer
{
private:
   struct CameraData
   {
      mat4x4 view;
      mat4x4 proj;
      vec3 cameraPos;
   };

private:
   Device* _device;
   Queue* _queue;

   RenderPipeline* _pipeline;
   BindGroup* _bindGroup;
   Texture* _cloudTexture;
   TextureView* _cloudTextureView;
   Buffer* _uCameraData;
   Sampler* _uCloudSampler;

   CameraData _shaderParams;

public:
   CloudRenderer(Device* device, Queue* queue);
   ~CloudRenderer();

   [[nodiscard]] bool Initialize();
   void Terminate();
   void Render(CommandEncoder* encoder, TextureView* surfaceTextureView);
};