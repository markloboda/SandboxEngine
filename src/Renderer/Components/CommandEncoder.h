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
   CommandEncoder(Device* device, WGPUCommandEncoderDescriptor* descriptor);
   ~CommandEncoder();

   WGPUCommandEncoder Get() const { return _encoder; }

   WGPUComputePassEncoder BeginComputePass(WGPUComputePassDescriptor* descriptor);
   WGPURenderPassEncoder BeginRenderPass(WGPURenderPassDescriptor* descriptor);
   WGPUCommandBuffer Finish();

   void CopyTextureToBuffer(WGPUTexelCopyTextureInfo const* source, WGPUTexelCopyBufferInfo const* destination, WGPUExtent3D const* copySize);

};