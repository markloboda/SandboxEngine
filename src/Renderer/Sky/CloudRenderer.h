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
      float cloudStartHeight = 1500.0f; // height of the bottom of the cloud layer
      float cloudEndHeight = 16300.0f; // height of the top of the cloud layer

      float lightAbsorption = 0.626f; // how strongly light is absorbed (scattering falloff)
      float coverageMultiplier = 6.963f; // scales the coverage read from the weather map
      float phaseEccentricity = -0.125f; // eccentricity for Henyey-Greenstein phase function
      float densityMultiplier = 0.033f; // scales the final computed cloud density
      float detailThreshold = 0.0f; // threshold for detail noise application

      int cloudRaymarchSteps = 300; // number of steps in raymarch()
      int lightRaymarchSteps = 7; // number of steps in raymarchToLight()
      float lightStepLength = 100.0f; // step size in raymarchToLight()
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
