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
      float cloudStartHeight = 6100.0;
      float cloudEndHeight = 18300.0;

      float coverageMultiplier = 0.5f;
   };

   CloudRenderSettings Settings;

private:
   RenderPipeline *_pipeline;
   BindGroup *_texturesBindGroup;
   BindGroup *_dataBindGroup;
   TextureView *_weatherMapTextureView;
   Sampler *_weatherMapSampler;
   TextureView *_cloudBaseLowFreqTextureView;
   Sampler *_uCloudBaseLowFreqSampler;
   TextureView *_cloudBaseHighFreqTextureView;
   Sampler *_uCloudBaseHighFreqSampler;
   Buffer *_uCameraData;
   Buffer *_uResolution;
   Buffer *_uCloudRenderSettings;

   CloudsModel *_cloudsModel;
   CameraData _shaderParams;


public:
   explicit CloudRenderer(Renderer &renderer);
   ~CloudRenderer();

   [[nodiscard]] bool Initialize(Renderer &renderer);
   void Terminate() const;
   void Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, int profilerIndex);
};
