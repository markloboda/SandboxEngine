#pragma once

class CloudsModel;

class CloudRenderer
{
private:
   struct CameraData
   {
      mat4x4 view;
      mat4x4 proj;
      vec3 pos;
   };

   struct ResolutionData
   {
      vec2 xy;
   };

public:
   struct CloudRenderSettings
   {
      float cloudStartHeight = 200;
      float cloudEndHeight = 500;
      float cloudScale = 1.0;
      float densityMultiplier = 1.0;
   };

   CloudRenderSettings Settings;

private:
   Device* _device;
   Queue* _queue;

   RenderPipeline* _pipeline;
   BindGroup* _bindGroup;
   Texture* _cloudTexture;
   TextureView* _cloudTextureView;
   Buffer* _uCameraData;
   Buffer* _uResolution;
   Buffer* _uCloudRenderSettings;
   Sampler* _uCloudSampler;

   CameraData _shaderParams;

   CloudsModel* _cloudsModel;

public:
   CloudRenderer(Device* device, Queue* queue);
   ~CloudRenderer();

   [[nodiscard]] bool Initialize();
   void Terminate();
   void Render(CommandEncoder* encoder, TextureView* surfaceTextureView);
};
