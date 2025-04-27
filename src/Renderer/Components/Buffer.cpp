#include <pch.h>

Buffer::Buffer(Device* device, WGPUBufferUsage usage, size_t size) :
   _size(size)
{
   WGPUBufferDescriptor bufferDesc = {};
   bufferDesc.size = _size;
   bufferDesc.usage = usage; // Ensure it's writable
   bufferDesc.mappedAtCreation = false;
   _buffer = wgpuDeviceCreateBuffer(device->Get(), &bufferDesc);
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
