#pragma once

class ClothRenderer
{
private:
   struct VertexData
   {
      vec3 position;
      vec3 normal;
   };

   struct CameraUniform
   {
      mat4 view;
      mat4 proj;
   };

   RenderPipeline *_renderPipeline;

   BindGroup *_cameraUniformBindGroup;
   Buffer *_cameraUniformBuffer = nullptr;
   Buffer *_vertexBuffer = nullptr;
   Buffer *_indexBuffer = nullptr;

   CameraUniform _cameraUniform = {
      .view = mat4(),
      .proj = mat4()
   };

public:
   explicit ClothRenderer(Renderer &renderer);
   ~ClothRenderer();

private:
   bool Initialize(Renderer &renderer);
   void Terminate() const;

public:
   void Render(const Renderer &renderer, const CommandEncoder &encoder, const TextureView &surfaceTextureView, uint32_t profilerIndex);

private:
   void GenerateVertexData(std::vector<VertexData> &outVertices, std::vector<uint32_t> &outIndices) const;
};
