#pragma once

class ShaderModule
{
private:
   WGPUShaderModule _shaderModule;

private:
   ShaderModule(WGPUShaderModule shaderModule);

public:
   ~ShaderModule();

   WGPUShaderModule Get() const { return _shaderModule; }

   static std::string GetShaderPath(const std::string& shaderName);
   static ShaderModule& LoadShaderModule(Device* device, const std::string& shaderName);

private:
   static ShaderModule& LoadSPIRVShaderModule(Device* device, const std::string& shaderName);
};