#include <pch.h>

Texture::Texture(Device* device, WGPUTextureDescriptor* desc) :
   _texture(wgpuDeviceCreateTexture(device->Get(), desc))
{
   if (_texture == nullptr)
   {
      throw std::runtime_error("Failed to create texture");
   }
}

Texture::~Texture()
{
   wgpuTextureRelease(_texture);
}

