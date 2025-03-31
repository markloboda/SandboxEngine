#version 450

layout(location = 0) out vec2 uv;

vec2 positions[3] = vec2[](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));

void main() {
    uv = (positions[gl_VertexIndex] + 1.0) * 0.5; // Convert to [0,1] UV space
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}