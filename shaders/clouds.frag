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

   int cloudNumSteps;
   int lightNumSteps;
   float cloudMaxStepSize;
} uSettings;

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragCol;

// constants
#define EPSILON 1e-5

//  settings
#define CLOUD_HEIGHT_GRADIENT 200.0
#define CLOUD_ABSORBTION 1.0
#define WEATHER_MAP_SCALING_FACTOR 1.0 / 10000.0 // covers 10km x 10km
#define SUN_DIR vec3(0.577, -0.577, 0.577)
#define CLOUD_BASE_DENSITY_MULTIPLIER 0.1
#define CLOUD_DETAIL_TEXTURE_SCALING_FACTOR 1 / 5000.0

#define SUN_COLOR vec3(1.0, 1.0, 1.0)
#define AMBIENT_COLOR vec3(0.1, 0.1, 0.1)

// Cloud type functions
#define STRATUS_OFFSET vec2(0.0, 0.2)
#define STRATOCUMULUS_OFFSET vec2(0.0, 0.5)
#define CUMULUS_OFFSET vec2(0.0, 0.8)

// helper functions
vec3 getStartRayDirection(vec2 uv)
{
   vec4 ndc = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
   vec4 worldPos = inverse(uCamera.proj) * ndc;
   worldPos /= worldPos.w;
   vec3 rayDir = normalize((inverse(uCamera.view) * vec4(worldPos.xyz, 0.0)).xyz);
   return rayDir;
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
      cloudTypeHeight = mix(STRATUS_OFFSET, STRATOCUMULUS_OFFSET, cloudType);
   }
   else
   {
      // Stratocumulus or cumulus
      cloudTypeHeight = mix(STRATOCUMULUS_OFFSET, CUMULUS_OFFSET, cloudType - 0.5);
   }

   float mappedHeight = (height - uSettings.cloudStartHeight) / (uSettings.cloudEndHeight - uSettings.cloudStartHeight);

   // Smooth step from heigh 0.0 to 0.2
   // and from 0.8 to 1.0
   float fadeIn = smoothstep(cloudTypeHeight.x, cloudTypeHeight.x + 0.2, mappedHeight);
   float fadeOut = 1.0 - smoothstep(cloudTypeHeight.y - 0.2, cloudTypeHeight.y, mappedHeight);
   float heightFraction = fadeIn * fadeOut;

   return heightFraction;
}

float sampleDensity(vec3 pos)
{
   if (pos.y < uSettings.cloudStartHeight || pos.y > uSettings.cloudEndHeight)
      return 0.0;

   // Sample weather map.
   vec2 weatherMapCoord = pos.xz * WEATHER_MAP_SCALING_FACTOR;
   float cloudCoverage = 0.0;
   float cloudType = 0.0;
   if (weatherMapCoord.x > 0.0 && weatherMapCoord.x < 1 && weatherMapCoord.y > 0.0 && weatherMapCoord.y < 1)
   {
      vec4 weatherMapSample = texture(sampler2D(weatherMap, weatherMapSampler), weatherMapCoord);
      cloudCoverage = weatherMapSample.r;
      cloudType = weatherMapSample.g;
   }

   // Sample detail texture (3D texture).
   vec3 detailTextureCoord = pos * CLOUD_DETAIL_TEXTURE_SCALING_FACTOR;
   vec4 detailTextureSample = texture(sampler3D(cloudBaseTexture, cloudBaseSampler), detailTextureCoord);

   // Height fraction
   float heightFraction = getCloudHeightFraction(pos.y, cloudType);

   float lowFreqDensity = detailTextureSample.r;

   float density = max(0.0, heightFraction * lowFreqDensity * cloudCoverage - uSettings.densityThreshold) * CLOUD_BASE_DENSITY_MULTIPLIER * uSettings.densityMultiplier;

   return density;
}

