#pragma once
#include <webgpu/webgpu.h>

struct BufferDesc
{
   WGPUBufferUsage usage;
};

class Buffer
{
public:
   Buffer(WGPUDevice device, BufferDesc desc, size_t size, const void* data);
   ~Buffer();

private:
   WGPUDevice _device;
   WGPUBuffer _buffer;
   size_t _size;

public:
   WGPUBuffer Get() const { return _buffer; }
   void UploadData(const void* data, size_t size);

private:
   void Init(WGPUBufferUsage usage);
};