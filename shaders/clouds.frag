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
   // Position
   float cloudStartHeight; // height of the bottom of the cloud layer
   float cloudEndHeight; // height of the top of the cloud layer

   // Densitites
   float coverageMultiplier; // scales the coverage read from the weather map
   float densityMultiplier; // scales the final computed cloud density
   float highFreqThreshold; // threshold for high frequency detail noise application
   float detailBlendStrength; // strength of the detail noise blending

   // Lighting
   float ambientLight; // ambient light intensity for the clouds
   float lightAbsorption; // how strongly light is absorbed (scattering falloff)
   float henyeyGreensteinStrength; // strength of the phase function, 0.0 = no phase function and 1.0 = full phase function
   float phaseEccentricity; // eccentricity for Henyey-Greenstein phase function
   float lightRayConeAngle; // angle of the light ray cone for raymarchToLight() in radians

   // Post Processing
   float toneMappingStrength; // strength of the tone mapping applied to the final cloud color
   float contrastGamma; // contrast gamma for the final cloud colors

   // Performance
   int cloudRaymarchSteps; // number of steps in raymarch()
   int lightRaymarchSteps; // number of steps in raymarchToLight()
   float lightStepLength; // step size in raymarchToLight()
   float coverageCullThreshold; // threshold for culling clouds based on coverage
   int  dynamicStep; // whether to use dynamic step size in raymarch()
   float stepSizeFarMultiplier; // far step size for raymarching
   float stepSizeNearMultiplier; // near step size for raymarching
   float maxEmptySteps; // maximum number of empty steps
} uSettings;

layout(set = 1, binding = 3) uniform CloudRenderWeather
{
   vec4 sunDirection;
   vec4 detailNoiseOffset;
} uWeather;

layout(set = 1, binding = 4) uniform CloudData
{
   vec2 position;
} uCloudData;

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragCol;

// math constants
#define EPSILON 1e-5
#define M_PI 3.14159265358979323846

// physical settings
#define SUN_COLOR vec3(1.0, 1.0, 1.0)
#define AMBIENT_COLOR vec3(0.45, 0.52, 0.61)

// raymarching settings
#define MAX_RAYMARCH_DISTANCE 512000.0
#define CLOUD_MAP_SCALING_FACTOR _cloudMapScalingFactor
const float _cloudMapScalingFactor = 1.0 / 25600.0;
const float _lowFreqTextureScale = 1.0 / 6000.0; // base scale for low frequency detail texture
const float _highFreqTextureScale = 1.0 / 1200.0; // base scale for high frequency detail texture

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

vec3 toneMapGamma(vec3 color, float strength)
{
   // strength = 0.0 → no tone mapping, strength = 1.0 → full gamma
   vec3 gammaCorrected = pow(color, vec3(1.0 / 2.2));
   return mix(color, gammaCorrected, strength);
}

vec3 toneMapSmoothstep(vec3 color, float strength)
{
   vec3 toneMapped = smoothstep(vec3(0.0), vec3(1.0), color);
   return mix(color, toneMapped, strength);
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
   return clampRemap(mappedHeight, 0.0, min, 0.0, 1.0) * (1.0 - clampRemap(mappedHeight, center, max, 0.0, 1.0));
}

// Calculate the cloud height fraction for the given position and cloud type
float getCloudTypeFraction(float height, float cloudType)
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
   float scale = _lowFreqTextureScale;
   vec3 detailTextureCoord = pos * scale + uWeather.detailNoiseOffset.xyz;
   vec4 detailSample = texture(sampler3D(cloudBaseLowFreqTexture, cloudBaseLowFreqSampler), detailTextureCoord);
   return detailSample;
}

// Sample cloud detail high frequency noise (3D texture)
vec3 sampleCloudDetailHighFreq(in vec3 pos)
{
   float scale = _highFreqTextureScale;
   vec3 detailTextureCoord = pos * scale;
   vec3 detailSample = texture(sampler3D(cloudBaseHighFreqTexture, cloudBaseHighFreqSampler), detailTextureCoord).rgb;
   return detailSample;
}

// Get cloud density at world position
float sampleCloudDensity(in vec3 pos)
{
   vec4  mapSample = sampleCloudMap(pos);
   float coverage  = clamp(mapSample.r * uSettings.coverageMultiplier, 0.0, 1.0);
   float cloudType = mapSample.b;
   vec4 detailLowFreqSample  = sampleCloudDetailLowFreq(pos);

   float lowFreqFBM = clamp(dot(detailLowFreqSample.gba, vec3(0.625, 0.25, 0.125)), 0.0, 1.0);
   float baseCloud = remap(detailLowFreqSample.r, -(1.0 - lowFreqFBM), 1.0, 0.0, 1.0);
   baseCloud *= getCloudTypeFraction(pos.y, cloudType);

   float heightFraction    = clampRemap(pos.y, uSettings.cloudStartHeight, uSettings.cloudEndHeight, 0.0, 1.0);
   float cloudWithCoverage = remap(baseCloud, (1.0 - coverage), 1.0, 0.0, 1.0);
   cloudWithCoverage *= heightFraction;

   // Erode edges.
   vec3 detailHighFreq = sampleCloudDetailHighFreq(pos).rgb;
   float highFreqFBM = clamp(dot(detailHighFreq, vec3(0.625, 0.25, 0.125)), 0.0, 1.0);
   if (cloudWithCoverage < uSettings.highFreqThreshold) {
      float eroded = remap(cloudWithCoverage, (1.0 - highFreqFBM), 1.0, 0.0, 1.0);
      cloudWithCoverage = mix(cloudWithCoverage, eroded, uSettings.detailBlendStrength);
   }

   return clamp(cloudWithCoverage, 0.0, 1.0);
}

