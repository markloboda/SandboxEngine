#pragma once

class CommandEncoder;

class CommandBuffer
{
private:
   WGPUCommandBuffer _cmdBuffer;

public:
   CommandBuffer() = default;
   CommandBuffer(WGPUCommandBuffer cmdBuffer);
   ~CommandBuffer();

   WGPUCommandBuffer Get() const { return _cmdBuffer; }
};