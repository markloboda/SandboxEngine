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
      throw std::runtime_error("Failed to create WebGPU buffer");
   }
}

Buffer::~Buffer()
{
   if (_buffer)
   {
      wgpuBufferRelease(_buffer);
   }
}

void Buffer::UploadData(Device* device, const void* data, size_t size)
{
   if (size > _size)
   {
      throw std::runtime_error("Data size exceeds buffer capacity");
   }

   WGPUQueue queue = wgpuDeviceGetQueue(device->Get());
   wgpuQueueWriteBuffer(queue, _buffer, 0, data, size);
}
