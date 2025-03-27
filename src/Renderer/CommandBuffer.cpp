#include <VolumetricClouds.h>

CommandBuffer::CommandBuffer(WGPUCommandBuffer cmdBuffer) :
   _cmdBuffer(cmdBuffer)
{}

CommandBuffer::~CommandBuffer()
{
   if (_cmdBuffer)
   {
      wgpuCommandBufferRelease(_cmdBuffer);
   }
}
