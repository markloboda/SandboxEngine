#pragma once

class ComputePassEncoder
{
private:
   WGPUComputePassEncoder _encoder;

public:
   explicit ComputePassEncoder(WGPUComputePassEncoder encoder);
   ~ComputePassEncoder();

   [[nodiscard]] WGPUComputePassEncoder Get() const { return _encoder; }

   void SetPipeline(const ComputePipeline &pipeline) const;
   void SetBindGroup(uint32_t index, const BindGroup &bindGroup) const;
   void Dispatch(uint32_t workgroupCountX, uint32_t workgroupCountY, uint32_t workgroupCountZ) const;

   void EndPass() const;
};

