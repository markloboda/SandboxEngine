#pragma once

class Profiler
{
private:
   QuerySet _gpuStatsQuerySet;
   Buffer _gpuStatsResolveBuffer;
   Buffer _gpuStatsResultBuffer;

   uint32_t timestampCount = 0;
   float *_timingResults = nullptr;
   bool *_usedTimestamps = nullptr;

public:
   explicit Profiler(Device &device, uint32_t profileCount);
   ~Profiler();

   void GetRenderPassTimestampWrites(uint32_t index, WGPURenderPassTimestampWrites &timestampWrites) const;
   void ResolveQuerySet(const CommandEncoder &encoder) const;
   void ReadResults();
   void ResetUsage() const;

   [[nodiscard]] float GetQueryData(uint32_t index) const;

   QuerySet &GetQuerySet() { return _gpuStatsQuerySet; }
};
