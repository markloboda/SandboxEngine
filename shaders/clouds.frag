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
#define M_PI 3.14159265358979323846

//  settings
#define CLOUD_ABSORBTION 1.0
#define CLOUD_MAP_SCALING_FACTOR 1.0 / 512000.0 // covers 512km x 512km
#define SUN_DIR vec3(0.577, -0.577, 0.577)
#define CLOUD_DETAIL_TEXTURE_SCALING_FACTOR (1.0 / 80000.0)
#define MAX_RAYMARCH_DISTANCE 500000.0

#define SUN_COLOR vec3(1.0, 1.0, 1.0)
#define AMBIENT_COLOR vec3(0.1, 0.1, 0.1)

// Cloud type functions
#define STRATUS_OFFSET       vec3(0.1, 0.2, 0.3)
#define STRATOCUMULUS_OFFSET vec3(0.1, 0.5, 0.6)
#define CUMULUS_OFFSET       vec3(0.1, 0.9, 1.0)

// helper functions
float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
   return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

float clampRemap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
   float remappedValue = remap(originalValue, originalMin, originalMax, newMin, newMax);
   return clamp(remappedValue, newMin, newMax);
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

float heightGradient(float mappedHeight, float min, float center, float max)
{
   return clamp(remap(mappedHeight, 0.0, min, 0.0, 1.0), 0.0, 1.0) * clamp(remap(mappedHeight, center, max, 1.0, 0.0), 0.0, 1.0);
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

   vec3 cloudTypeHeight = vec3(0.0);
   if (cloudType < 0.5)
   {
      // stratus or stratocumulus
      cloudTypeHeight = mix(STRATUS_OFFSET, STRATOCUMULUS_OFFSET, cloudType * 2.0);
   }
   else
   {
      // stratocumulus or cumulus
      cloudTypeHeight = mix(STRATOCUMULUS_OFFSET, CUMULUS_OFFSET, (cloudType - 0.5) * 2.0);
   }

   float mappedHeight = remap(height, uSettings.cloudStartHeight, uSettings.cloudEndHeight, 0.0, 1.0); // is already between cloudStartHeight and cloudEndHeight
   float heightFraction = heightGradient(mappedHeight, cloudTypeHeight.x, cloudTypeHeight.y, cloudTypeHeight.z);
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

// HenyeyGreenstein phase function
float henyeyGreenstein(float cos, float eccentricity)
{
   return ((1.0 - eccentricity * eccentricity) / pow(1.0 + eccentricity * eccentricity - 2.0 * eccentricity * cos, 1.5)) / (4.0 * M_PI);
}

float raymarchToLight(vec3 rayOrigin, vec3 rayDir)
{
   return 0.0;
}

vec4 raymarch(vec3 start, vec3 end)
{
   // Ray setup
   vec3  ray       = end - start;
   float rayLength = length(ray);
   vec3  rayDir    = ray / rayLength;

   // Volumetric state
   float transmittance    = 1.0;

   // Log‑biased steps parameters
   const int   numSteps   = 120;
   const float biasPower  = 2.0;    // >1 biases samples toward the camera

   for (int i = 0; i < numSteps; ++i)
   {
      // update raymarching position
      float stepIndex = float(i + 1) / float(numSteps);
      float stepDist  = pow(stepIndex, biasPower) * rayLength;
      vec3 rayPos = start + rayDir * stepDist;

      // cheap map sampling
      vec4  mapSample      = getCloudMapSample(rayPos);
      float cloudCoverage  = mapSample.r;
      float cloudType      = mapSample.b;

      float heightFraction  = getCloudHeightFraction(rayPos.y, cloudType);
      float coverageDensity = cloudCoverage * heightFraction;

      if (coverageDensity < uSettings.densityThreshold * 0.5)
         continue;

      // expensive detail sampling
      vec4  detailSample    = getCloudDetailSample(rayPos);
      float perlinWorley        = detailSample.r;
      float worley1             = detailSample.g;
      float worley2             = detailSample.b;
      float worley3             = detailSample.a;

      // base cloud density
      float lowFreqNoise = perlinWorley;
      float highFreqNoise = dot(vec3(worley1, worley2, worley3), vec3(0.625, 0.25, 0.125));
      float baseDensity = clampRemap(lowFreqNoise, highFreqNoise, 1.0, 0.0, 1.0);

      float cloudDensity = max(0.0, coverageDensity * baseDensity - uSettings.densityThreshold) * uSettings.densityMultiplier;



//      float cloudDensity = max(0.0, cloudCoverage * heightFraction * perlinWorley * worley1 - uSettings.densityThreshold) * uSettings.densityMultiplier;

//      const float eccentricity = 0.6;
//      const float silverIntensity = 0.5;
//      const float silverSpread = 0.5;
//      float cos = dot(normalize(SUN_DIR), rayDir);
//      float hg = max(henyeyGreenstein(cos, eccentricity), silverIntensity * henyeyGreenstein(cos, 0.99 - silverSpread));

      // Accumulate raymarching parameters
      transmittance *= exp(-cloudDensity * stepDist * CLOUD_ABSORBTION);

      if (transmittance < 0.001)
         break;
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