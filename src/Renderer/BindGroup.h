#pragma once

struct BindGroupDesc
{
   const std::vector<WGPUBindGroupLayoutEntry>& layoutEntries;
   const std::vector<WGPUBindGroupEntry>& entries;
};

class Device;

class BindGroup
{
public:
   BindGroup(Device* device, BindGroupDesc desc);
   ~BindGroup();

private:
   WGPUBindGroupLayout _bindGroupLayout;
   WGPUBindGroup _bindGroup;

public:
   WGPUBindGroup const* Get() const { return &_bindGroup; }
   WGPUBindGroupLayout const* GetLayout() const { return &_bindGroupLayout; }
};