#include <pch.h>

RenderPipeline::RenderPipeline(Device* device, WGPURenderPipelineDescriptor* desc) :
   _renderPipeline(wgpuDeviceCreateRenderPipeline(device->Get(), desc))
{
   if (!_renderPipeline)
   {
      throw std::runtime_error("Failed to create render pipeline");
   }
}

RenderPipeline::~RenderPipeline()
{
   if (_renderPipeline)
   {
      wgpuRenderPipelineRelease(_renderPipeline);
   }
}
