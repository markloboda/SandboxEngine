#version 450

#define SUN_DIR vec3(0.577, -0.577, 0.577)

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main() {
   vec3 N = normalize(fragNormal);
   vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
   vec3 viewDir = normalize(-fragPosition);

   float diff = max(dot(N, SUN_DIR), 0.0);
   vec3 diffuse = vec3(1.0) * diff;

   outColor = vec4(diffuse, 1.0);
}
