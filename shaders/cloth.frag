#version 450

#define SUN_DIR vec3(0.577, -0.577, 0.577)

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main() {
   vec3 baseColor = vec3(0.8, 0.5, 0.9);
   vec3 N = normalize(gl_FrontFacing ? fragNormal : -fragNormal);

   float diff = max(dot(N, SUN_DIR), 0.0);
   vec3 diffuse = baseColor * diff;

   vec3 ambient = baseColor * 0.2;

   outColor = vec4(diffuse + ambient, 1.0);
}
