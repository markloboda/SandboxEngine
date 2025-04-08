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

   int cloudNumSteps;
   int lightNumSteps;
} uSettings;

layout(location = 0) in vec2 uv;

// outputs
layout(location = 0) out vec4 fragCol;

// constants
#define EPSILON 1e-5

// define settings
#define CLOUD_START_HEIGHT uSettings.cloudStartHeight
#define CLOUD_END_HEIGHT uSettings.cloudEndHeight

#define DENSITY_MULTIPLIER uSettings.densityMultiplier
#define DENSITY_THRESHOLD uSettings.densityThreshold

#define CLOUD_MARCH_STEPS_COUNT uSettings.cloudNumSteps
#define LIGHT_MARCH_STEPS_COUNT uSettings.lightNumSteps

// constant settings
#define NOISE_SCALING_FACTOR 0.00001
#define SUN_DIR vec3(0.577, -0.577, 0.577)

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
   float res = 0.0;

   if (rayOrigin.y < CLOUD_START_HEIGHT)
   {
      // Ray starts below the cloud layer
      if (rayDir.y < EPSILON)
      {
         // no intersection
         res = -1.0;
      }
      else 
      {
         res = (CLOUD_START_HEIGHT - rayOrigin.y) / rayDir.y;
      }
   }
   else if (rayOrigin.y > CLOUD_END_HEIGHT)
   {
      // Ray starts above the cloud layer
      if (rayDir.y > -EPSILON)
      {
         // no intersection
         res = -1.0;
      }
      else
      {
         res = (CLOUD_END_HEIGHT - rayOrigin.y) / rayDir.y;
      }
   }
   else
   {
      // Ray starts inside the cloud layer
      res = 0.0;
   }

   return res;
}

float getCloudInsideDistance(vec3 rayOrigin, vec3 rayDir)
{
   float res = 0.0;

   if (rayOrigin.y < CLOUD_START_HEIGHT)
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
         res = (CLOUD_END_HEIGHT - CLOUD_START_HEIGHT) / rayDir.y;
      }
   }
   else if (rayOrigin.y > CLOUD_END_HEIGHT)
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
         res = (CLOUD_START_HEIGHT - CLOUD_END_HEIGHT) / rayDir.y;
      }
   }
   else
   {
      // Ray starts inside the cloud layer
      if (abs(rayDir.y) < EPSILON)
      {
         // Ray is parallel to the cloud layer, default distance
         res = 500.0;
      }
      else if (rayDir.y > 0)
      {
         // Ray is going up
         res = (CLOUD_END_HEIGHT - rayOrigin.y) / rayDir.y;
      }
      else
      {
         // Ray is going down
         res = (CLOUD_START_HEIGHT - rayOrigin.y) / rayDir.y;
      }
   }

   return res;
}

float sampleDensity(vec3 pos)
{
   if (pos.y < CLOUD_START_HEIGHT || pos.y > CLOUD_END_HEIGHT)
      return 0.0;

   vec3 noiseCoord = fract(pos * NOISE_SCALING_FACTOR);
   float noise = texture(sampler3D(cloudTexture, cloudSampler), noiseCoord).r;
   float density = max(0.0, noise - DENSITY_THRESHOLD) * DENSITY_MULTIPLIER;

   // height gradient
   float heightGradient = smoothstep(CLOUD_START_HEIGHT, CLOUD_START_HEIGHT + 100.0, pos.y) * 
                          (1.0 - smoothstep(CLOUD_END_HEIGHT - 100.0, CLOUD_END_HEIGHT, pos.y));
   density *= heightGradient;

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
      while (dstTravelled < dstLimit)
      {
         vec3 rayPos = rayEntryPoint + rayDir * dstTravelled;
         float density = sampleDensity(rayPos);
         if (density > 0.0)
         {
            float lightTransmittance = raymarchToLight(rayPos, SUN_DIR);
            lightEnergy += density * stepSize * transmittance * lightTransmittance;
            transmittance *= exp(-density * stepSize);

            if (transmittance < 0.01)
            {
               // Early exit if transmittance is very low
               break;
            }
         }

         dstTravelled += stepSize;
      }
   }

   // Output color
   vec3 cloudCol = vec3(1.0, 1.0, 1.0) * (1.0 - lightEnergy);
   fragCol = vec4(cloudCol, 1.0 - transmittance);
}