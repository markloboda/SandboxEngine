#version 450

// inputs
layout(set = 0, binding = 0) uniform texture2D weatherMap;
layout(set = 0, binding = 1) uniform sampler weatherMapSampler;

layout(set = 0, binding = 2) uniform texture3D cloudBaseTexture;
layout(set = 0, binding = 3) uniform sampler cloudBaseSampler;

layout(set = 1, binding = 0) uniform CameraData
{
   mat4 view;
   mat4 proj;
   vec3 pos;
} uCamera;

layout(set = 1, binding = 1) uniform ResolutionData
{
   vec2 xy;
} uResolution;

layout(set = 1, binding = 2) uniform CloudRenderSettings
{
   float cloudStartHeight;
   float cloudEndHeight;

   float densityMultiplier;
   float densityThreshold;
} uSettings;

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragCol;

// constants
#define EPSILON 1e-5

//  settings
#define CLOUD_ABSORBTION 1.0
#define CLOUD_MAP_SCALING_FACTOR 1.0 / 10240.0 // covers 10.24km x 10.24km
#define SUN_DIR vec3(0.577, -0.577, 0.577)
#define CLOUD_DETAIL_TEXTURE_SCALING_FACTOR (1.0 / 50000.0)
#define MAX_RAYMARCH_DISTANCE 10000.0

#define SUN_COLOR vec3(1.0, 1.0, 1.0)
#define AMBIENT_COLOR vec3(0.1, 0.1, 0.1)

// Cloud type functions
#define STRATUS_OFFSET vec2(0.0, 0.2)
#define STRATOCUMULUS_OFFSET vec2(0.0, 0.5)
#define CUMULUS_OFFSET vec2(0.0, 1.0)

// helper functions
float remap(float inMin, float inMax, float outMin, float outMax, float value) {
   float t = clamp((value - inMin) / (inMax - inMin), 0.0, 1.0);
   return mix(outMin, outMax, t);
}

vec3 getStartRayDirection(vec2 uv)
{
   vec4 ndc = vec4(uv * 2.0 - 1.0, -1.0, 1.0);
   vec4 worldPos = inverse(uCamera.proj) * ndc;
   worldPos /= worldPos.w;
   vec3 rayDir = normalize((inverse(uCamera.view) * vec4(worldPos.xyz, 0.0)).xyz);
   return rayDir;
}

// Calculate the distance to the cloud layer to entry point
float getCloudEntryDistance(vec3 rayOrigin, vec3 rayDir)
{
   float res = 0.0;

   if (rayOrigin.y < uSettings.cloudStartHeight)
   {
      // Ray starts below the cloud layer
      if (rayDir.y < EPSILON)
      {
         // no intersection
         res = -1.0;
      }
      else
      {
         res = (uSettings.cloudStartHeight - rayOrigin.y) / rayDir.y;
      }
   }
   else if (rayOrigin.y > uSettings.cloudEndHeight)
   {
      // Ray starts above the cloud layer
      if (rayDir.y > -EPSILON)
      {
         // no intersection
         res = -1.0;
      }
      else
      {
         res = (uSettings.cloudEndHeight - rayOrigin.y) / rayDir.y;
      }
   }
   else
   {
      // Ray starts inside the cloud layer
      res = 0.0;
   }

   return res;
}

// Calculate the distance inside the cloud layer from entry point to the exit point
float getCloudInsideDistance(vec3 rayOrigin, vec3 rayDir)
{
   float res = 0.0;

   if (rayOrigin.y < uSettings.cloudStartHeight)
   {
      // Ray starts below the cloud layer
      if (rayDir.y < EPSILON)
      {
         // No intersection
         res = -1.0;
      }
      else
      {
         // Ray is going up
         res = (uSettings.cloudEndHeight - uSettings.cloudStartHeight) / rayDir.y;
      }
   }
   else if (rayOrigin.y > uSettings.cloudEndHeight)
   {
      // Ray starts above the cloud layer
      if (rayDir.y > -EPSILON)
      {
         // No intersection
         res = -1.0;
      }
      else
      {
         // Ray is going down
         res = (uSettings.cloudStartHeight - uSettings.cloudEndHeight) / rayDir.y;
      }
   }
   else
   {
      // Ray starts inside the cloud layer
      if (abs(rayDir.y) < EPSILON)
      {
         // Ray is parallel to the cloud layer, max distance
         res = MAX_RAYMARCH_DISTANCE;
      }
      else if (rayDir.y > 0)
      {
         // Ray is going up
         res = (uSettings.cloudEndHeight - rayOrigin.y) / rayDir.y;
      }
      else
      {
         // Ray is going down
         res = (uSettings.cloudStartHeight - rayOrigin.y) / rayDir.y;
      }
   }

   return res;
}

// Calculate the cloud height fraction for the given position and cloud type
float getCloudHeightFraction(float height, float cloudType)
{
   // Cloud type float from 0.0 to 1.0
   // 0.0 = stratus
   // 0.5 = stratocumulus
   // 1.0 = cumulus

   if (height < uSettings.cloudStartHeight || height > uSettings.cloudEndHeight)
   {
      return 0.0;
   }

   vec2 cloudTypeHeight = vec2(0.0, 0.0);
   if (cloudType < 0.5)
   {
      // Stratus or stratocumulus
      cloudTypeHeight.x = mix(STRATUS_OFFSET.x, STRATOCUMULUS_OFFSET.x, cloudType * 2.0);
   }
   else
   {
      // Stratocumulus or cumulus
      cloudTypeHeight = mix(STRATOCUMULUS_OFFSET, CUMULUS_OFFSET, (cloudType - 0.5) * 2.0);
   }

   float mappedHeight = (height - uSettings.cloudStartHeight) / (uSettings.cloudEndHeight - uSettings.cloudStartHeight);

   // Smooth step from heigh 0.0 to 0.1
   // and from 0.9 to 1.0
   float fadeIn = smoothstep(cloudTypeHeight.x, cloudTypeHeight.x + 0.1, mappedHeight);
   float fadeOut = 1.0 - smoothstep(cloudTypeHeight.y - 0.1, cloudTypeHeight.y, mappedHeight);
   float heightFraction = fadeIn * fadeOut;

   return heightFraction;
}

