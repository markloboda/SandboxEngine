#pragma once

class ComputePipeline
{
private:
   WGPUComputePipeline _pipeline;

public:
   ComputePipeline(Device* device, WGPUComputePipelineDescriptor* desc);
   ~ComputePipeline();

   WGPUComputePipeline Get() const { return _pipeline; }
};

