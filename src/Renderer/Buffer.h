#pragma once
#include <webgpu/webgpu.h>

struct BufferDesc
{
   WGPUBufferUsage usage;
};

class Device;

class Buffer
{
public:
   Buffer(Device* device, BufferDesc desc, size_t size, const void* data);
   ~Buffer();

private:
   WGPUBuffer _buffer;
   size_t _size;

public:
   WGPUBuffer Get() const { return _buffer; }
   void UploadData(Device* device, const void* data, size_t size);

private:
};