#pragma once

struct GridUniforms
{
   alignas(16) float viewProj[16];
   float gridSize;
   float gridSpacing;
   uint32_t numHorizontal;
   uint32_t numVertical;
};

static_assert(sizeof(GridUniforms) == 80, "GridUniforms size mismatch");

class GridRenderer
{
private:
   Buffer* _vertexBuffer;
   Buffer* _uniformBuffer;

   WGPUPipelineLayout _pipelineLayout;
   WGPURenderPipeline _renderPipeline;

   ShaderModule _vertexShader;
   ShaderModule _fragmentShader;

   GridUniforms _uniformsData;

   BindGroup* _uniformsBindGroup;

public:
   GridRenderer(Device* device);
   ~GridRenderer();

private:
   bool Initialize(Device* device);
   void Terminate();

public:
   void Render();
};