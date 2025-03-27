#include <VolumetricClouds.h>

CommandEncoder::CommandEncoder(Device* device, WGPUCommandEncoderDescriptor* descriptor) :
   _encoder(wgpuDeviceCreateCommandEncoder(device->Get(), descriptor))
{
   if (!_encoder)
   {
      throw std::runtime_error("Failed to create command encoder");
   }
}

CommandEncoder::~CommandEncoder()
{
   if (_encoder)
   {
      wgpuCommandEncoderRelease(_encoder);
   }
}

WGPUCommandBuffer CommandEncoder::Finish()
{
   return wgpuCommandEncoderFinish(_encoder, nullptr);
}
