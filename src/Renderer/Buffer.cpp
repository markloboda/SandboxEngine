#include <VolumetricClouds.h>
#include <Renderer/Buffer.h>

Buffer::Buffer(WGPUDevice device, BufferDesc desc, size_t size, const void* data)
   : _device(device), _size(size)
{
   Init(desc.usage);
   if (data)
   {
      UploadData(data, _size);
   }
}

Buffer::~Buffer()
{
   if (_buffer)
   {
      wgpuBufferRelease(_buffer);
   }
}

void Buffer::UploadData(const void* data, size_t size)
{
   if (size > _size)
   {
      throw std::runtime_error("Data size exceeds buffer capacity");
   }

   WGPUQueue queue = wgpuDeviceGetQueue(_device);
   wgpuQueueWriteBuffer(queue, _buffer, 0, data, size);
}

void Buffer::Init(WGPUBufferUsage usage)
{
   WGPUBufferDescriptor bufferDesc = {};
   bufferDesc.size = _size;
   bufferDesc.usage = usage | WGPUBufferUsage_CopyDst; // Ensure it's writable
   bufferDesc.mappedAtCreation = false;
   _buffer = wgpuDeviceCreateBuffer(_device, &bufferDesc);
   if (!_buffer)
   {
      throw std::runtime_error("Failed to create WebGPU buffer");
   }
}
