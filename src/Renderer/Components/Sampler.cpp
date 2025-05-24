#include <pch.h>

Sampler::Sampler(const Device &device, const WGPUSamplerDescriptor *desc) : _sampler(wgpuDeviceCreateSampler(device.Get(), desc))
{
   if (!_sampler)
   {
      std::cerr << ("Failed to create sampler");
   }
}

Sampler::~Sampler()
{
   if (_sampler)
   {
      wgpuSamplerRelease(_sampler);
   }
}
