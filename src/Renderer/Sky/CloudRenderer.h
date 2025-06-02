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
      float cloudStartHeight = 2000.0f; // height of the bottom of the cloud layer
      float cloudEndHeight = 7000.0f; // height of the top of the cloud layer

      // Densitites
      float coverageMultiplier = 1.0; // scales the coverage read from the weather map
      float densityMultiplier = 1.0; // scales the final computed cloud density
      float highFreqThreshold = 1.0; // threshold for high frequency detail noise application
      float detailBlendStrength = 0.02f; // strength of the detail noise blending

      // Lighting
      float ambientLight = 0.1; // ambient light intensity for the clouds
      float lightAbsorption = 0.375; // how strongly light is absorbed (scattering falloff)
      float henyeyGreensteinStrength = 0.0f; // phase function strength
      float phaseEccentricity = 0.0; // eccentricity for Henyey-Greenstein phase function
      float lightRayConeAngle = 0.0f; // angle of the light ray cone for raymarchToLight() in radians

      // Post Processing
      float toneMappingStrength = 1.0f; // strength of the tone mapping applied to the final cloud color
      float contrastGamma = 0.5f; // contrast gamma for the final cloud colors

      // Performance
      int cloudRaymarchSteps = 500; // number of steps in raymarch()
      int lightRaymarchSteps = 6; // number of steps in raymarchToLight()
      float lightStepLength = 50.0f; // step size in raymarchToLight()
      float coverageCullThreshold = 0.0f; // threshold for culling clouds based on coverage
      int dynamicStep = 1; // whether to use dynamic step size in raymarch()
      float stepSizeFarMultiplier = 2.0; // far step size for raymarching
      float stepSizeNearMultiplier = 1.0; // near step size for raymarching
      float maxEmptySteps = 5; // maximum number of empty steps
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
