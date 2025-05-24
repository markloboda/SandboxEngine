#include <pch.h>

BindGroup::BindGroup(const Device &device, const BindGroupDesc desc)
{
   assert(desc.layoutEntries.size() == desc.entries.size());

   // Create the BindGroupLayout
   {
      WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
      bindGroupLayoutDesc.entryCount = static_cast<uint32_t>(desc.layoutEntries.size());
      bindGroupLayoutDesc.entries = desc.layoutEntries.data();
      _bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device.Get(), &bindGroupLayoutDesc);
      if (!_bindGroupLayout)
      {
         std::cerr << ("Failed to create WebGPU BindGroupLayout");
      }
   }

   // Create the BindGroup using the layout
   {
      WGPUBindGroupDescriptor bindGroupDesc = {};
      bindGroupDesc.layout = _bindGroupLayout;
      bindGroupDesc.entryCount = static_cast<uint32_t>(desc.entries.size());
      bindGroupDesc.entries = desc.entries.data();
      _bindGroup = wgpuDeviceCreateBindGroup(device.Get(), &bindGroupDesc);
      if (!_bindGroup)
      {
         std::cerr << ("Failed to create WebGPU BindGroup");
      }
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
