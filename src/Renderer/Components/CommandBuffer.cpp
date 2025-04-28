#include <pch.h>

CommandBuffer::CommandBuffer(WGPUCommandBuffer cmdBuffer) : _cmdBuffer(cmdBuffer)
{
   if (!_cmdBuffer)
   {
      std::cerr << ("Failed to create command buffer");
   }
}

CommandBuffer::~CommandBuffer()
{
   if (_cmdBuffer)
   {
      wgpuCommandBufferRelease(_cmdBuffer);
   }
}
