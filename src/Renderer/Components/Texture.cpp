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

void Texture::UploadData(WGPUQueue queue, const void* data, size_t dataSize, const WGPUExtent3D* writeSize, int bytesPerRow)
{
   WGPUTexelCopyTextureInfo destination = {};
   destination.texture = _texture;
   destination.mipLevel = 0;
   destination.origin = { 0, 0, 0 };
   WGPUTexelCopyBufferLayout bufferLayout = {};
   bufferLayout.offset = 0;
   bufferLayout.bytesPerRow = bytesPerRow * writeSize->width;
   bufferLayout.rowsPerImage = writeSize->height;
   wgpuQueueWriteTexture(queue, &destination, data, dataSize, &bufferLayout, writeSize);
}
