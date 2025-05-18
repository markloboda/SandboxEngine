#include <pch.h>

QuerySet::QuerySet(Device *device, WGPUQueryType type, uint32_t count)
   : _count(count)
{
   // Build the descriptor for the query set:
   WGPUQuerySetDescriptor desc = {};
   desc.type = type;
   desc.count = count;

   // Create the WGPUQuerySet on the GPU device:
   _querySet = wgpuDeviceCreateQuerySet(device->Get(), &desc);
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
void QuerySet::WriteTimestamp(const CommandEncoder *encoder, const uint32_t queryIndex) const
{
   wgpuCommandEncoderWriteTimestamp(encoder->Get(), _querySet, queryIndex);
}
void QuerySet::Resolve(const CommandEncoder *encoder, Buffer *buffer, uint32_t bufferOffset) const
{
   
}
