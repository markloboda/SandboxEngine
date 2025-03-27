#include <VolumetricClouds.h>

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

void RenderPassEncoder::EndPass()
{
   wgpuRenderPassEncoderEnd(_encoder);
}
