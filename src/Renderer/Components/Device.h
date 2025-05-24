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

   [[nodiscard]] WGPUDevice Get() const { return _device; }
   [[nodiscard]] WGPUInstance GetInstance() const { return _instance; }

   [[nodiscard]] WGPUShaderModule CreateShaderModuleSpirV(const std::vector<uint32_t> &spirvCode) const;
   void Poll(bool wait = false) const;

private:
};