#pragma once

class ComputePipeline
{
private:
   WGPUComputePipeline _pipeline;

public:
   ComputePipeline(const Device &device, const WGPUComputePipelineDescriptor *desc);
   ~ComputePipeline();

   [[nodiscard]] WGPUComputePipeline Get() const { return _pipeline; }
};

