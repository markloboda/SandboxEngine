#include <pch.h>

RenderPassEncoder::RenderPassEncoder(CommandEncoder* commandEncoder, WGPURenderPassDescriptor* descriptor) :
   _encoder(wgpuCommandEncoderBeginRenderPass(commandEncoder->Get(), descriptor))
{
   if (!_encoder)
   {
      throw std::runtime_error("Failed to begin render pass");
   }
}

RenderPassEncoder::~RenderPassEncoder()
{
   if (_encoder)
   {
      wgpuRenderPassEncoderRelease(_encoder);
   }
}

void RenderPassEncoder::SetPipeline(RenderPipeline* pipeline)
{
   wgpuRenderPassEncoderSetPipeline(_encoder, pipeline->Get());
}

void RenderPassEncoder::SetVertexBuffer(uint32_t slot, Buffer* buffer)
{
   wgpuRenderPassEncoderSetVertexBuffer(_encoder, slot, buffer->Get(), 0, buffer->GetSize());
}

void RenderPassEncoder::SetBindGroup(uint32_t index, BindGroup* bindGroup)
{
   wgpuRenderPassEncoderSetBindGroup(_encoder, index, *bindGroup->Get(), 0, 0);
}

void RenderPassEncoder::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
   wgpuRenderPassEncoderDraw(_encoder, vertexCount, instanceCount, firstVertex, firstInstance);
}

void RenderPassEncoder::EndPass()
{
   wgpuRenderPassEncoderEnd(_encoder);
}
