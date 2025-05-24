#include <pch.h>
#include <Renderer/Utils/Profiler.h>

Profiler::Profiler(Device &device, const uint32_t queryCount):
   _gpuStatsQuerySet(device, WGPUQueryType_Timestamp, queryCount * 2),
   _gpuStatsResolveBuffer(device, WGPUBufferUsage_QueryResolve | WGPUBufferUsage_CopySrc, _gpuStatsQuerySet.GetCount() * sizeof(uint64_t)),
   _gpuStatsResultBuffer(device, WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead, _gpuStatsResolveBuffer.GetSize()),
   _queryDataCount(queryCount * 2)
{
   _queryData = new float[_queryDataCount];
}

Profiler::~Profiler()
{
   delete[] _queryData;
}

void Profiler::ResolveQuerySet(const CommandEncoder &encoder) const
{
   encoder.ResolveQuerySet(_gpuStatsQuerySet, 0, _queryDataCount, _gpuStatsResolveBuffer, 0);
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
         }
         auto *profiler = static_cast<Profiler *>(userdata);
         const auto *data = static_cast<const uint64_t *>(profiler->_gpuStatsResultBuffer.GetMappedRange());

         for (uint32_t i = 0; i < profiler->_queryDataCount; i += 2)
         {
            profiler->_queryData[i / 2] = static_cast<float>(data[i + 1] - data[i]) * 1e-6f; // Convert to milliseconds
         }

         profiler->_gpuStatsResultBuffer.Unmap();
      };
      _gpuStatsResultBuffer.MapAsync(WGPUMapMode_Read, 0, _gpuStatsResultBuffer.GetSize(), mapCallback);
   }
}
