#pragma once
#include <webgpu/webgpu.h>

class Queue
{
public:
   Queue(WGPUDevice device);
   ~Queue();

private:
   WGPUQueue _queue = nullptr;

public:
   WGPUQueue Get() const { return _queue; }
};