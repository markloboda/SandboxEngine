#include <pch.h>

Sampler::Sampler(Device* device, WGPUSamplerDescriptor* desc) :
   _sampler(wgpuDeviceCreateSampler(device->Get(), desc))
{
   if (!_sampler)
   {
      throw std::runtime_error("Failed to create sampler");
   }
}

Sampler::~Sampler()
{
   if (_sampler)
   {
      wgpuSamplerRelease(_sampler);
   }
}
