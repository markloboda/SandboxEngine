#pragma once

class Sampler
{
private:
   WGPUSampler _sampler;

public:
   Sampler(Device* device, WGPUSamplerDescriptor* desc);
   ~Sampler();

   WGPUSampler Get() const { return _sampler; }
};