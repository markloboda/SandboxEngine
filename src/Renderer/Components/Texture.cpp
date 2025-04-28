#include <pch.h>

Texture::Texture(Device *device, WGPUTextureDescriptor *desc) : _texture(wgpuDeviceCreateTexture(device->Get(), desc)),
                                                                _format(desc->format)
{
   if (_texture == nullptr)
   {
      std::cerr << ("Failed to create texture");
   }
}

Texture::~Texture()
{
   wgpuTextureRelease(_texture);
}
