// QuerySet.h
#pragma once

class QuerySet
{
private:
   WGPUQuerySet _querySet;
   uint32_t _count;

public:
   QuerySet(const Device &device, const WGPUQuerySetDescriptor &desc);
   ~QuerySet();

   [[nodiscard]] WGPUQuerySet Get() const { return _querySet; }
   [[nodiscard]] uint32_t GetCount() const { return _count; }

   void WriteTimestamp(const CommandEncoder &encoder, uint32_t queryIndex) const;
   void Resolve(const CommandEncoder &encoder, uint32_t queryCount, const Buffer &destination, uint64_t destinationOffset) const;
};
