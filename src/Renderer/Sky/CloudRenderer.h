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
      float cloudStartHeight = 12000.0f; // height of the bottom of the cloud layer
      float cloudEndHeight = 26800.0f; // height of the top of the cloud layer

      // Densitites
      float coverageMultiplier = 3.31f; // scales the coverage read from the weather map
      float densityMultiplier = 0.511; // scales the final computed cloud density
      float highFreqThreshold = 0.1; // threshold for high frequency detail noise application
      float detailBlendStrength = 1.0f; // strength of the detail noise blending

      // Lighting
      float ambientLight = 0.11; // ambient light intensity for the clouds
      float lightAbsorption = 0.53; // how strongly light is absorbed (scattering falloff)
      float henyeyGreensteinStrength = 0.1f; // strength of the phase function, 0.0 = no phase function and 1.0 = full phase function
      float phaseEccentricity = 0.851f; // eccentricity for Henyey-Greenstein phase function
      float lightRayConeAngle = 1.6f; // angle of the light ray cone for raymarchToLight() in radians

      // Textures
      float lowFreqTextureScale = 1.2f; // scaling factor for the first detail texture (low frequency)
      float highFreqTextureScale = 1.0f; // scaling factor for the second detail texture (high frequency)

      // Post Processing
      float toneMappingStrength = 1.0f; // strength of the tone mapping applied to the final cloud color
      float contrastGamma = 0.4f; // contrast gamma for the final cloud colors

      // Performance
      int cloudRaymarchSteps = 500; // number of steps in raymarch()
      int lightRaymarchSteps = 6; // number of steps in raymarchToLight()
      float lightStepLength = 200.0f; // step size in raymarchToLight()
      float coverageCullThreshold = 0.5f; // threshold for culling clouds based on coverage
      bool dynamicStepSize = false; // whether to use dynamic step size in raymarch()
      float stepSizeFarMultiplier = 1.0; // far step size for raymarching
      float stepSizeNearMultiplier = 0.2; // near step size for raymarching
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
