#pragma once

class ClothParticleSystem;
class ClothRenderer
{
private:
   struct VertexData
   {
      vec3 position;
      vec3 normal;
   };

   ClothParticleSystem* _clothParticleSystem;

   struct CameraUniform
   {
      mat4 view;
      mat4 proj;
   };

   RenderPipeline* _renderPipeline;

   BindGroup* _cameraUniformBindGroup;
   Buffer *_cameraUniformBuffer = nullptr;
   Buffer *_vertexBuffer = nullptr;
   Buffer *_indexBuffer = nullptr;

   CameraUniform _cameraUniform = {
      .view = mat4(),
      .proj = mat4()
   };

public:
   ClothRenderer(Renderer *renderer);

   ~ClothRenderer();

private:
   bool Initialize(Renderer *renderer);
   void Terminate();

public:
   void Update(float dt);
   void Render(Renderer *renderer, CommandEncoder *encoder, TextureView *surfaceTextureView);

private:
   void GenerateVertexData(std::vector<VertexData> &outVertices, std::vector<uint32_t> &outIndices);
};
