#pragma once

class Sampler
{
private:
   WGPUSampler _sampler;

public:
   Sampler(const Device &device, const WGPUSamplerDescriptor *desc);
   ~Sampler();

   [[nodiscard]] WGPUSampler Get() const { return _sampler; }
};