#version 450 core

layout(set = 0, binding = 0) uniform ShaderParamsUniform {
   float time;
   vec2 resolution;
   vec3 cameraPos;
} params;

layout(binding = 1) uniform texture2D noiseTexture;
layout(binding = 2) uniform sampler noiseSampler;

layout(location = 0) out vec4 outColor;

// Forward declarations
float sampleCloudDensity(vec3 pos);
float calculateLighting(vec3 pos);

void main() {
   vec2 uv = gl_FragCoord.xy / params.resolution;
   
   // Ray setup
   vec3 rayOrigin = params.cameraPos;
   vec3 rayDir = normalize(vec3((uv - 0.5) * vec2(2.0 * (params.resolution.x/params.resolution.y), -2.0), 1.0));
   
   // Ray marching parameters
   float t = 0.0;
   vec4 cloudColor = vec4(0.0);
   const int steps = 128;
   const float stepSize = 0.2;
   
   for (int i = 0; i < steps; i++) {
      vec3 pos = rayOrigin + rayDir * t;
   
      // Sample density
      float density = sampleCloudDensity(pos);
      
      // Light calculation
      float light = calculateLighting(pos);
      
      // Accumulate color
      float transmittance = exp(-cloudColor.a * stepSize);
      cloudColor.rgb += transmittance * density * light * stepSize;
      cloudColor.a += density * stepSize;
      
      t += stepSize;

      
      
      if (cloudColor.a > 0.99) {
         break;
      }
   }
   
   outColor = cloudColor;
}

float sampleCloudDensity(vec3 pos) {
   // Simple noise implementation
   float noise = texture(sampler2D(noiseTexture, noiseSampler), pos.xy * 0.1).r;
   noise = fract(noise * 12345.6789);
   return max(0.0, noise - 0.5) * 2.0;
}

float calculateLighting(vec3 pos) {
   // Simple lighting
   return 1.0;
}