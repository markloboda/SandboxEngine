#pragma once

struct GridUniforms
{
   alignas(16) mat4x4 view;
   alignas(16) mat4x4 proj;
   float gridSize;
   float gridSpacing;
   uint32_t numHorizontal;
   uint32_t numVertical;
};

static_assert(sizeof(GridUniforms) == 144, "GridUniforms size mismatch");

class GridRenderer
{
private:
   GLFWwindow* _window;
   Device* _device;

   Buffer* _vertexBuffer;
   Buffer* _uniformBuffer;

   RenderPipeline* _renderPipeline;

   ShaderModule _vertexShader;
   ShaderModule _fragmentShader;

   GridUniforms _uniformsData;

   BindGroup* _uniformsBindGroup;

public:
   GridRenderer(GLFWwindow* window, Device* device);
   ~GridRenderer();

private:
   bool Initialize(Device* device);
   void Terminate();

public:
   void Render(RenderPassEncoder* renderPassEncoder);
};