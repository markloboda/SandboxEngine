#pragma once

class Device;

class TextureView
{
public:
   TextureView(WGPUTexture texture, WGPUTextureViewDescriptor* descriptor);
   ~TextureView();

private:
   WGPUTextureView _textureView;

public:
   WGPUTextureView Get() const { return _textureView; }
};