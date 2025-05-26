#version 450

// inputs
layout(set = 0, binding = 0) uniform texture2D weatherMap;
layout(set = 0, binding = 1) uniform sampler weatherMapSampler;

layout(set = 0, binding = 2) uniform texture3D cloudBaseLowFreqTexture;
layout(set = 0, binding = 3) uniform sampler cloudBaseLowFreqSampler;

layout(set = 0, binding = 4) uniform texture3D cloudBaseHighFreqTexture;
layout(set = 0, binding = 5) uniform sampler cloudBaseHighFreqSampler;

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

   float coverageMultiplier;
} uSettings;

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragCol;

// math constants
#define EPSILON 1e-5
#define M_PI 3.14159265358979323846

// physical settings
#define SUN_COLOR vec3(1.0)
#define SUN_DIR vec3(0.577, -0.577, 0.577)
#define LIGHT_ABSORPTION 1.0

// raymarching settings
#define MAX_RAYMARCH_STEPS 120
#define MAX_RAYMARCH_LIGHT_STEPS 6
#define MAX_RAYMARCH_DISTANCE 102400.0
#define CLOUD_MAP_SCALING_FACTOR 1.0 / 51200.0 // covers 51.2km x 51.2km
#define CLOUD_DETAIL_TEXTURE_SCALING_FACTOR (1.0 / 10000)

// cloud heights
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
   return clampRemap(mappedHeight, 0.0, min, 0.0, 1.0) * clampRemap(mappedHeight, center, max, 1.0, 0.0);
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

   float mappedHeight = clampRemap(height, uSettings.cloudStartHeight, uSettings.cloudEndHeight, 0.0, 1.0); // is already between cloudStartHeight and cloudEndHeight
   float heightFraction = heightGradient(mappedHeight, cloudTypeHeight.x, cloudTypeHeight.y, cloudTypeHeight.z);
   return heightFraction;
}

// Sample weather map (2D texture)
vec4 sampleCloudMap(in vec3 pos)
{
   if (pos.y < uSettings.cloudStartHeight || pos.y > uSettings.cloudEndHeight)
   {
      // Outside of the cloud layer
      return vec4(0.0);
   }

   vec2 cloudMapUV = pos.xz * CLOUD_MAP_SCALING_FACTOR;
   if (cloudMapUV.x < 0.0 || cloudMapUV.x > 1.0 || cloudMapUV.y < 0.0 || cloudMapUV.y > 1.0)
   {
      // Out of weather map bounds
      return vec4(0.0);
   }

   vec4 weatherMapSample = texture(sampler2D(weatherMap, weatherMapSampler), cloudMapUV);
   return weatherMapSample;
}

// Sample cloud detail low frequency noise (3D texture)
vec4 sampleCloudDetailLowFreq(in vec3 pos)
{
   vec3 detailTextureCoord = pos * CLOUD_DETAIL_TEXTURE_SCALING_FACTOR;
   vec4 detailSample = texture(sampler3D(cloudBaseLowFreqTexture, cloudBaseLowFreqSampler), detailTextureCoord);
   return detailSample;
}

// Sample cloud detail high frequency noise (3D texture)
vec3 sampleCloudDetailHighFreq(in vec3 pos)
{
   vec3 detailTextureCoord = pos * CLOUD_DETAIL_TEXTURE_SCALING_FACTOR;
   vec3 detailSample = texture(sampler3D(cloudBaseHighFreqTexture, cloudBaseHighFreqSampler), detailTextureCoord).rgb;
   return detailSample;
}

