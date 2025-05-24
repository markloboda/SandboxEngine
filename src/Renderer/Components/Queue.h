#pragma once

class Device;
class CommandBuffer;

class Queue
{
private:
   WGPUQueue _queue;

public:
   Queue(const Device &device);
   ~Queue();

   WGPUQueue Get() const { return _queue; }

   void Submit(size_t commandCount, const CommandBuffer *commands) const;
};