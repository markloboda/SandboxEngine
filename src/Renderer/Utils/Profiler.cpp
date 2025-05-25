#include <pch.h>
#include <Renderer/Utils/Profiler.h>

Profiler::Profiler(Device &device, const uint32_t profileCount):
   _gpuStatsQuerySet(device, WGPUQuerySetDescriptor{
                        .label = WGPUStringView{"GPU Stats Query Set", WGPU_STRLEN},
                        .type = WGPUQueryType_Timestamp,
                        .count = profileCount * 2, // Two timestamps per profile
                     }),
   _gpuStatsResolveBuffer(device, WGPUBufferUsage_QueryResolve | WGPUBufferUsage_CopySrc, _gpuStatsQuerySet.GetCount() * sizeof(uint64_t)),
   _gpuStatsResultBuffer(device, WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead, _gpuStatsResolveBuffer.GetSize()),
   timestampCount(profileCount * 2)
{
   _timingResults = new float[profileCount];
   _usedTimestamps = new bool[timestampCount];
   ResetUsage();
}

Profiler::~Profiler()
{
   delete[] _usedTimestamps;
   delete[] _timingResults;
}

void Profiler::GetRenderPassTimestampWrites(uint32_t index, WGPURenderPassTimestampWrites &timestampWrites) const
{
   const uint32_t begin = index * 2;
   const uint32_t end = begin + 1;

   _usedTimestamps[begin] = true;
   _usedTimestamps[end] = true;

   timestampWrites.querySet = _gpuStatsQuerySet.Get();
   timestampWrites.beginningOfPassWriteIndex = begin;
   timestampWrites.endOfPassWriteIndex = end;
}

void Profiler::ResolveQuerySet(const CommandEncoder &encoder) const
{
   for (uint32_t i = 0; i < timestampCount;)
   {
      // Skip unused indices
      while (i < timestampCount && !_usedTimestamps[i])
         ++i;

      if (i >= timestampCount)
         break;

      // Start of a contiguous used range
      uint32_t start = i;
      while (i < timestampCount && _usedTimestamps[i])
         ++i;

      uint32_t count = i - start;
      uint64_t bufferOffset = start * sizeof(uint64_t);
      bufferOffset = (bufferOffset / 256) * 256; // Align to 256 bytes

      encoder.ResolveQuerySet(_gpuStatsQuerySet, start, count, _gpuStatsResolveBuffer, bufferOffset);
   }

   if (!_gpuStatsResultBuffer.IsMapped())
   {
      encoder.CopyBufferToBuffer(_gpuStatsResolveBuffer, 0, _gpuStatsResultBuffer, 0, _gpuStatsResolveBuffer.GetSize());
   }
}

void Profiler::ReadResults()
{
   if (!_gpuStatsResultBuffer.IsMapped())
   {
      WGPUBufferMapCallbackInfo mapCallback = {};
      mapCallback.userdata1 = this;
      mapCallback.callback = [](WGPUMapAsyncStatus status, WGPUStringView message, void *userdata, void *)
      {
         if (status != WGPUMapAsyncStatus_Success)
         {
            std::cerr << "Failed to map stats result buffer." << message.data << "\n";
            return;
         }
         Profiler *profiler = static_cast<Profiler *>(userdata);
         uint64_t *data = static_cast<uint64_t *>(profiler->_gpuStatsResultBuffer.GetMappedRange());

         for (uint32_t i = 0; i < profiler->timestampCount; i += 2)
         {
            profiler->_timingResults[i / 2] = static_cast<float>(data[i + 1] - data[i]) * 1e-6f; // Convert nanoseconds to milliseconds
         }

         profiler->_gpuStatsResultBuffer.Unmap();
      };
      _gpuStatsResultBuffer.MapAsync(WGPUMapMode_Read, 0, _gpuStatsResultBuffer.GetSize(), mapCallback);
   }
}

void Profiler::ResetUsage() const
{
   // Reset the usage flags for timestamps
   for (uint32_t i = 0; i < timestampCount; ++i)
   {
      _usedTimestamps[i] = false;
   }
}

float Profiler::GetQueryData(const uint32_t index) const
{
   if (index >= timestampCount / 2)
   {
      throw std::out_of_range("Index out of range in Profiler::GetQueryData");
   }
   return _timingResults[index];
}
