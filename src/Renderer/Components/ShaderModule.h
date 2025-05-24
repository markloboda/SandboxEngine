#pragma once

class ShaderModule
{
private:
   WGPUShaderModule _shaderModule;

private:
   explicit ShaderModule(WGPUShaderModule shaderModule);

public:
   ~ShaderModule();

   [[nodiscard]] WGPUShaderModule Get() const { return _shaderModule; }

   static std::string GetShaderPath(const std::string& shaderName);
   static ShaderModule &LoadShaderModule(const Device &device, const std::string &shaderName);

private:
   static ShaderModule &LoadSPIRVShaderModule(const Device &device, const std::string &shaderName);
};