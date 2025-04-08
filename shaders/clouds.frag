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
   float densityMultiplier;
   float densityThreshold;
} uSettings;

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragColor;

// define settings
#define NOISE_SCALING_FACTOR 0.00001

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
   float t = 0.0;

   if (rayOrigin.y > uSettings.cloudStartHeight && rayOrigin.y < uSettings.cloudEndHeight)
   {
      // Ray starts inside the cloud layer
      t = 0.0;
   }
   else if (abs(rayDir.y) < 1e-5)
   {
      // Ray is parallel to the cloud layer (no intersection)
      t = -1.0;
   }
   else if (rayOrigin.y < uSettings.cloudStartHeight)
   {
      // Ray starts below the cloud layer
      t = (uSettings.cloudStartHeight - rayOrigin.y) / rayDir.y;
   }
   else if (rayOrigin.y > uSettings.cloudEndHeight)
   {
      // Ray starts above the cloud layer
      t = (uSettings.cloudEndHeight - rayOrigin.y) / rayDir.y;
   }

   return (t >= 0.0) ? t : -1.0;
}

float getCloudInsideDistance(vec3 rayOrigin, vec3 rayDir)
{
   float t = 0.0;

   if (rayOrigin.y > uSettings.cloudStartHeight && rayOrigin.y < uSettings.cloudEndHeight)
   {
      // Ray starts inside the cloud layer
      if (abs(rayDir.y) < 1e-5)
      {
         // Ray is parallel to the cloud layer, default distance
         t = 1000.0;
      }
      else
      {
         if (rayDir.y > 0)
         {
            // Ray is going up
            t = (uSettings.cloudEndHeight - rayOrigin.y) / rayDir.y;
         }
         else
         {
            // Ray is going down
            t = (uSettings.cloudStartHeight - rayOrigin.y) / rayDir.y;
         }
      }
   }
   else
   {
      // Ray starts outside the cloud layer
      if (abs(rayDir.y) < 1e-5)
      {
         // Ray is parallel to the cloud layer
         t = -1.0;
      }
      else
      {
         t = abs(uSettings.cloudEndHeight- uSettings.cloudStartHeight) / rayDir.y;
      }
   }

   return (t > 0.0) ? t : -1.0;
}

float sampleDensity(vec3 pos)
{
   if (pos.y < uSettings.cloudStartHeight || pos.y > uSettings.cloudEndHeight)
      return 0.0;

   vec3 noiseCoord = fract(pos * NOISE_SCALING_FACTOR);
   float noise = texture(sampler3D(cloudTexture, cloudSampler), noiseCoord).r;
   float density = max(0.0, noise - uSettings.densityThreshold) * uSettings.densityMultiplier;
   return density;
}

void main()
{
   vec3 rayOrigin = uCamera.pos;
   vec3 rayDir = getStartRayDirection(uv);
   
   int numSteps = 64;

   float dstToCloud = getCloudEntryDistance(rayOrigin, rayDir);
   float dstInsideCloud = getCloudInsideDistance(rayOrigin, rayDir);
   float dstLimit = dstInsideCloud;
   float stepSize = dstLimit / float(numSteps);

   float dstTravelled = 0.0;
   float totalDensity = 0.0;
   if (dstToCloud < 0.0 || dstLimit < 0.0)
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
         float density = sampleDensity(rayPos);

         totalDensity += density * stepSize;

         if (totalDensity > 1.0)
         {
            totalDensity = 1.0;
            break;
         }

         dstTravelled += stepSize;
      }
   }

   // Output color
   float transmittance = exp(-totalDensity);
   fragColor = vec4(vec3(1.0), 1.0 - transmittance);
}