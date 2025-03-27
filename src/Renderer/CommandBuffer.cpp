#include <VolumetricClouds.h>
#include <Renderer/CommandBuffer.h>

#include <Renderer/CommandEncoder.h>

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
