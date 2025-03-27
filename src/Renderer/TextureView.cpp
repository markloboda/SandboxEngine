#include <VolumetricClouds.h>
#include <Renderer/TextureView.h>

TextureView::TextureView(WGPUTexture texture, WGPUTextureViewDescriptor* descriptor) :
   _textureView(wgpuTextureCreateView(texture, descriptor))
{
   if (!_textureView)
   {
      throw std::runtime_error("Failed to create texture view");
   }
}

TextureView::~TextureView()
{
   if (_textureView)
   {
      wgpuTextureViewRelease(_textureView);
   }
}
