#include <VolumetricClouds.h>
#include <Renderer/BindGroup.h>

BindGroup::BindGroup(WGPUDevice device, BindGroupDesc desc)
{
   // Create the BindGroupLayout
   WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
   bindGroupLayoutDesc.entryCount = static_cast<uint32_t>(desc.layoutEntries.size());
   bindGroupLayoutDesc.entries = desc.layoutEntries.data();
   _bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDesc);
   if (!_bindGroupLayout)
   {
      throw std::runtime_error("Failed to create WebGPU BindGroupLayout");
   }

   // Create the BindGroup using the layout
   WGPUBindGroupDescriptor bindGroupDesc = {};
   bindGroupDesc.layout = _bindGroupLayout;
   bindGroupDesc.entryCount = static_cast<uint32_t>(desc.entries.size());
   bindGroupDesc.entries = desc.entries.data();

   _bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDesc);
   if (!_bindGroup)
   {
      throw std::runtime_error("Failed to create WebGPU BindGroup");
   }
}

BindGroup::~BindGroup()
{
   if (_bindGroup)
   {
      wgpuBindGroupRelease(_bindGroup);
   }
   if (_bindGroupLayout)
   {
      wgpuBindGroupLayoutRelease(_bindGroupLayout);
   }
}