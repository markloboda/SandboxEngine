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

   ComputePassEncoder* BeginComputePass(WGPUComputePassDescriptor* descriptor);
   RenderPassEncoder* BeginRenderPass(WGPURenderPassDescriptor* descriptor);
   CommandBuffer* Finish();

   void CopyTextureToBuffer(WGPUTexelCopyTextureInfo const* source, WGPUTexelCopyBufferInfo const* destination, WGPUExtent3D const* copySize);

};