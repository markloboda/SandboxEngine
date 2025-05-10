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

#define CLOUD_START_HEIGHT uSettings.cloudStartHeight
#define CLOUD_END_HEIGHT uSettings.cloudEndHeight
#define DENSITY_MULTIPLIER uSettings.densityMultiplier
#define DENSITY_THRESHOLD uSettings.densityThreshold
#define CLOUD_MARCH_STEPS_COUNT uSettings.cloudNumSteps
#define LIGHT_MARCH_STEPS_COUNT uSettings.lightNumSteps
#define CLOUD_MAX_STEP_SIZE uSettings.cloudMaxStepSize

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragCol;

// constants
#define EPSILON 1e-5

//  settings
#define CLOUD_HEIGHT_GRADIENT 200.0
#define LIGHT_ABSORPTION 0.1
#define WEATHER_MAP_SCALING_FACTOR 1.0 / 10000.0 // covers 10km x 10km
#define SUN_DIR vec3(0.577, -0.577, 0.577)
#define CLOUD_BASE_DENSITY_MULTIPLIER 0.1
#define CLOUD_DETAIL_TEXTURE_SCALING_FACTOR 1 / 1000.0

// helper functions
vec3 getStartRayDirection(vec2 uv)
{
   vec4 ndc = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
   vec4 worldPos = inverse(uCamera.proj) * ndc;
   worldPos /= worldPos.w;
   vec3 rayDir = normalize((inverse(uCamera.view) * vec4(worldPos.xyz, 0.0)).xyz);
   return rayDir;
}

// Calculate the distance to the cloud layer
// to entry point
float getCloudEntryDistance(vec3 rayOrigin, vec3 rayDir)
{
   float res = 0.0;

   if (rayOrigin.y < CLOUD_START_HEIGHT)
   {
      // Ray starts below the cloud layer
      if (rayDir.y < EPSILON)
         // no intersection
         res = -1.0;
      else
         res = (CLOUD_START_HEIGHT - rayOrigin.y) / rayDir.y;
   }
   else if (rayOrigin.y > CLOUD_END_HEIGHT)
   {
      // Ray starts above the cloud layer
      if (rayDir.y > -EPSILON)
         // no intersection
         res = -1.0;
      else
         res = (CLOUD_END_HEIGHT - rayOrigin.y) / rayDir.y;
   }
   else
      // Ray starts inside the cloud layer
      res = 0.0;

   return res;
}

// Calculate the distance inside the cloud layer
// from entry point to the exit point
float getCloudInsideDistance(vec3 rayOrigin, vec3 rayDir)
{
   float res = 0.0;

   if (rayOrigin.y < CLOUD_START_HEIGHT)
   {
      // Ray starts below the cloud layer
      if (rayDir.y < EPSILON)
         // No intersection
         res = -1.0;
      else
         // Ray is going up
         res = (CLOUD_END_HEIGHT - CLOUD_START_HEIGHT) / rayDir.y;
   }
   else if (rayOrigin.y > CLOUD_END_HEIGHT)
   {
      // Ray starts above the cloud layer
      if (rayDir.y > -EPSILON)
         // No intersection
         res = -1.0;
      else
         // Ray is going down
         res = (CLOUD_START_HEIGHT - CLOUD_END_HEIGHT) / rayDir.y;
   }
   else
   {
      // Ray starts inside the cloud layer
      if (abs(rayDir.y) < EPSILON)
         // Ray is parallel to the cloud layer, default distance
         res = 5000.0;
      else if (rayDir.y > 0)
         // Ray is going up
         res = (CLOUD_END_HEIGHT - rayOrigin.y) / rayDir.y;
      else
         // Ray is going down
         res = (CLOUD_START_HEIGHT - rayOrigin.y) / rayDir.y;
   }

   return res;
}

float sampleDensity(vec3 pos)
{
   if (pos.y < CLOUD_START_HEIGHT || pos.y > CLOUD_END_HEIGHT)
      return 0.0;

   // Sample weatherMap
   vec2 weatherMapCoord = pos.xz * WEATHER_MAP_SCALING_FACTOR;

   float weatherMapValue = 0.0;
   if (weatherMapCoord.x > 0.0 && weatherMapCoord.x < 1 && weatherMapCoord.y > 0.0 && weatherMapCoord.y < 1)
   {
      weatherMapValue = texture(sampler2D(weatherMap, weatherMapSampler), weatherMapCoord).r;
   }

   // height gradient
   float gradientHeight = CLOUD_HEIGHT_GRADIENT;
   float gradient = smoothstep(CLOUD_START_HEIGHT, CLOUD_START_HEIGHT + gradientHeight, pos.y) *
                          (1.0 - smoothstep(CLOUD_END_HEIGHT - gradientHeight, CLOUD_END_HEIGHT, pos.y));

   // Detail texture
   vec3 detailTextureCoord = pos * CLOUD_DETAIL_TEXTURE_SCALING_FACTOR;
   float detailTextureValue = texture(sampler3D(cloudBaseTexture, cloudBaseSampler), detailTextureCoord).x;

   // Calculate density
   float density = max(0.0, weatherMapValue * gradient * detailTextureValue - DENSITY_THRESHOLD) * CLOUD_BASE_DENSITY_MULTIPLIER * DENSITY_MULTIPLIER;

   return density;
}

float raymarchToLight(vec3 rayOrigin, vec3 rayDir)
{
   float dstInsideCloud = getCloudInsideDistance(rayOrigin, rayDir);
   float stepSize = dstInsideCloud / float(LIGHT_MARCH_STEPS_COUNT);

   float totalDensity = 0.0;
   for (int i = 0; i < LIGHT_MARCH_STEPS_COUNT; i++)
   {
      vec3 rayPos = rayOrigin + rayDir * (i * stepSize);
      float density = sampleDensity(rayPos);
      totalDensity += density * stepSize;
   }

   float transmittance = exp(-totalDensity);
   return transmittance;
}

void main()
{
   vec3 rayOrigin = uCamera.pos;
   vec3 rayDir = getStartRayDirection(uv);

   int numSteps = CLOUD_MARCH_STEPS_COUNT;

   float dstToCloud = getCloudEntryDistance(rayOrigin, rayDir);
   vec3 rayEntryPoint = rayOrigin + rayDir * dstToCloud;
   float dstInsideCloud = getCloudInsideDistance(rayOrigin, rayDir);
   float dstLimit = dstInsideCloud;
   float stepSize = dstLimit / float(numSteps);
   stepSize = min(stepSize, CLOUD_MAX_STEP_SIZE); // Prevent too large step size

   float dstTravelled = 0.0;
   float transmittance = 1.0;
   float lightEnergy = 0.0;
   if (dstToCloud < -EPSILON || dstLimit < -EPSILON)
   {
      // Ray does not intersect the cloud layer
   }
   else
   {
      // Ray march cloud.
      for (int i = 0; i < numSteps; i++)
      {
         vec3 rayPos = rayEntryPoint + rayDir * dstTravelled;
         float density = sampleDensity(rayPos);
         if (density > 0.0)
         {
            float lightTransmittance = raymarchToLight(rayPos, SUN_DIR);
            lightEnergy += density * stepSize * transmittance * lightTransmittance;
            transmittance *= exp(-density * stepSize);

            if (transmittance < 0.01)
               // Early exit if transmittance is very low
               break;
         }

         dstTravelled += stepSize;
      }
   }

   // Output color
   vec3 cloudCol = vec3(1.0, 1.0, 1.0) * (1.0 - lightEnergy);
   fragCol = vec4(cloudCol, 1.0 - transmittance);
}