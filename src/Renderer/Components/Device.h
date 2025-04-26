#pragma once

class Device
{
private:
   WGPUInstance _instance;
   WGPUAdapter _adapter;
   WGPUDevice _device;

public:
   Device();
   ~Device();

   WGPUDevice Get() const { return _device; }
   WGPUInstance GetInstance() const { return _instance; }
   WGPUShaderModule CreateShaderModuleSpirV(const std::vector<uint32_t>& spirvCode) const;
   void Poll() const;

private:
};