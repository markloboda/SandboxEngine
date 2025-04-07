#include <pch.h>

CommandEncoder::CommandEncoder(Device* device, WGPUCommandEncoderDescriptor* descriptor) :
   _encoder(wgpuDeviceCreateCommandEncoder(device->Get(), descriptor))
{
   if (!_encoder)
   {
      throw std::runtime_error("Failed to create command encoder");
   }
}

CommandEncoder::~CommandEncoder()
{
   if (_encoder)
   {
      wgpuCommandEncoderRelease(_encoder);
   }
}

CommandBuffer* CommandEncoder::Finish()
{
   return new CommandBuffer(wgpuCommandEncoderFinish(_encoder, nullptr));
}

void CommandEncoder::CopyTextureToBuffer(WGPUTexelCopyTextureInfo const* source, WGPUTexelCopyBufferInfo const* destination, WGPUExtent3D const* copySize)
{
   wgpuCommandEncoderCopyTextureToBuffer(_encoder, source, destination, copySize);
}

ComputePassEncoder* CommandEncoder::BeginComputePass(WGPUComputePassDescriptor* descriptor)
{
   return new ComputePassEncoder(this, descriptor);
}

RenderPassEncoder* CommandEncoder::BeginRenderPass(WGPURenderPassDescriptor* descriptor)
{
   return new RenderPassEncoder(this, descriptor);
}
