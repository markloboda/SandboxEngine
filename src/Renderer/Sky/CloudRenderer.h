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

   struct CloudData
   {
      vec2 position;
   };

public:
   struct CloudRenderSettings
   {
      // Position
      float cloudStartHeight = 11000.0f; // height of the bottom of the cloud layer
      float cloudEndHeight = 16000.0f; // height of the top of the cloud layer

      // Densitites
      float coverageMultiplier = 1.278; // scales the coverage read from the weather map
      float densityMultiplier = 0.3; // scales the final computed cloud density
      float detailBlendStrength = 0.02f; // strength of the detail noise blending

      // Lighting
      float ambientLight = 0.09; // ambient light intensity for the clouds
      float lightAbsorption = 0.218; // how strongly light is absorbed (scattering falloff)
      float henyeyGreensteinStrength = 0.7f; // phase function strength
      float phaseEccentricity = 0.262; // eccentricity for Henyey-Greenstein phase function
      float lightRayConeAngle = 0.3f; // angle of the light ray cone for raymarchToLight() in radians

      // Post Processing
      float toneMappingStrength = 0.6f; // strength of the tone mapping applied to the final cloud color
      float contrastGamma = 0.5f; // contrast gamma for the final cloud colors

      // Performance
      int cloudRaymarchStepsVer = 60; // number of steps in raymarch() vertical
      int cloudRaymarchStepsHor = 200; // number of steps in raymarch() horizontal
      int lightRaymarchSteps = 6; // number of steps in raymarchToLight()
      float lightStepLength = 100.0f; // step size in raymarchToLight()
      float coverageCullThreshold = 0.35f; // threshold for culling clouds based on coverage
      float erodeCullThreshold = 0.08f; // threshold for culling high frequency texture sampling
      int dynamicStep = 1; // whether to use dynamic step size in raymarch()
      float maxEmptySteps = 6; // maximum number of empty steps
      float stepSizeFarMultiplierVer = 1.29; // far step size for raymarching
      float stepSizeNearMultiplierVer = 0.68; // near step size for raymarching
      float stepSizeFarMultiplierHor = 1.0; // far step size for raymarching
      float stepSizeNearMultiplierHor = 0.24; // near step size for raymarching
   };

   struct CloudRenderWeather
   {
      vec4 sunDirection = vec4(0.0f, 1.0f, 0.0f, 1.0f);
      vec4 detailNoiseOffset = vec4(0.0f);
   };

   CloudRenderSettings Settings;
   CloudRenderWeather Weather;

private:
   RenderPipeline *_pipeline;
   BindGroup *_texturesBindGroup;
   BindGroup *_dataBindGroup;
   Texture *_weatherMapTexture;
   TextureView *_weatherMapTextureView;
   Sampler *_weatherMapSampler;
   TextureView *_cloudBaseLowFreqTextureView;
   Sampler *_uCloudBaseLowFreqSampler;
   TextureView *_cloudBaseHighFreqTextureView;
   Sampler *_uCloudBaseHighFreqSampler;
   Buffer *_uCameraData;
   Buffer *_uResolution;
   Buffer *_uCloudRenderSettings;
   Buffer *_uCloudRenderWeather;
   Buffer *_uCloudData;

   CameraData _shaderParams;

   size_t _weatherMapHash = 0;

public:
   explicit CloudRenderer(Renderer &renderer, CloudsModel &cloudsModel);
   ~CloudRenderer();

   [[nodiscard]] bool Initialize(Renderer &renderer, CloudsModel &cloudsModel);
   void Terminate() const;
   void Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, const CloudsModel &cloudsModel, int profilerIndex);
};
