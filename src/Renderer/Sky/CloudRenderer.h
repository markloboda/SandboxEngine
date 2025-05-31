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
      float cloudStartHeight = 1500.0f; // height of the bottom of the cloud layer
      float cloudEndHeight = 16300.0f; // height of the top of the cloud layer
      float lightAbsorption = 0.881f; // how strongly light is absorbed (scattering falloff)
      float coverageMultiplier = 2.513f; // scales the coverage read from the weather map
      float phaseEccentricity = 0.144f; // eccentricity for Henyey-Greenstein phase function
      float densityMultiplier = 0.059f; // scales the final computed cloud density
      float detailThreshold = 0.2f; // threshold for detail noise application
      float lightRayConeAngle = 1.0f; // angle of the light ray cone for raymarchToLight() in radians
      float ambientLight = 0.263; // ambient light intensity for the clouds
      int highFreqTextureScale = 1;
      int cloudRaymarchSteps = 300; // number of steps in raymarch()
      int lightRaymarchSteps = 6; // number of steps in raymarchToLight()
      float lightStepLength = 120.0f; // step size in raymarchToLight()
      float cloudDetailTextureScalingFactor1 = 1.2f; // scaling factor for the first detail texture (low frequency)
      float toneMappingStrength = 1.0f; // strength of the tone mapping applied to the final cloud color
      float henyeyGreensteinStrength = 0.0f;
      float multipleScatteringStrength = 0.0f; // strength of the multiple scattering effect
      float contrastGamma = 0.5f; // contrast gamma for the final cloud colors
      float detailBlendStrength = 0.42f; // strength of the detail noise blending
   };

   struct CloudRenderWeather
   {
      vec3 sunDirection = vec3(0.0f, 1.0f, 0.0f);
      vec3 detailNoiseOffset = vec3(0.0f);
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
