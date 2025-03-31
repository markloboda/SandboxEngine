#version 450

layout(set = 0, binding = 0) uniform texture3D cloudTexture;
layout(set = 0, binding = 1) uniform sampler cloudSampler;
layout(set = 0, binding = 2) uniform CameraData {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} camera;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

#define MAX_STEPS 64
#define STEP_SIZE 1.0
#define CLOUD_DENSITY_SCALE 3.0

// Generate a world-space ray from UV coordinates
vec3 getRayDirection(vec2 uv) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 worldPos = inverse(camera.proj) * ndc;
    worldPos /= worldPos.w;
    vec3 rayDir = normalize((inverse(camera.view) * vec4(worldPos.xyz, 0.0)).xyz);
    return rayDir;
}

// Sample the 3D cloud density texture
float sampleCloudDensity(vec3 pos) {
    vec3 cloudUV = pos * 0.005; // Scale world position to texture UV space
    return texture(sampler3D(cloudTexture, cloudSampler), cloudUV).r * CLOUD_DENSITY_SCALE;
}

// Raymarching algorithm for volumetric clouds
float raymarchClouds(vec3 rayOrigin, vec3 rayDir) {
    float transmittance = 1.0; // Start fully transparent
    float totalDensity = 0.0;
    
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 samplePos = rayOrigin + rayDir * (STEP_SIZE * i);
        
        float density = sampleCloudDensity(samplePos);
        totalDensity += density * STEP_SIZE;

        // Exponential absorption (Beer’s law)
        transmittance *= exp(-density * 0.1);
        if (transmittance < 0.01) break; // Early exit if fully opaque
    }
    
    return clamp(totalDensity, 0.0, 1.0);
}

// Simple lighting model for clouds
float computeLighting(vec3 pos, vec3 lightDir) {
    float lightIntensity = 0.5 + 0.5 * dot(normalize(pos), lightDir); // Fake scattering
    return lightIntensity;
}

void main() {
    vec3 rayDir = getRayDirection(uv);
    float cloudDensity = raymarchClouds(camera.cameraPos, rayDir);
    
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5)); // Example sun direction
    float lighting = computeLighting(rayDir, lightDir);

    vec3 cloudColor = mix(vec3(0.5, 0.6, 0.7), vec3(1.0), lighting) * cloudDensity;

    fragColor = vec4(cloudColor, cloudDensity);
}