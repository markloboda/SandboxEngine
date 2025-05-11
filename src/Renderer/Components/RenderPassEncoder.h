#pragma once

class RenderPipeline;
class CommandEncoder;
class TextureView;

class RenderPassEncoder
{
private:
   WGPURenderPassEncoder _encoder;

public:
   RenderPassEncoder(WGPURenderPassEncoder encoder);
   ~RenderPassEncoder();

   WGPURenderPassEncoder Get() const { return _encoder; }

   void SetPipeline(RenderPipeline* pipeline);
   void SetVertexBuffer(uint32_t slot, Buffer* buffer);
   void RenderPassEncoder::SetIndexBuffer(Buffer *buffer, WGPUIndexFormat format);
   void SetBindGroup(uint32_t index, BindGroup* bindGroup);
   void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
   void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);

   void EndPass();
};