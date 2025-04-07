#include <pch.h>

ComputePipeline::ComputePipeline(Device* device, WGPUComputePipelineDescriptor* desc) :
   _pipeline(wgpuDeviceCreateComputePipeline(device->Get(), desc))
{
   if (!_pipeline)
   {
      throw std::runtime_error("Failed to create compute pipeline");
   }
}

ComputePipeline::~ComputePipeline()
{
   if (_pipeline)
   {
      wgpuComputePipelineRelease(_pipeline);
   }
}
