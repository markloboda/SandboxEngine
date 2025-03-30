#pragma once

class
   Device;
class RenderPassEncoder;
class CommandBuffer;

class CommandEncoder
{
public:
   CommandEncoder(Device* device, WGPUCommandEncoderDescriptor* descriptor);
   ~CommandEncoder();

private:
   WGPUCommandEncoder _encoder;

public:
   WGPUCommandEncoder Get() const { return _encoder; }

   WGPUCommandBuffer Finish();
};