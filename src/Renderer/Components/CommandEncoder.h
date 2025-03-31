#pragma once

class
   Device;
class RenderPassEncoder;
class CommandBuffer;

class CommandEncoder
{
private:
   WGPUCommandEncoder _encoder;

public:
   CommandEncoder(Device* device, WGPUCommandEncoderDescriptor* descriptor);
   ~CommandEncoder();

   WGPUCommandEncoder Get() const { return _encoder; }

   CommandBuffer* Finish();
};