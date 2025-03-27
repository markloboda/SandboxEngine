#include <VolumetricClouds.h>
#include <Renderer/Queue.h>

WGPUQueueWorkDoneCallback queueWorkDone = [](WGPUQueueWorkDoneStatus status, void*, void*)
{
   std::cout << "Queued work finished with status: " << status << std::endl;
};

Queue::Queue(WGPUDevice device)
{
   _queue = wgpuDeviceGetQueue(device);

   // Queue work done callback
   WGPUQueueWorkDoneCallbackInfo callbackInfo = {};
   callbackInfo.callback = queueWorkDone;
   wgpuQueueOnSubmittedWorkDone(_queue, callbackInfo);

}

Queue::~Queue()
{}
