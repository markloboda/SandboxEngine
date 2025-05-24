#include <pch.h>

ComputePipeline::ComputePipeline(const Device &device, const WGPUComputePipelineDescriptor *desc) : _pipeline(wgpuDeviceCreateComputePipeline(device.Get(), desc))
{
   if (!_pipeline)
   {
      std::cerr << ("Failed to create compute pipeline");
   }
}

ComputePipeline::~ComputePipeline()
{
   if (_pipeline)
   {
      wgpuComputePipelineRelease(_pipeline);
   }
}
