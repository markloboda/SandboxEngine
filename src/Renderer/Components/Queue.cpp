#include <pch.h>

WGPUQueueWorkDoneCallback queueWorkDone = [](WGPUQueueWorkDoneStatus status, void *, void *)
{
   std::cout << "Queued work finished with status: " << status << "\n";
};

Queue::Queue(Device *device) : _queue(wgpuDeviceGetQueue(device->Get()))
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

void Queue::Submit(size_t commandCount, CommandBuffer *commands) const
{
   std::vector<WGPUCommandBuffer> wgpuCommandBuffers(commandCount);
   for (size_t i = 0; i < commandCount; ++i)
   {
      wgpuCommandBuffers[i] = commands[i].Get();
   }
   wgpuQueueSubmit(_queue, commandCount, wgpuCommandBuffers.data());
}
