#version 450

// inputs
layout(set = 0, binding = 0) uniform texture3D cloudTexture;

layout(set = 0, binding = 1) uniform sampler cloudSampler;

layout(set = 0, binding = 2) uniform CameraData {
   mat4 view;
   mat4 proj;
   vec3 pos;
} uCamera;

layout(set = 0, binding = 3) uniform ResolutionData {
   vec2 xy;
} uResolution;

layout(location = 0) in vec2 uv;


// outputs
layout(location = 0) out vec4 fragColor;


// define settings
#define CLOUD_START_HEIGHT 500.0
#define CLOUD_END_HEIGHT 1500.0
#define DENSITY_MULTIPLIER 0.15

const vec3 LIGHT_DIR = normalize(vec3(0.8, -0.6, 0.2));

// helper functions
vec3 getRayDirection(vec2 uv) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 worldPos = inverse(uCamera.proj) * ndc;
    worldPos /= worldPos.w;
    vec3 rayDir = normalize((inverse(uCamera.view) * vec4(worldPos.xyz, 0.0)).xyz);
    return rayDir;
}

float getCloudEntryDistance(vec3 rayOrigin, vec3 rayDir) {
   // Ray starts inside the cloud layer
   if (rayOrigin.y > CLOUD_START_HEIGHT && rayOrigin.y < CLOUD_END_HEIGHT)
      return 0.0;

   // Ray is parallel to the cloud layer (no intersection)
   if (abs(rayDir.y) < 1e-5)
      return -1.0;

   float t = 0.0;
   if (rayOrigin.y < CLOUD_START_HEIGHT) {
      // Ray starts below the cloud layer
      t = (CLOUD_START_HEIGHT - rayOrigin.y) / rayDir.y;
      
   } else if (rayOrigin.y > CLOUD_END_HEIGHT) {
      // Ray starts above the cloud layer
      t = (CLOUD_END_HEIGHT - rayOrigin.y) / rayDir.y;
   }

   // We only care about intersections in front of the camera
   return (t > 0.0) ? t : -1.0;
}

float getCloudDensity(vec3 pos) {
   if (pos.y < CLOUD_START_HEIGHT || pos.y > CLOUD_END_HEIGHT)
      return 0.0;

   float noiseScalingFactor = 0.000005;
   vec3 noiseCoord = fract(pos * noiseScalingFactor);
   float noise = texture(sampler3D(cloudTexture, cloudSampler), noiseCoord).r;
   float density = (noise - 0.5) * DENSITY_MULTIPLIER;
   density = clamp(density, 0.0, 1.0);
   return density;
}

float raymarchToLight(vec3 pos) {
    float shadow = 1.0;
    for(int s = 0; s < 8; s++) {
        pos += LIGHT_DIR * 2.0;
        shadow -= getCloudDensity(pos) * 0.2;
    }
    return clamp(shadow, 0.3, 1.0);
}

void main() {
   vec3 rayOrigin = uCamera.pos;
   vec3 rayDir = getRayDirection(uv);
   
   float t = min(getCloudEntryDistance(rayOrigin, rayDir), 10000); // total distance traveled
   
   float stepSize = 0.7;
   int stepCount = 1024;

   float accDensity = 0.0;
   if (t >= 0.0) {
      for (int i = 0; i < stepCount; i++) {
         vec3 rayPos = rayOrigin + rayDir * t;

         float densitySample = getCloudDensity(rayPos) * stepSize;
         accDensity += densitySample;
         if (accDensity > 1.0) {
            accDensity = 1.0;
            break;
         }

         // Adjust stepSize based on density
         if (densitySample > 0.1) {
            stepSize = 0.5;
         } else {
            stepSize *= 1.01;
         }

         t += stepSize;
      }
   }

   fragColor = vec4(vec3(1), accDensity);
}