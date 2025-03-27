#pragma once
#include <webgpu/webgpu.h>

class CommandEncoder;
class TextureView;

class RenderPassEncoder
{
public:
   RenderPassEncoder(CommandEncoder* commandEncoder, WGPURenderPassDescriptor* descriptor);
   ~RenderPassEncoder();

private:
   WGPURenderPassEncoder _encoder;

public:
   WGPURenderPassEncoder Get() const { return _encoder; }

   void EndPass();
};