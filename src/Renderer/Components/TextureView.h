#pragma once

class Device;

class TextureView
{
private:
   WGPUTextureView _textureView;

public:
   TextureView(WGPUTexture texture, WGPUTextureViewDescriptor* descriptor);
   ~TextureView();

   WGPUTextureView Get() const { return _textureView; }
};