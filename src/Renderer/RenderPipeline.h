#pragma once

class RenderPipeline
{
private:
   WGPURenderPipeline _renderPipeline;

public:
   RenderPipeline(Device* device, WGPURenderPipelineDescriptor* desc);
   ~RenderPipeline();

   WGPURenderPipeline Get() const { return _renderPipeline; }
};

