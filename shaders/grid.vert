#version 450 core

layout(location = 0) in float vertex_pos;

layout(std140, binding = 0) uniform GridUniforms {
    mat4 view;
    mat4 proj;
    float gridSize;
    float gridSpacing;
    uint numHorizontal;
    uint numVertical;
};

void main() {
    uint instance_id = gl_InstanceIndex;
    float x, y, z;

    vec3 camera_pos = vec3(inverse(view)[3]);

    // Snap camera position to the nearest grid line
    float snapped_x = round(camera_pos.x / gridSpacing) * gridSpacing;
    float snapped_z = round(camera_pos.z / gridSpacing) * gridSpacing;

    if (instance_id < numHorizontal) {
        // Horizontal line
        z = (instance_id * gridSpacing) - gridSize / 2 + snapped_z;
        x = vertex_pos * 2.0 * gridSize - gridSize / 2 + snapped_x;
        y = 0.0;
    } else {
        // Vertical line
        uint vertical_id = instance_id - numHorizontal;
        x = (vertical_id * gridSpacing) - gridSize / 2 + snapped_x;
        z = vertex_pos * 2.0 * gridSize - gridSize / 2 + snapped_z;
        y = 0.0;
    }

    gl_Position = proj * view * vec4(x, y, z, 1.0);
}