// Get cloud density at world position
float density(in vec3 pos)
{
   // map sampling
   vec4  mapSample      = sampleCloudMap(pos);

   if (mapSample.a < EPSILON)
   {
      // no clouds at this position
      return 0.0;
   }

   float cloudCoverage  = clamp(mapSample.r * uSettings.coverageMultiplier, 0.0, 1.0);
   float cloudType      = mapSample.b;
   float heightFraction = getCloudHeightFraction(pos.y, cloudType);
   float heightGradient = clampRemap(pos.y, uSettings.cloudStartHeight, uSettings.cloudEndHeight, 0.0, 1.0);

   // detail sampling
   vec4  detailLowFreqSample  = sampleCloudDetailLowFreq(pos);
   float lowFreqPerlin  = detailLowFreqSample.r;
   float lowFreqFBM = clamp(dot(detailLowFreqSample.gba, vec3(0.625, 0.25, 0.125)), 0.0, 1.0);
   vec3 detailHighFreqSample = sampleCloudDetailHighFreq(pos).rgb;
   float highFreqFBM = clamp(dot(detailHighFreqSample, vec3(0.625, 0.25, 0.125)), 0.0, 1.0);

   // final density calculation
   float baseDensity = remap(lowFreqPerlin, (1.0 - lowFreqFBM), 1.0, 0.0, 1.0) * heightFraction; // combine low frequency noise and height fraction
   float cloudDensity = remap(baseDensity, (1.0 - cloudCoverage), 1.0, 0.0, 1.0);
   cloudDensity *= heightGradient; // apply height gradient

   // TODO: add erosion at edges

   return clamp(cloudDensity, 0.0, 1.0);
}

// HenyeyGreenstein phase function
float henyeyGreenstein(float cos, float eccentricity)
{
   return ((1.0 - eccentricity * eccentricity) / pow(1.0 + eccentricity * eccentricity - 2.0 * eccentricity * cos, 1.5)) / (4.0 * M_PI);
}

float raymarchToLight(vec3 rayOrigin, vec3 rayDir)
{
   const int numSteps = MAX_RAYMARCH_LIGHT_STEPS;
   const float stepSize = 200.0 / float(numSteps);

   float lightTransmittance = 1.0;

   vec3 step = stepSize * rayDir;
   vec3 rayPos = rayOrigin + step;

   for (int i = 0; i < numSteps; ++i)
   {
      rayPos += step;

      float cloudDensity = density(rayPos);
      if (cloudDensity > EPSILON)
         continue;

      lightTransmittance *= exp(-cloudDensity * stepSize * LIGHT_ABSORPTION);
   }

   return lightTransmittance;
}

vec4 raymarch(vec3 start, vec3 end)
{
   // Ray setup
   vec3  ray       = end - start;
   float rayLength = length(ray);
   vec3  rayDir    = ray / rayLength;

   // Volumetric state
   float transmittance = 1.0;
   float lightEnergy   = 0.0;

   // Log‑biased steps parameters
   const int   numSteps   = MAX_RAYMARCH_STEPS;
   const float biasPower  = 2.0;    // >1 biases samples toward the camera

   float prevRayDst = 0.0;
   for (int i = 0; i < numSteps; ++i)
   {
      // update raymarching position
      float stepIndex = float(i + 1) / float(numSteps);
      float rayDst  = pow(stepIndex, biasPower) * rayLength;
      float stepSize = rayDst - prevRayDst;
      vec3 rayPos = start + rayDir * rayDst;

      prevRayDst = rayDst;

      float cloudDensity = density(rayPos);
      if (cloudDensity < EPSILON)
         continue;

      float lightTransmittance = raymarchToLight(rayPos, -SUN_DIR);
      float phaseFunction = henyeyGreenstein(dot(rayDir, SUN_DIR), 0.5);

      transmittance *= exp(-cloudDensity * stepSize);
      lightEnergy += transmittance * cloudDensity * phaseFunction * lightTransmittance * stepSize;

      if (transmittance <= 0.01)
      {
         transmittance = 0.0;
         break;
      }
   }

   float alpha = 1.0 - transmittance;
   vec3 lightColor = SUN_COLOR;
   vec3 color = lightColor * (1.0 - lightEnergy);

   return vec4(color, alpha);
}

void main()
{
   vec3 rayOrigin = uCamera.pos;
   vec3 rayDir = getStartRayDirection(uv);

   float dstToStart = getCloudEntryDistance(rayOrigin, rayDir);
   float dstInside  = getCloudInsideDistance(rayOrigin, rayDir);

   if (dstToStart < -EPSILON || dstInside < 0.0)
   {
      fragCol = vec4(0.0);
      return;
   }

   dstInside = min(dstInside, MAX_RAYMARCH_DISTANCE);

   vec3 start = rayOrigin + rayDir * dstToStart;
   vec3 end   = start     + rayDir * dstInside;

   fragCol = raymarch(start, end);
}