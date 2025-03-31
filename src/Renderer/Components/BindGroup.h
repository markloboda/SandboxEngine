#pragma once

struct BindGroupDesc
{
   const std::vector<WGPUBindGroupLayoutEntry>& layoutEntries;
   const std::vector<WGPUBindGroupEntry>& entries;
};

class Device;

class BindGroup
{
private:
   WGPUBindGroupLayout _bindGroupLayout;
   WGPUBindGroup _bindGroup;

public:
   BindGroup(Device* device, BindGroupDesc desc);
   ~BindGroup();

   WGPUBindGroup const* Get() const { return &_bindGroup; }
   WGPUBindGroupLayout const* GetLayout() const { return &_bindGroupLayout; }
};