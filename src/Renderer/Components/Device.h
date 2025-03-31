#pragma once

class Device
{
private:
   WGPUInstance _instance;
   WGPUAdapter _adapter;
   WGPUDevice _device;
   WGPUQueue _queue;

public:
   Device();
   ~Device();

   WGPUDevice Get() const { return _device; }
   WGPUQueue GetQueue() const { return _queue; }
   WGPUInstance GetInstance() const { return _instance; }
   WGPUShaderModule CreateShaderModule(const std::vector<uint32_t>& spirvCode) const;
   void Poll() const;

private:
};