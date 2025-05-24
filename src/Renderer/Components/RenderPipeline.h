#pragma once

class RenderPipeline
{
private:
   WGPURenderPipeline _renderPipeline;

public:
   RenderPipeline(const Device &device, const WGPURenderPipelineDescriptor *desc);
   ~RenderPipeline();

   [[nodiscard]] WGPURenderPipeline Get() const { return _renderPipeline; }
};

