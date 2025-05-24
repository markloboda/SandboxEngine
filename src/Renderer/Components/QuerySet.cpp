#include <pch.h>

QuerySet::QuerySet(const Device &device, const WGPUQueryType type, const uint32_t count)
   : _count(count)
{
   // Build the descriptor for the query set:
   WGPUQuerySetDescriptor desc = {};
   desc.type = type;
   desc.count = count;

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

void QuerySet::WriteTimestamp(const CommandEncoder &encoder, uint32_t queryIndex) const
{
   encoder.WriteTimestamp(*this, queryIndex);
}

void QuerySet::Resolve(const CommandEncoder &encoder, uint32_t queryCount, const Buffer &destination, uint64_t destinationOffset) const
{
   encoder.ResolveQuerySet(*this, 0, queryCount, destination, destinationOffset);
}
