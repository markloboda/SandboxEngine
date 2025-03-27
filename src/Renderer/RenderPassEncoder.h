#pragma once

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