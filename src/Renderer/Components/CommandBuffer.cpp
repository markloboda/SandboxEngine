#include <pch.h>

CommandBuffer::CommandBuffer(const CommandEncoder &cmdEncoder):
   _cmdBuffer(cmdEncoder.Finish())
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
