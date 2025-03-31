#version 450 core

layout(location = 0) in uint VertexIndex;

void main() {
   vec2 pos[3] = vec2[3](
      vec2(-1.0, -1.0),
      vec2(3.0, -1.0),
      vec2(-1.0, 3.0)
   );
   gl_Position = vec4(pos[VertexIndex], 0.0, 1.0);
}