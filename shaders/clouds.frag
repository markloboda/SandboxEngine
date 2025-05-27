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
   float cloudStartHeight;       // height of the bottom of the cloud layer
   float cloudEndHeight;         // height of the top of the cloud layer

   float lightAbsorption;        // how strongly light is absorbed (scattering falloff)
   float coverageMultiplier;     // scales the coverage read from the weather map
   float phaseEccentricity;      // eccentricity for Henyey-Greenstein phase function
   float detailStrength;         // weight of detail FBM in final density
   float densityMultiplier;      // scales the final computed cloud density

   int cloudRaymarchSteps;       // number of steps in raymarch()
   int lightRaymarchSteps;       // number of steps in raymarchToLight()
   float lightStepLength;        // step size in raymarchToLight()
} uSettings;

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragCol;

// math constants
#define EPSILON 1e-5
#define M_PI 3.14159265358979323846

// physical settings
#define SUN_COLOR vec3(0.92, 0.8, 0.71) // yellow sunlight color
#define SUN_DIR _sunDir
const vec3 _sunDir = normalize(vec3(0.0, -1.0, 0.0));

// raymarching settings
#define MAX_RAYMARCH_STEPS uSettings.cloudRaymarchSteps
#define MAX_RAYMARCH_LIGHT_STEPS uSettings.lightRaymarchSteps
#define MAX_RAYMARCH_DISTANCE 51200.0
#define CLOUD_MAP_SCALING_FACTOR (1.0 / 51200.0) // covers 51.2km x 51.2km
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
float sampleCloudDensity(in vec3 pos)
{
   vec4 mapSample = sampleCloudMap(pos);
   if (mapSample.a < EPSILON)
   {
      // no clouds at this position (outside cloud layer or tile bounds)
      return 0.0;
   }

   // map data
   float cloudCoverage  = clamp(mapSample.r * uSettings.coverageMultiplier, 0.0, 1.0);
   float cloudType      = mapSample.b;
   float heightFraction = getCloudHeightFraction(pos.y, cloudType);
   float heightGradient = clampRemap(pos.y, uSettings.cloudStartHeight, uSettings.cloudEndHeight, 0.0, 1.0);

   // detail low freq data
   vec4  detailLowFreqSample  = sampleCloudDetailLowFreq(pos);
   float lowFreqPerlin  = detailLowFreqSample.r;
   float lowFreqFBM = clamp(dot(detailLowFreqSample.gba, vec3(0.625, 0.25, 0.125)), 0.0, 1.0);

   // detail high freq data
   vec3  detailHighFreqSample = sampleCloudDetailHighFreq(pos).rgb;
   float highFreqFBM = clamp(dot(detailHighFreqSample, vec3(0.625, 0.25, 0.125)), 0.0, 1.0);

   // final density calculation
   float baseDensity = remap(lowFreqPerlin, (1.0 - lowFreqFBM), 1.0, 0.0, 1.0);
   baseDensity *= heightFraction;
   float cloudDensity = remap(baseDensity, (1.0 - cloudCoverage), 1.0, 0.0, 1.0);
   cloudDensity *= heightGradient;

   cloudDensity *= uSettings.densityMultiplier;

   // Add high-frequency detail based on detailStrength
   float detailModulation = mix(1.0, highFreqFBM, uSettings.detailStrength);
   cloudDensity *= detailModulation;

   return clamp(cloudDensity, 0.0, 1.0);
}

// HenyeyGreenstein phase function
float henyeyGreenstein(float cosTheta, float g)
{
   float gg = g * g;
   float denom = 1.0 + gg - 2.0 * g * cosTheta;
   return ((1.0 - gg) / (denom * sqrt(denom))) * (1.0 / (4.0 * M_PI));
}

float raymarchToLight(vec3 rayOrigin, vec3 rayDir)
{
   const int numSteps = MAX_RAYMARCH_LIGHT_STEPS;
   const float stepSize = uSettings.lightStepLength;

   float lightTransmittance = 1.0;

   vec3 step = stepSize * rayDir;
   vec3 rayPos = rayOrigin + step;

   for (int i = 0; i < numSteps; ++i)
   {
      float density = sampleCloudDensity(rayPos);
      if (density > EPSILON)
      {
         lightTransmittance *= exp(-density * uSettings.lightAbsorption * stepSize);
         if (lightTransmittance <= EPSILON) break;
      }

      rayPos += step;
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
   float radiance      = 0.0;

   float prevDensity = 0.0;

   // constants
   const float hgCos = dot(rayDir, SUN_DIR);

   // Log‑biased steps parameters
   const int   numSteps = MAX_RAYMARCH_STEPS;


   float prevRayDst = 0.0;
   for (int i = 0; i < numSteps; ++i)
   {
      // update raymarching position
      float progress = float(i + 1) / float(numSteps);
      float rayDst   = progress * rayLength;
      float stepSize = rayDst - prevRayDst;
      vec3  rayPos   = start + rayDir * rayDst;

      prevRayDst = rayDst;

      float density = sampleCloudDensity(rayPos);
      float avgDensity = (density + prevDensity) * 0.5;
      prevDensity = density;
      if (density < EPSILON)
         continue;

      float lightTransmittance = raymarchToLight(rayPos, -SUN_DIR);
      float phase = henyeyGreenstein(dot(rayDir, -SUN_DIR), uSettings.phaseEccentricity);
      phase *= 4.0 * M_PI; // normalize the phase value
      float T = exp(-avgDensity * uSettings.lightAbsorption * stepSize);

      transmittance *= T;
      radiance += transmittance * lightTransmittance * phase * avgDensity * stepSize;

      if (transmittance <= 0.01)
      {
         transmittance = 0.0;
         break;
      }
   }

   radiance = clamp(radiance, 0.0, 1.0);

   float alpha = 1.0 - transmittance;
   vec3 lightColor = SUN_COLOR;
   vec3 color = lightColor * radiance;

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