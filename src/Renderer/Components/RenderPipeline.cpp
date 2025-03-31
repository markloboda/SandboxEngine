#include <pch.h>

RenderPipeline::RenderPipeline(Device* device, WGPURenderPipelineDescriptor* desc)
{
   _renderPipeline = wgpuDeviceCreateRenderPipeline(device->Get(), desc);
}

RenderPipeline::~RenderPipeline()
{
   wgpuRenderPipelineRelease(_renderPipeline);
}
