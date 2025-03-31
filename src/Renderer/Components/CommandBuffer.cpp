#include <pch.h>

CommandBuffer::CommandBuffer(WGPUCommandBuffer cmdBuffer) :
   _cmdBuffer(cmdBuffer)
{
   if (!_cmdBuffer)
   {
      throw std::runtime_error("Failed to create command buffer");
   }
}

CommandBuffer::~CommandBuffer()
{
   if (_cmdBuffer)
   {
      wgpuCommandBufferRelease(_cmdBuffer);
   }
}
