#version 450

vec2 positions[3] = vec2[](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));

layout(location = 0) out vec2 uv;

void main() {
   uv = (positions[gl_VertexIndex] + 1.0) * 0.5;
   gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}