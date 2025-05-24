#include <pch.h>

WGPUQueueWorkDoneCallback queueWorkDone = [](const WGPUQueueWorkDoneStatus status, void *, void *)
{
   std::cout << "Queued work finished with status: " << status << "\n";
};

Queue::Queue(const Device &device) : _queue(wgpuDeviceGetQueue(device.Get()))
{
   // Queue work done callback
   WGPUQueueWorkDoneCallbackInfo callbackInfo = {};
   callbackInfo.callback = queueWorkDone;
   wgpuQueueOnSubmittedWorkDone(_queue, callbackInfo);
}

Queue::~Queue()
{
   if (_queue)
   {
      wgpuQueueRelease(_queue);
   }
}

void Queue::Submit(const size_t commandCount, const CommandBuffer *commands) const
{
   std::vector<WGPUCommandBuffer> wgpuCommandBuffers(commandCount);
   for (size_t i = 0; i < commandCount; ++i)
   {
      wgpuCommandBuffers[i] = commands[i].Get();
   }
   wgpuQueueSubmit(_queue, commandCount, wgpuCommandBuffers.data());
}