float henyeyGreensteinPhase(float cosTheta, float g)
{
   float g2 = g * g;
   float denom = 1.0 + g2 - 2.0 * g * cosTheta;
   return (1.0 - g2) / (4.0 * M_PI * pow(denom, 1.5));
}

float raymarchToLight(vec3 rayOrigin, vec3 rayDir, float coneAngle)
{
   const int numSteps = uSettings.lightRaymarchSteps;
   const float stepSize = uSettings.lightStepLength;

   float lightTransmittance = 1.0;

   // Tangent basis for cone deviation
   vec3 up = abs(rayDir.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
   vec3 right = normalize(cross(up, rayDir));
   vec3 forward = normalize(cross(rayDir, right));

   vec3 rayPos = rayOrigin;

   for (int i = 0; i < numSteps; ++i)
   {
      // Generate a random offset within the cone (same per-frame per-pixel)
      float r1 = fract(sin(dot(rayPos.xy + float(i), vec2(12.9898, 78.233))) * 43758.5453);
      float r2 = fract(sin(dot(rayPos.zy + float(i) * 1.37, vec2(39.3468, 11.135))) * 96321.9124);
      float theta = r1 * 2.0 * M_PI;
      float radius = r2 * coneAngle;

      vec2 offset = vec2(cos(theta), sin(theta)) * radius;
      vec3 coneDir = normalize(rayDir + offset.x * right + offset.y * forward);

      // Apply the offset direction per step
      float density = sampleCloudDensity(rayPos);
      if (density > EPSILON)
      {
         lightTransmittance *= exp(-density * uSettings.lightAbsorption * stepSize);
         if (lightTransmittance <= 0.001)
         { break; }
      }

      rayPos += coneDir * stepSize;
   }

   return lightTransmittance;
}

vec4 raymarch(vec3 start, vec3 end)
{
   const vec3  ray = end - start;
   const float rayLength = length(ray);
   const vec3  rayDir = ray / rayLength;

   const float hgCos = dot(rayDir, uWeather.sunDirection.xyz);

   float transmittance = 1.0;
   vec3  lightColor = vec3(0.0);
   float prevDensity = 0.0;

   float rayDst = 0.0;

   float stepSize, stepSizeFar, stepSizeNear;
   if (uSettings.dynamicStep == 1)
   {
      float stepSizeNormal = rayLength / float(uSettings.cloudRaymarchSteps);
      // Dynamic step size based on distance
      stepSizeFar = stepSizeNormal * uSettings.stepSizeFarMultiplier;
      stepSizeNear = stepSizeNormal * uSettings.stepSizeNearMultiplier;
      stepSize = stepSizeFar;
   }
   else
   {
      stepSize = rayLength / float(uSettings.cloudRaymarchSteps);
   }

   int   emptySteps = 0;
   bool  inCloud = false;

   for (int i = 0; i < uSettings.cloudRaymarchSteps; ++i)
   {
      if (rayDst > rayLength)
      break;

      vec3 rayPos = start + rayDir * rayDst;
      float density = sampleCloudDensity(rayPos);

      if (density > 0.0)
      {
         if (uSettings.dynamicStep == 1 && !inCloud)
         {
            // Hit cloud: step back and reduce step size
            rayDst -= stepSize;
            stepSize = stepSizeNear;
            inCloud = true;
            emptySteps = 0;
            continue;
         }

         float avgDensity = 0.5 * (prevDensity + density);
         prevDensity = density;

         float lightT = raymarchToLight(rayPos, uWeather.sunDirection.xyz, uSettings.lightRayConeAngle);
         float phase = mix(1.0, henyeyGreensteinPhase(hgCos, uSettings.phaseEccentricity), uSettings.henyeyGreensteinStrength);

         float T = exp(-avgDensity * uSettings.lightAbsorption * stepSize);
         transmittance *= T;

         float scatterProb = 1.0 - exp(-avgDensity);
         float inscatter = lightT * phase * scatterProb;
         float ambient = uSettings.ambientLight * scatterProb;

         lightColor += transmittance * (inscatter * SUN_COLOR + ambient * AMBIENT_COLOR) * stepSize;

         if (transmittance <= 0.01)
         {
            transmittance = 0.0;
            break;
         }
      }
      else
      {
         if (uSettings.dynamicStep == 1 && inCloud)
         {
            emptySteps++;
            if (emptySteps >= uSettings.maxEmptySteps)
            {
               // No cloud for a while, increase step size again
               stepSize = stepSizeFar;
               inCloud = false;
               emptySteps = 0;
            }
         }
      }

      rayDst += stepSize;
   }

   lightColor = toneMapGamma(lightColor, uSettings.toneMappingStrength);
   lightColor = pow(lightColor, vec3(uSettings.contrastGamma));
   float alpha = 1.0 - transmittance;
   return vec4(lightColor, alpha);
}

void main()
{
   vec3 rayOrigin = uCamera.pos - vec3(uCloudData.position.x, 0.0, uCloudData.position.y);
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
   fragCol = clamp(fragCol, 0.0, 1.0);
}