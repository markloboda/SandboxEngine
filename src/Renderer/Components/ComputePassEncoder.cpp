#include <pch.h>

ComputePassEncoder::ComputePassEncoder(WGPUComputePassEncoder encoder) : _encoder(encoder)
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

void ComputePassEncoder::SetPipeline(ComputePipeline *pipeline)
{
   wgpuComputePassEncoderSetPipeline(_encoder, pipeline->Get());
}

void ComputePassEncoder::SetBindGroup(uint32_t index, BindGroup *bindGroup)
{
   wgpuComputePassEncoderSetBindGroup(_encoder, index, *bindGroup->Get(), 0, 0);
}

void ComputePassEncoder::Dispatch(uint32_t workgroupCountX, uint32_t workgroupCountY, uint32_t workgroupCountZ)
{
   wgpuComputePassEncoderDispatchWorkgroups(_encoder, workgroupCountX, workgroupCountY, workgroupCountZ);
}

void ComputePassEncoder::EndPass()
{
   wgpuComputePassEncoderEnd(_encoder);
}
