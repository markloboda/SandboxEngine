#include <pch.h>

CommandEncoder::CommandEncoder(Device *device, WGPUCommandEncoderDescriptor *descriptor) : _encoder(wgpuDeviceCreateCommandEncoder(device->Get(), descriptor))
{
   if (!_encoder)
   {
      std::cerr << ("Failed to create command encoder");
   }
}

CommandEncoder::~CommandEncoder()
{
   if (_encoder)
   {
      wgpuCommandEncoderRelease(_encoder);
   }
}

WGPUComputePassEncoder CommandEncoder::BeginComputePass(WGPUComputePassDescriptor *descriptor)
{
   return wgpuCommandEncoderBeginComputePass(Get(), descriptor);
}

WGPURenderPassEncoder CommandEncoder::BeginRenderPass(WGPURenderPassDescriptor *descriptor)
{
   return wgpuCommandEncoderBeginRenderPass(Get(), descriptor);
}

WGPUCommandBuffer CommandEncoder::Finish()
{
   WGPUCommandBufferDescriptor desc = {};
   desc.label = WGPUStringView{"My Command Buffer", WGPU_STRLEN};
   return wgpuCommandEncoderFinish(_encoder, &desc);
}

void CommandEncoder::CopyTextureToBuffer(WGPUTexelCopyTextureInfo const *source, WGPUTexelCopyBufferInfo const *destination, WGPUExtent3D const *copySize)
{
   wgpuCommandEncoderCopyTextureToBuffer(_encoder, source, destination, copySize);
}
