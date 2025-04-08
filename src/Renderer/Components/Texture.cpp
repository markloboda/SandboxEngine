#include <pch.h>

Texture::Texture(Device* device, WGPUTextureDescriptor* desc) :
   _texture(wgpuDeviceCreateTexture(device->Get(), desc)),
   _format(desc->format)
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

void Texture::UploadData(Queue* queue, const void* data, size_t dataSize, const WGPUExtent3D* writeSize)
{
   WGPUTexelCopyTextureInfo destination = {};
   destination.texture = _texture;
   destination.mipLevel = 0;
   destination.origin = { 0, 0, 0 };
   WGPUTexelCopyBufferLayout bufferLayout = {};
   bufferLayout.offset = 0;
   bufferLayout.bytesPerRow = dataSize / static_cast<size_t>(writeSize->height * writeSize->depthOrArrayLayers);
   bufferLayout.rowsPerImage = writeSize->height;
   wgpuQueueWriteTexture(queue->Get(), &destination, data, dataSize, &bufferLayout, writeSize);
}
