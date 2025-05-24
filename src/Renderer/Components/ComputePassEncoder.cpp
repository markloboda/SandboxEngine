#include <pch.h>

ComputePassEncoder::ComputePassEncoder(const WGPUComputePassEncoder encoder):
   _encoder(encoder)
{
   if (!_encoder)
   {
      std::cerr << "Failed to begin compute pass";
   }
}

ComputePassEncoder::~ComputePassEncoder()
{
   if (_encoder)
   {
      wgpuComputePassEncoderRelease(_encoder);
   }
}

void ComputePassEncoder::SetPipeline(const ComputePipeline &pipeline) const
{
   wgpuComputePassEncoderSetPipeline(_encoder, pipeline.Get());
}

void ComputePassEncoder::SetBindGroup(const uint32_t index, const BindGroup &bindGroup) const
{
   wgpuComputePassEncoderSetBindGroup(_encoder, index, bindGroup.Get(), 0, nullptr);
}

void ComputePassEncoder::Dispatch(const uint32_t workgroupCountX, const uint32_t workgroupCountY, const uint32_t workgroupCountZ) const
{
   wgpuComputePassEncoderDispatchWorkgroups(_encoder, workgroupCountX, workgroupCountY, workgroupCountZ);
}

void ComputePassEncoder::EndPass() const
{
   wgpuComputePassEncoderEnd(_encoder);
}
