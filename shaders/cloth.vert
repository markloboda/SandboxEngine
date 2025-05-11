#version 450

layout(location = 0) in vec3 inPosition;

layout(std140, binding = 0) uniform CameraUniform
{
   mat4 view;
   mat4 proj;
};

void main()
{
   int instance_id = gl_InstanceIndex;
   gl_Position = proj * view * vec4(inPosition, 1.0);
   gl_PointSize = 10.0;
}