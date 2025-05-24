#include <pch.h>

Buffer::Buffer(Device &device, const WGPUBufferUsage usage, const size_t size):
   _size(size),
   usage_(usage)
{
   WGPUBufferDescriptor bufferDesc = {};
   bufferDesc.size = _size;
   bufferDesc.usage = usage;
   bufferDesc.mappedAtCreation = false;
   _buffer = wgpuDeviceCreateBuffer(device.Get(), &bufferDesc);
   if (!_buffer)
   {
      std::cerr << "Failed to create WebGPU buffer";
   }
}

Buffer::~Buffer()
{
   if (_buffer)
   {
      wgpuBufferRelease(_buffer);
   }
}

void Buffer::MapAsync(const WGPUMapMode mode, const size_t offset, const size_t size, const WGPUBufferMapCallbackInfo &callbackInfo)
{
   _mapped = true;
   wgpuBufferMapAsync(_buffer, mode, offset, size, callbackInfo);
}

void Buffer::Unmap()
{
   wgpuBufferUnmap(_buffer);
   _mapped = false;
}
