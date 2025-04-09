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

WGPUComputePassEncoder CommandEncoder::BeginComputePass(WGPUComputePassDescriptor* descriptor)
{
   return wgpuCommandEncoderBeginComputePass(Get(), descriptor);
}

WGPURenderPassEncoder CommandEncoder::BeginRenderPass(WGPURenderPassDescriptor* descriptor)
{
   return wgpuCommandEncoderBeginRenderPass(Get(), descriptor);
}

WGPUCommandBuffer CommandEncoder::Finish()
{
   WGPUCommandBufferDescriptor desc = {};
   std::string label = "My Command Buffer";
   desc.label = WGPUStringView{ label.c_str(), label.size() };
   return wgpuCommandEncoderFinish(_encoder, &desc);
}

void CommandEncoder::CopyTextureToBuffer(WGPUTexelCopyTextureInfo const* source, WGPUTexelCopyBufferInfo const* destination, WGPUExtent3D const* copySize)
{
   wgpuCommandEncoderCopyTextureToBuffer(_encoder, source, destination, copySize);
}
