#pragma once

class Device;

class Buffer
{
private:
   WGPUBuffer _buffer;
   size_t _size;

public:
   Buffer(Device* device, WGPUBufferUsage usage, size_t size, const void* data);
   ~Buffer();

   WGPUBuffer Get() const { return _buffer; }
   size_t GetSize() const { return _size; }
   void UploadData(Device* device, const void* data, size_t size);

private:
};