#pragma once

class Device;
class RenderPassEncoder;
class CommandBuffer;
class ComputePassEncoder;

class CommandEncoder
{
private:
   WGPUCommandEncoder _encoder;

public:
   CommandEncoder(const Device &device, const WGPUCommandEncoderDescriptor *descriptor);
   ~CommandEncoder();

   WGPUCommandEncoder Get() const { return _encoder; }

   WGPUComputePassEncoder BeginComputePass(const WGPUComputePassDescriptor *descriptor) const;
   WGPURenderPassEncoder BeginRenderPass(const WGPURenderPassDescriptor *descriptor) const;
   WGPUCommandBuffer Finish() const;

   void CopyTextureToBuffer(WGPUTexelCopyTextureInfo const *source, WGPUTexelCopyBufferInfo const *destination, WGPUExtent3D const *copySize) const;
   void CopyBufferToBuffer(const Buffer &source, uint64_t sourceOffset, const Buffer &destination, uint64_t destinationOffset, uint64_t size) const;

   void WriteTimestamp(const QuerySet &querySet, uint32_t queryIndex) const;
   void ResolveQuerySet(const QuerySet &querySet, uint32_t firstQuery, uint32_t queryCount, const Buffer &destination, uint64_t destinationOffset) const;
};