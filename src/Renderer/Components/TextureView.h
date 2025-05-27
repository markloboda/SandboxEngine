#pragma once

class TextureView
{
private:
   WGPUTextureView _textureView;

public:
   TextureView(WGPUTexture texture, const WGPUTextureViewDescriptor* descriptor);
   ~TextureView();

   [[nodiscard]] WGPUTextureView Get() const { return _textureView; }
};