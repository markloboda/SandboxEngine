#include <pch.h>

QuerySet::QuerySet(const Device &device, const WGPUQuerySetDescriptor &desc)
   : _count(desc.count)
{
   // Create the WGPUQuerySet on the GPU device:
   _querySet = wgpuDeviceCreateQuerySet(device.Get(), &desc);
   assert(_querySet && "Failed to create WGPUQuerySet");
}

QuerySet::~QuerySet()
{
   if (_querySet)
   {
      wgpuQuerySetRelease(_querySet);
      _querySet = nullptr;
   }
}