// Calculate the distance to the cloud layer to entry point
float getCloudEntryDistance(vec3 rayOrigin, vec3 rayDir)
{
   float res = 0.0;

   if (rayOrigin.y < uSettings.cloudStartHeight)
   {
      // Ray starts below the cloud layer
      if (rayDir.y < EPSILON)
      // no intersection
      res = -1.0;
      else
      res = (uSettings.cloudStartHeight - rayOrigin.y) / rayDir.y;
   }
   else if (rayOrigin.y > uSettings.cloudEndHeight)
   {
      // Ray starts above the cloud layer
      if (rayDir.y > -EPSILON)
      // no intersection
      res = -1.0;
      else
      res = (uSettings.cloudEndHeight - rayOrigin.y) / rayDir.y;
   }
   else
   // Ray starts inside the cloud layer
   res = 0.0;

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
      // No intersection
      res = -1.0;
      else
      // Ray is going up
      res = (uSettings.cloudEndHeight - uSettings.cloudStartHeight) / rayDir.y;
   }
   else if (rayOrigin.y > uSettings.cloudEndHeight)
   {
      // Ray starts above the cloud layer
      if (rayDir.y > -EPSILON)
      // No intersection
      res = -1.0;
      else
      // Ray is going down
      res = (uSettings.cloudStartHeight - uSettings.cloudEndHeight) / rayDir.y;
   }
   else
   {
      // Ray starts inside the cloud layer
      if (abs(rayDir.y) < EPSILON)
      // Ray is parallel to the cloud layer, default distance
      res = 5000.0;
      else if (rayDir.y > 0)
      // Ray is going up
      res = (uSettings.cloudEndHeight - rayOrigin.y) / rayDir.y;
      else
      // Ray is going down
      res = (uSettings.cloudStartHeight - rayOrigin.y) / rayDir.y;
   }

   return res;
}

float raymarchToLight(vec3 rayOrigin, vec3 rayDir)
{
   float dstInsideCloud = getCloudInsideDistance(rayOrigin, rayDir);
   float stepSize = dstInsideCloud / float(uSettings.lightNumSteps);

   float totalDensity = 0.0;
   for (int i = 0; i < uSettings.lightNumSteps; i++)
   {
      vec3 rayPos = rayOrigin + rayDir * (i * stepSize);
      float density = sampleDensity(rayPos);
      totalDensity += density * stepSize;
   }

   float transmittance = exp(-totalDensity);
   return transmittance;
}

vec4 raymarch(vec3 start, vec3 end)
{
   // Ray
   vec3 ray = end - start;
   float rayLength = length(ray);
   vec3 rayDir = normalize(ray);

   // Raymarching parameters
   float transmittance = 1.0;
   float totalDensity = 0.0;
   float inscatteredLight = 0.0;
   float ambientLight = 0.0;

   // Start raymarching
   vec3  rayPos = start;
   float baseStepSize = min(uSettings.cloudMaxStepSize, rayLength / float(uSettings.cloudNumSteps));
   float curStepSize = baseStepSize;
   float remainingDst = rayLength;
   int curSteps = 0;
   while (remainingDst > 0.0)
   {
      if (curSteps >= uSettings.cloudNumSteps)
         break;
      curSteps++;

      float density = sampleDensity(rayPos);
      totalDensity += density;

      if (density > 0.0)
      {
         float lightTransmittance = raymarchToLight(rayPos, SUN_DIR);
         inscatteredLight += density * curStepSize * transmittance * lightTransmittance;
         ambientLight += density * transmittance;

         transmittance *= exp(-density * curStepSize * CLOUD_ABSORBTION);
      }

      // early exit
      if (transmittance < 0.001)
         break;

      // Move forward
      rayPos += rayDir * curStepSize;
      remainingDst -= curStepSize;
   }

   float alpha = 1.0 - transmittance;

   vec3 directCol  = SUN_COLOR * (1 - inscatteredLight);
   vec3 ambientCol = vec3(0.0);//AMBIENT_COLOR * totalDensity;

   return vec4(directCol + ambientCol, alpha);
}

void main()
{
   vec3 rayOrigin = uCamera.pos;
   vec3 rayDir = getStartRayDirection(uv);

   float dstToStart = getCloudEntryDistance(rayOrigin, rayDir);
   float dstInside  = getCloudInsideDistance(rayOrigin, rayDir);

   vec4 cloudCol;
   if (dstToStart < -EPSILON || dstInside < -EPSILON)
   {
      // No intersection with the cloud layer
      cloudCol = vec4(0.0);
      fragCol = cloudCol;
      return;
   }
   else
   {
      vec3 start = rayOrigin + rayDir * dstToStart;
      vec3 end = start + rayDir * (dstToStart + dstInside);

      cloudCol = raymarch(start, end);
   }

   fragCol = cloudCol;
}