#include <pch.h>

TextureView::TextureView(WGPUTexture texture, WGPUTextureViewDescriptor *descriptor) : _textureView(wgpuTextureCreateView(texture, descriptor))
{
   if (!_textureView)
   {
      std::cerr << ("Failed to create texture view");
   }
}

TextureView::~TextureView()
{
   if (_textureView)
   {
      wgpuTextureViewRelease(_textureView);
   }
}
