#pragma once

struct BindGroupDesc
{
   const std::vector<WGPUBindGroupLayoutEntry>& layoutEntries;
   const std::vector<WGPUBindGroupEntry>& entries;
};

class BindGroup
{
private:
   WGPUBindGroupLayout _bindGroupLayout;
   WGPUBindGroup _bindGroup;

public:
   BindGroup(const Device &device, BindGroupDesc desc);
   ~BindGroup();

   [[nodiscard]] WGPUBindGroup const &Get() const { return _bindGroup; }
   [[nodiscard]] WGPUBindGroupLayout const &GetLayout() const { return _bindGroupLayout; }
};