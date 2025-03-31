#version 450 core

layout(location = 0) in float vertex_pos;

layout(std140, binding = 0) uniform GridUniforms {
   mat4 view;
   mat4 proj;
   float gridSpacing;
   uint numHorizontal;
   uint numVertical;
};

void main() {
   uint instance_id = gl_InstanceIndex;
   float x, y, z;

   vec3 cameraPos = vec3(inverse(view)[3]);

   float dynamicGridSpacing = gridSpacing; //floor((abs(cameraPos.y) + 10.0) / 10.0) * gridSpacing;

   float snappedX = round(cameraPos.x / gridSpacing);
   float snappedZ = round(cameraPos.z / gridSpacing);

   float gridSizeX = dynamicGridSpacing * float(numVertical);
   float gridSizeZ = dynamicGridSpacing * float(numHorizontal);

   if (instance_id < numHorizontal) {
      // Horizontal grid lines
      int zIndex = int(instance_id - (numHorizontal / 2));
      x = (vertex_pos * gridSizeX) + snappedX;
      y = 0.0;
      z = ((zIndex + 0.5) * dynamicGridSpacing) + snappedZ;
   } else {
      // Vertical grid lines
      int xIndex = int((instance_id - numHorizontal) - (numVertical / 2));
      x = ((xIndex + 0.5) * dynamicGridSpacing) + snappedX;
      y = 0.0;
      z = (vertex_pos * gridSizeZ) + snappedZ;
   }

   gl_Position = proj * view * vec4(x, y, z, 1.0);
}