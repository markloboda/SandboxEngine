#include <pch.h>

Texture::Texture(Device* device, WGPUTextureDescriptor desc) :
   _texture(wgpuDeviceCreateTexture(device->Get(), &desc))
{}

Texture::~Texture()
{
   wgpuTextureRelease(_texture);
}

