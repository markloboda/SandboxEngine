#pragma once
#include <webgpu/webgpu.h>

struct BindGroupDesc
{
   const std::vector<WGPUBindGroupLayoutEntry>& layoutEntries;
   const std::vector<WGPUBindGroupEntry>& entries;
};

class BindGroup
{
public:
   BindGroup(WGPUDevice device, BindGroupDesc desc);
   ~BindGroup();

private:
   WGPUBindGroupLayout _bindGroupLayout;
   WGPUBindGroup _bindGroup;

public:
   WGPUBindGroup Get() const { return _bindGroup; }
   WGPUBindGroupLayout GetLayout() const { return _bindGroupLayout; }
};