#pragma once
#include <webgpu/webgpu.h>

class Device
{
public:
   Device();
   ~Device();

private:
   WGPUInstance _instance;
   WGPUAdapter _adapter;
   WGPUDevice _device;
   WGPUQueue _queue;

public:
   WGPUDevice Get() const { return _device; }
   WGPUQueue GetQueue() const { return _queue; }
   WGPUInstance GetInstance() const { return _instance; }
   WGPUShaderModule CreateShaderModule(const std::vector<uint32_t>& spirvCode);

private:
   void Init();
};