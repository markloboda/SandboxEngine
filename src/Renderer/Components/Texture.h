#pragma once

class Texture
{
private:
   WGPUTexture _texture;

public:
   Texture(Device* device, WGPUTextureDescriptor* desc);
   ~Texture();

   WGPUTexture Get() const { return _texture; }
   bool IsValid() const { return _texture != nullptr; }

   // Copy with void wgpuQueueWriteTexture(WGPUQueue queue, WGPUTexelCopyTextureInfo const * destination, void const * data, size_t dataSize, WGPUTexelCopyBufferLayout const * dataLayout, WGPUExtent3D const * writeSize)
   void UploadData(Queue* queue, const void* data, size_t dataSize, const WGPUExtent3D* writeSize);
};
