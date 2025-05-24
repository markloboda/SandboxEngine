#include <pch.h>

ShaderModule::ShaderModule(WGPUShaderModule shaderModule):
   _shaderModule(shaderModule)
{
   if (_shaderModule == nullptr)
   {
      std::cerr << ("Failed to create shader module");
   }
}

ShaderModule::~ShaderModule()
{
   if (_shaderModule)
   {
      wgpuShaderModuleRelease(_shaderModule);
   }
}


std::string inline ShaderModule::GetShaderPath(const std::string &shaderName)
{
   return "shaders/" + shaderName;
}

ShaderModule &ShaderModule::LoadShaderModule(const Device &device, const std::string &shaderName)
{
   if (shaderName.ends_with(".frag") || shaderName.ends_with(".vert") || shaderName.ends_with(".spv"))
   {
      return LoadSPIRVShaderModule(device, shaderName);
   }

   std::cerr << ("Unsupported shader file extension: " + shaderName);
   return *new ShaderModule(nullptr);
}

ShaderModule &ShaderModule::LoadSPIRVShaderModule(const Device &device, const std::string &shaderName)
{
   std::string path = GetShaderPath(shaderName);

   // Open file
   std::ifstream file(path, std::ios::binary | std::ios::ate);
   if (!file.is_open())
   {
      std::cerr << ("Failed to open shader file: " + path);
   }

   // Read file
   size_t fileSize = file.tellg();
   file.seekg(0);
   std::vector<uint32_t> spirv(fileSize / sizeof(uint32_t));
   file.read(reinterpret_cast<char *>(spirv.data()), fileSize);
   file.close();

   // Create shader module
   WGPUShaderModuleDescriptorSpirV descriptor = {};
   descriptor.sourceSize = static_cast<uint32_t>(spirv.size());
   descriptor.source = spirv.data();

   WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModuleSpirV(device.Get(), &descriptor);
   if (!shaderModule)
   {
      std::cerr << ("Failed to create shader module: " + path);
   }

   return *new ShaderModule(shaderModule);
}
