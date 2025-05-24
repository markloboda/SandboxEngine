#include <pch.h>

RenderPassEncoder::RenderPassEncoder(WGPURenderPassEncoder encoder) : _encoder(encoder)
{
   if (!_encoder)
   {
      std::cerr << ("Failed to begin render pass");
   }
}

RenderPassEncoder::~RenderPassEncoder()
{
   if (_encoder)
   {
      wgpuRenderPassEncoderRelease(_encoder);
   }
}

void RenderPassEncoder::SetPipeline(const RenderPipeline &pipeline) const
{
   wgpuRenderPassEncoderSetPipeline(_encoder, pipeline.Get());
}

void RenderPassEncoder::SetVertexBuffer(const uint32_t slot, const Buffer &buffer) const
{
   wgpuRenderPassEncoderSetVertexBuffer(_encoder, slot, buffer.Get(), 0, buffer.GetSize());
}

void RenderPassEncoder::SetIndexBuffer(const Buffer &buffer, WGPUIndexFormat format) const
{
   wgpuRenderPassEncoderSetIndexBuffer(_encoder, buffer.Get(), format, 0, buffer.GetSize());
}

void RenderPassEncoder::SetBindGroup(const uint32_t index, const BindGroup &bindGroup) const
{
   wgpuRenderPassEncoderSetBindGroup(_encoder, index, bindGroup.Get(), 0, nullptr);
}

void RenderPassEncoder::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) const
{
   wgpuRenderPassEncoderDraw(_encoder, vertexCount, instanceCount, firstVertex, firstInstance);
}
void RenderPassEncoder::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t baseVertex,
                                    const uint32_t firstInstance) const
{
   wgpuRenderPassEncoderDrawIndexed(_encoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void RenderPassEncoder::EndPass() const
{
   wgpuRenderPassEncoderEnd(_encoder);
}
