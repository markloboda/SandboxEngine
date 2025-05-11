#version 450

layout(std140, binding = 0) uniform CameraUniform
{
   mat4 view;
   mat4 proj;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragPosition;

void main()
{
   fragNormal = inNormal;
   vec4 worlsPos = proj * view * vec4(inPosition, 1.0);
   fragPosition = worlsPos.xyz / worlsPos.w;
   gl_Position = worlsPos;
}