// Sample weather map (2D texture)
vec4 getCloudMapSample(vec3 pos)
{
   if (pos.y < uSettings.cloudStartHeight || pos.y > uSettings.cloudEndHeight)
   {
      // Outside of the cloud layer
      return vec4(0.0);
   }

   vec2 cloudMap = pos.xz * CLOUD_MAP_SCALING_FACTOR;
   if (cloudMap.x < 0.0 || cloudMap.x > 1.0 || cloudMap.y < 0.0 || cloudMap.y > 1.0)
   {
      // Out of weather map bounds
      return vec4(0.0);
   }

   vec4 weatherMapSample = texture(sampler2D(weatherMap, weatherMapSampler), cloudMap);
   return weatherMapSample;
}

// Sample cloud detail noise (3D texture)
vec4 getCloudDetailSample(vec3 pos)
{
   vec3 detailTextureCoord = pos * CLOUD_DETAIL_TEXTURE_SCALING_FACTOR;
   vec4 detailSample = texture(sampler3D(cloudBaseTexture, cloudBaseSampler), detailTextureCoord);
   return detailSample;
}

//float raymarchToLight(vec3 rayOrigin, vec3 rayDir)
//{
//   int lightNumSteps = 6;
//   float dstInsideCloud = min(100.0, getCloudInsideDistance(rayOrigin, rayDir));
//   float stepSize = dstInsideCloud / lightNumSteps;
//
//   float totalDensity = 0.0;
//   for (int i = 0; i < lightNumSteps; i++)
//   {
//      vec3 rayPos = rayOrigin + rayDir * (i * stepSize);
//      float density = getCloudDensity(rayPos, 0);
//      totalDensity += density * stepSize;
//   }
//
//   float transmittance = exp(-totalDensity * CLOUD_ABSORBTION);
//   return transmittance;
//}

vec4 raymarch(vec3 start, vec3 end)
{
   // Ray
   vec3  ray = end - start;
   vec3  rayDir = normalize(ray);
   float rayLength = length(ray);

   // Raymarching parameters
   float transmittance = 1.0;

   // Start raymarching
   vec3  rayPos = start;

   int         lodCurrent       = 1;
   const float lodStepSize[2]   = float[2](20.0, 60.0);
   const int   lodSwitchMiss    = int(lodStepSize[1] / lodStepSize[0]);
   int         lodSwitchCounter = 0;
   float       lodDensityLimit  = uSettings.densityThreshold + 0.12;

   int   maxSteps = 120;
   int   stepCount = 0;
   float remainingDst = rayLength;
   while (remainingDst > 0.0)
   {
      if (stepCount++ >= maxSteps)
      {
         break;
      }

      float stepSize = lodStepSize[lodCurrent];

      if (lodCurrent == 0)
      {
         vec4 weatherMapSample = getCloudMapSample(rayPos);
         float coverage1 = weatherMapSample.r;
         float cloudType = weatherMapSample.b;

         float weatherMapDensity = coverage1 * getCloudHeightFraction(rayPos.y, cloudType);
         if (weatherMapDensity < lodDensityLimit)
         {
            if (++lodSwitchCounter >= lodSwitchMiss)
            {
               // Switch to lod=1 (density too low)
               lodSwitchCounter = 0;
               lodCurrent = 1;
               continue;
            }
         }

         lodSwitchCounter = 0;

         // Process clouds (DETAILED)
         vec4 detailSample = getCloudDetailSample(rayPos);
         float perlinWorley = detailSample.r;

         float cloudDensity = max(0.0, weatherMapDensity * perlinWorley - uSettings.densityThreshold) * uSettings.densityMultiplier;
         transmittance *= exp(-cloudDensity * stepSize * CLOUD_ABSORBTION);

         // Early exit if transmittance is low
         if (transmittance < 0.01)
         {
            transmittance = 0.0;
            break;
         }
      }
      else
      {
         vec4 weatherMapSample = getCloudMapSample(rayPos);
         float coverage1 = weatherMapSample.r;
         float cloudType = weatherMapSample.b;

         float weatherMapDensity = coverage1 * getCloudHeightFraction(rayPos.y, cloudType);
         if (weatherMapDensity > lodDensityLimit)
         {
            // Take a step back and switch to lod=0
            rayPos -= rayDir * stepSize;
            remainingDst += stepSize;
            lodCurrent = 0;
            continue;
         }
      }

      // Move forward
      rayPos += rayDir * stepSize;
      remainingDst -= stepSize;
   }

   float alpha = 1.0 - transmittance;
   return vec4(vec3(1.0), alpha);
}

void main()
{
   vec3 rayOrigin = uCamera.pos;
   vec3 rayDir = getStartRayDirection(uv);

   float dstToStart = getCloudEntryDistance(rayOrigin, rayDir);
   float dstInside  = getCloudInsideDistance(rayOrigin, rayDir);

   if (dstToStart < -EPSILON || dstInside < 0.0)
   {
      // no clouds
      fragCol = vec4(0.0);
      return;
   }

   dstInside = min(dstInside, MAX_RAYMARCH_DISTANCE);

   // Raymarch
   vec3 start = rayOrigin + rayDir * dstToStart;
   vec3 end   = start     + rayDir * dstInside;

   fragCol = raymarch(start, end);
}