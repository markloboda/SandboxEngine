#include <pch.h>

CommandEncoder::CommandEncoder(const Device &device, const WGPUCommandEncoderDescriptor *descriptor):
   _encoder(wgpuDeviceCreateCommandEncoder(device.Get(), descriptor))
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

WGPUComputePassEncoder CommandEncoder::BeginComputePass(const WGPUComputePassDescriptor *descriptor) const
{
   return wgpuCommandEncoderBeginComputePass(Get(), descriptor);
}

WGPURenderPassEncoder CommandEncoder::BeginRenderPass(const WGPURenderPassDescriptor *descriptor) const
{
   return wgpuCommandEncoderBeginRenderPass(Get(), descriptor);
}

WGPUCommandBuffer CommandEncoder::Finish() const
{
   WGPUCommandBufferDescriptor desc = {};
   desc.label = WGPUStringView{"My Command Buffer", WGPU_STRLEN};
   return wgpuCommandEncoderFinish(_encoder, &desc);
}

void CommandEncoder::CopyTextureToBuffer(WGPUTexelCopyTextureInfo const *source, WGPUTexelCopyBufferInfo const *destination, WGPUExtent3D const *copySize) const
{
   wgpuCommandEncoderCopyTextureToBuffer(_encoder, source, destination, copySize);
}

void CommandEncoder::CopyBufferToBuffer(const Buffer &source, uint64_t sourceOffset, const Buffer &destination, uint64_t destinationOffset, uint64_t size) const
{
   wgpuCommandEncoderCopyBufferToBuffer(_encoder, source.Get(), sourceOffset, destination.Get(), destinationOffset, size);
}

void CommandEncoder::ClearBuffer(const Buffer &buffer, uint64_t offset, uint64_t size) const
{
   wgpuCommandEncoderClearBuffer(_encoder, buffer.Get(), offset, size);
}

void CommandEncoder::WriteTimestamp(const QuerySet &querySet, uint32_t queryIndex) const
{
   wgpuCommandEncoderWriteTimestamp(_encoder, querySet.Get(), queryIndex);
}

void CommandEncoder::ResolveQuerySet(const QuerySet &querySet, uint32_t firstQuery, uint32_t queryCount, const Buffer &destination, uint64_t destinationOffset) const
{
   wgpuCommandEncoderResolveQuerySet(_encoder, querySet.Get(), firstQuery, queryCount, destination.Get(), destinationOffset);
}
