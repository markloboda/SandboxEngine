#pragma once

class ComputePipeline;

class ComputePassEncoder
{
private:
   WGPUComputePassEncoder _encoder;

public:
   ComputePassEncoder(CommandEncoder* commandEncoder, WGPUComputePassDescriptor* descriptor);
   ~ComputePassEncoder();

   WGPUComputePassEncoder Get() const { return _encoder; }

   void SetPipeline(ComputePipeline* pipeline);
   void SetBindGroup(uint32_t index, BindGroup* bindGroup);
   void Dispatch(uint32_t workgroupCountX, uint32_t workgroupCountY, uint32_t workgroupCountZ);

   void EndPass();
};

