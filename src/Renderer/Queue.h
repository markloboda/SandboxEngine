#pragma once
#include <webgpu/webgpu.h>

class Device;
class CommandBuffer;

class Queue
{
public:
   Queue(Device* device);
   ~Queue();

private:
   WGPUQueue _queue;

public:
   WGPUQueue Get() const { return _queue; }

   void Submit(size_t commandCount, CommandBuffer* commands) const;
};