#pragma once

class CommandEncoder;

class CommandBuffer
{
public:
   CommandBuffer() = default;
   CommandBuffer(WGPUCommandBuffer cmdBuffer);
   ~CommandBuffer();

private:
   WGPUCommandBuffer _cmdBuffer;

public:
   WGPUCommandBuffer Get() const { return _cmdBuffer; }
};