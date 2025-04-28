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
      float cloudStartHeight = 1000.0;
      float cloudEndHeight = 1400.0;

      float densityMultiplier = 1.0;
      float densityThreshold = 0.291f;

      int cloudNumSteps = 128;
      int lightNumSteps = 6;
      float cloudMaxStepSize = 25.0f;
   };

   CloudRenderSettings Settings;

private:
   RenderPipeline* _pipeline;
   BindGroup* _texturesBindGroup;
   BindGroup* _dataBindGroup;
   TextureView* _weatherMapTextureView;
   Sampler* _weatherMapSampler;
   TextureView* _cloudBaseTextureView;
   Sampler* _uCloudBaseSampler;
   Buffer* _uCameraData;
   Buffer* _uResolution;
   Buffer* _uCloudRenderSettings;

   CameraData _shaderParams;

   CloudsModel* _cloudsModel;

public:
   CloudRenderer(Renderer* renderer);
   ~CloudRenderer();

   [[nodiscard]] bool Initialize(Renderer* renderer);
   void Terminate();
   void Render(Renderer* renderer, CommandEncoder* encoder, TextureView* surfaceTextureView);
};
