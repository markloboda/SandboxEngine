#include <pch.h>

RenderPipeline::RenderPipeline(const Device &device, const WGPURenderPipelineDescriptor *desc) : _renderPipeline(wgpuDeviceCreateRenderPipeline(device.Get(), desc))
{
   if (!_renderPipeline)
   {
      std::cerr << ("Failed to create render pipeline");
   }
}

RenderPipeline::~RenderPipeline()
{
   if (_renderPipeline)
   {
      wgpuRenderPipelineRelease(_renderPipeline);
   }
}
