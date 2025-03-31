#version 450 core

layout(set = 0, binding = 0) uniform Params {
    float time;
    vec2 resolution;
    vec3 cameraPos;
} params;

layout(binding = 1) uniform texture2D noiseTexture;
layout(binding = 2) uniform sampler noiseSampler;

layout(location = 0) out vec4 outColor;

void main() {
}
