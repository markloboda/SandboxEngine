#pragma once

class Profiler
{
private:
   QuerySet _gpuStatsQuerySet;
   Buffer _gpuStatsResolveBuffer;
   Buffer _gpuStatsResultBuffer;

   uint32_t _queryDataCount = 0;
   float *_queryData = nullptr;

public:
   explicit Profiler(Device &device, uint32_t queryCount);
   ~Profiler();

   void SetupTimestamps(WGPURenderPassDescriptor &desc, const uint32_t index) const
   {
      const uint32_t idx = index * 2;
      desc.timestampWrites = new WGPURenderPassTimestampWrites(_gpuStatsQuerySet.Get(), idx, idx + 1);
   }

   void ResolveQuerySet(const CommandEncoder &encoder) const;
   void ReadResults();

   [[nodiscard]] float GetQueryData(const uint32_t index) const
   {
      if (index >= _queryDataCount / 2)
      {
         throw std::out_of_range("Index out of range in Profiler::GetQueryData");
      }
      return _queryData[index / 2];
   }
};
