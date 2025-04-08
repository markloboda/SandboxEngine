#version 450

// inputs
layout(set = 0, binding = 0) uniform texture3D cloudTexture;

layout(set = 0, binding = 1) uniform sampler cloudSampler;

layout(set = 0, binding = 2) uniform CameraData 
{
   mat4 view;
   mat4 proj;
   vec3 pos;
} uCamera;

layout(set = 0, binding = 3) uniform ResolutionData
{
   vec2 xy;
} uResolution;

layout(set = 0, binding = 4) uniform CloudRenderSettings
{
   float cloudStartHeight; 
   float cloudEndHeight; 
   float cloudScale; 
   float densityMultiplier; 
} uSettings;

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragColor;


// define settings
#define DENSITY_MULTIPLIER 0.005
#define NOISE_SCALING_FACTOR 0.00005

// helper functions
vec3 getStartRayDirection(vec2 uv)
{
   vec4 ndc = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
   vec4 worldPos = inverse(uCamera.proj) * ndc;
   worldPos /= worldPos.w;
   vec3 rayDir = normalize((inverse(uCamera.view) * vec4(worldPos.xyz, 0.0)).xyz);
   return rayDir;
}

float getCloudEntryDistance(vec3 rayOrigin, vec3 rayDir)
{
   // Ray starts inside the cloud layer
   if (rayOrigin.y > uSettings.cloudStartHeight && rayOrigin.y < uSettings.cloudEndHeight)
      return 0.0;

   // Ray is parallel to the cloud layer (no intersection)
   if (abs(rayDir.y) < 1e-5)
      return -1.0;

   float t = 0.0;
   if (rayOrigin.y < uSettings.cloudStartHeight)
   {
      // Ray starts below the cloud layer
      t = (uSettings.cloudStartHeight - rayOrigin.y) / rayDir.y;
   }
   else if (rayOrigin.y > uSettings.cloudEndHeight)
   {
      // Ray starts above the cloud layer
      t = (uSettings.cloudEndHeight - rayOrigin.y) / rayDir.y;
   }

   // We only care about intersections in front of the camera
   return (t > 0.0) ? t : -1.0;
}

float sampleDensity(vec3 pos)
{
   if (pos.y < uSettings.cloudStartHeight || pos.y > uSettings.cloudEndHeight)
      return 0.0;

   vec3 noiseCoord = fract(pos * NOISE_SCALING_FACTOR * uSettings.cloudScale);
   float noise = texture(sampler3D(cloudTexture, cloudSampler), noiseCoord).r;
   float density = clamp(noise * DENSITY_MULTIPLIER * uSettings.densityMultiplier, 0.0, 1.0);
   return density;
}

void main()
{
   vec3 rayOrigin = uCamera.pos;
   vec3 rayDir = getStartRayDirection(uv);
   
   float dstToCloud = getCloudEntryDistance(rayOrigin, rayDir);

   float dstTravelled = 0.0;
   float stepSize = 1.2;
   float dstLimit = 300;

   float totalDensity = 0.0;
   if (dstToCloud < 0.0)
   {
      // We will never reach a cloud with this ray.
      totalDensity = 0.0;
   }
   else 
   {
      // Ray march cloud.
      while (dstTravelled < dstLimit)
      {
         vec3 rayPos = rayOrigin + rayDir * (dstToCloud + dstTravelled);
         totalDensity += sampleDensity(rayPos) * stepSize;
         dstTravelled += stepSize;
      }
   }

   float transmittance = exp(-totalDensity);
   fragColor = vec4(vec3(1), totalDensity);
}