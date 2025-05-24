#pragma once

class CommandEncoder;

class CommandBuffer
{
private:
   WGPUCommandBuffer _cmdBuffer;

public:
   CommandBuffer() = default;
   explicit CommandBuffer(const CommandEncoder &cmdEncoder);
   ~CommandBuffer();

   [[nodiscard]] WGPUCommandBuffer Get() const { return _cmdBuffer; }
};