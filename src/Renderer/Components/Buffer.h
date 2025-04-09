#pragma once

class Device;

class Buffer
{
private:
   WGPUBuffer _buffer;
   size_t _size;

public:
   Buffer(Device* device, WGPUBufferUsage usage, size_t size);
   ~Buffer();

   WGPUBuffer Get() const { return _buffer; }
   size_t GetSize() const { return _size; }
};