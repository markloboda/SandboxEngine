#pragma once

class RenderPassEncoder
{
private:
   WGPURenderPassEncoder _encoder;

public:
   explicit RenderPassEncoder(WGPURenderPassEncoder encoder);
   ~RenderPassEncoder();

   [[nodiscard]] WGPURenderPassEncoder Get() const { return _encoder; }

   void SetPipeline(const RenderPipeline &pipeline) const;
   void SetVertexBuffer(uint32_t slot, const Buffer &buffer) const;
   void SetIndexBuffer(const Buffer &buffer, WGPUIndexFormat format) const;
   void SetBindGroup(uint32_t index, const BindGroup &bindGroup) const;
   void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const;
   void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) const;

   void EndPass() const;
};