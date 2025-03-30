#version 450 core

layout(location = 0) in float vertex_pos;

layout(std140, binding = 0) uniform GridUniforms {
    mat4 viewProj;
    float gridSize;
    float gridSpacing;
    uint numHorizontal;
    uint numVertical;
};

void main() {
    uint instance_id = gl_InstanceIndex;
    float x, y, z;

    if (instance_id < numHorizontal) {
        // Horizontal line
        z = (instance_id * gridSpacing) - gridSize;
        x = vertex_pos * 2.0 * gridSize - gridSize;
        y = 0.0;
    } else {
        // Vertical line
        uint vertical_id = instance_id - numHorizontal;
        x = (vertical_id * gridSpacing) - gridSize;
        z = vertex_pos * 2.0 * gridSize - gridSize;
        y = 0.0;
    }

    gl_Position = viewProj * vec4(x, y, z, 1.0);
}