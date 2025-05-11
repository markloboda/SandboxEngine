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

void RenderPassEncoder::SetPipeline(RenderPipeline *pipeline)
{
   wgpuRenderPassEncoderSetPipeline(_encoder, pipeline->Get());
}

void RenderPassEncoder::SetVertexBuffer(uint32_t slot, Buffer *buffer)
{
   wgpuRenderPassEncoderSetVertexBuffer(_encoder, slot, buffer->Get(), 0, buffer->GetSize());
}

void RenderPassEncoder::SetIndexBuffer(Buffer *buffer, WGPUIndexFormat format)
{
   wgpuRenderPassEncoderSetIndexBuffer(_encoder, buffer->Get(), format, 0, buffer->GetSize());
}

void RenderPassEncoder::SetBindGroup(uint32_t index, BindGroup *bindGroup)
{
   wgpuRenderPassEncoderSetBindGroup(_encoder, index, *bindGroup->Get(), 0, 0);
}

void RenderPassEncoder::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
   wgpuRenderPassEncoderDraw(_encoder, vertexCount, instanceCount, firstVertex, firstInstance);
}
void RenderPassEncoder::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
{
   wgpuRenderPassEncoderDrawIndexed(_encoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void RenderPassEncoder::EndPass()
{
   wgpuRenderPassEncoderEnd(_encoder);
}
