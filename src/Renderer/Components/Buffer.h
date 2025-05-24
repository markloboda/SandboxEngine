#pragma once

class Device;

class Buffer
{
private:
    WGPUBuffer _buffer;
    size_t _size;
    bool _mapped = false;
    WGPUBufferUsage usage_;

public:
    Buffer(const Device &device, WGPUBufferUsage usage, size_t size);
    Buffer(Device &device, WGPUBufferUsage usage, size_t size);
    ~Buffer();

    void MapAsync(WGPUMapMode mode, size_t offset, size_t size, const WGPUBufferMapCallbackInfo &callbackInfo);
    void Unmap();

    [[nodiscard]] WGPUBuffer Get() const { return _buffer; }
    [[nodiscard]] size_t GetSize() const { return _size; }
    [[nodiscard]] bool IsMapped() const { return _mapped; }
    [[nodiscard]] void *GetMappedRange(const size_t offset = 0, const size_t size = 0) const { return wgpuBufferGetMappedRange(_buffer, offset, size); }
};