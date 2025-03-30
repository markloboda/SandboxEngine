#include <pch.h>

WGPUDeviceLostCallback deviceLostCallback = [](WGPUDevice const* device, WGPUDeviceLostReason reason, WGPUStringView message, void*, void*)
{
   std::cerr << "Device lost: reason " << reason;
   std::cerr << ", message: " << (message.data ? message.data : "Unknown error") << "\n";
   __debugbreak();
};

WGPUPopErrorScopeCallback popErrorScopeCallback = [](WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message, void*, void*)
{
   std::cerr << "Error: " << (message.data ? message.data : "Unknown error") << "\n";
   __debugbreak();
};

WGPUUncapturedErrorCallback uncapturedErrorCallback = [](WGPUDevice const* device, WGPUErrorType type, WGPUStringView message, void*, void*)
{
   std::cerr << "Uncaptured error: " << (message.data ? message.data : "Unknown error") << "\n";
   __debugbreak();
};

Device::Device()
{   // Create the instance.
   {
      WGPUInstanceExtras extras = {};
      extras.chain.sType = (WGPUSType)WGPUSType_InstanceExtras;
      extras.flags = WGPUInstanceFlag_Validation | WGPUInstanceFlag_Debug;

      WGPUInstanceDescriptor instanceDesc = {};
      instanceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&extras);
      _instance = wgpuCreateInstance(&instanceDesc);
      if (!_instance)
      {
         throw std::runtime_error("Failed to create WebGPU instance");
      }

      // Request adapter.
      std::promise<WGPUAdapter> adapterPromise;
      auto adapterFuture = adapterPromise.get_future();

      WGPURequestAdapterOptions adapterOptions = {};
      adapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;

      WGPURequestAdapterCallbackInfo adapterCallbackInfo = {};
      adapterCallbackInfo.userdata1 = &adapterPromise;
      adapterCallbackInfo.callback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2)
      {
         std::promise<WGPUAdapter>* promise = reinterpret_cast<std::promise<WGPUAdapter>*>(userdata1);
         if (status == WGPURequestAdapterStatus_Success)
         {
            promise->set_value(adapter);
         }
         else
         {
            std::cerr << "Adapter request failed: " << (message.data ? message.data : "Unknown error") << "\n";
            promise->set_value(nullptr);
         }
      };
      wgpuInstanceRequestAdapter(_instance, &adapterOptions, adapterCallbackInfo);
      _adapter = adapterFuture.get();
      if (!_adapter)
      {
         throw std::runtime_error("Failed to obtain WebGPU adapter");
      }
   }

   // Request device.
   {
      std::promise<WGPUDevice> devicePromise;
      auto deviceFuture = devicePromise.get_future();

      WGPUDeviceLostCallbackInfo deviceLostCallbackInfo = {};
      deviceLostCallbackInfo.callback = deviceLostCallback;

      WGPUUncapturedErrorCallbackInfo uncapturedErrorCallbackInfo = {};
      uncapturedErrorCallbackInfo.callback = uncapturedErrorCallback;

      WGPUDeviceDescriptor deviceDesc = {};
      deviceDesc.deviceLostCallbackInfo = deviceLostCallbackInfo;
      deviceDesc.uncapturedErrorCallbackInfo = uncapturedErrorCallbackInfo;

      WGPUFeatureName features[] = {
          static_cast<WGPUFeatureName>(WGPUNativeFeature_SpirvShaderPassthrough)
      };

      deviceDesc.requiredFeatures = features;
      deviceDesc.requiredFeatureCount = 1;

      WGPURequestDeviceCallbackInfo deviceCallbackInfo = {};
      deviceCallbackInfo.nextInChain = nullptr;
      deviceCallbackInfo.callback = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void*)
      {
         std::promise<WGPUDevice>* promise = reinterpret_cast<std::promise<WGPUDevice>*>(userdata1);
         if (status == WGPURequestDeviceStatus_Success)
         {
            promise->set_value(device);
         }
         else
         {
            std::cerr << "Device request failed: " << (message.data ? message.data : "Unknown error") << "\n";
            promise->set_value(nullptr);
         }
      };
      deviceCallbackInfo.userdata1 = &devicePromise;
      wgpuAdapterRequestDevice(_adapter, &deviceDesc, deviceCallbackInfo);
      _device = deviceFuture.get();
      if (!_device)
      {
         throw std::runtime_error("Failed to create WebGPU device");
      }
   }

   // Retrieve the queue from the device.
   {
      _queue = wgpuDeviceGetQueue(_device);
      if (!_queue)
      {
         throw std::runtime_error("Failed to obtain WebGPU queue");
      }
      WGPUQueueWorkDoneCallbackInfo queueWorkDoneCallbackInfo = {};
      queueWorkDoneCallbackInfo.callback = [](WGPUQueueWorkDoneStatus status, void* userdata1, void*)
      {
         if (status != WGPUQueueWorkDoneStatus_Success)
         {
            std::cerr << "Queue work done failed\n";
            __debugbreak();
         }
      };
      wgpuQueueOnSubmittedWorkDone(_queue, queueWorkDoneCallbackInfo);
   }
}

Device::~Device()
{
   if (_device)
   {
      wgpuDeviceRelease(_device);
   }
   if (_adapter)
   {
      wgpuAdapterRelease(_adapter);
   }
   if (_instance)
   {
      wgpuInstanceRelease(_instance);
   }
}

WGPUShaderModule Device::CreateShaderModule(const std::vector<uint32_t>& spirvCode) const
{
   WGPUShaderSourceSPIRV spirvDesc{};
   spirvDesc.chain.sType = WGPUSType_ShaderSourceSPIRV;
   spirvDesc.codeSize = spirvCode.size() * sizeof(uint32_t);
   spirvDesc.code = spirvCode.data();

   WGPUShaderModuleDescriptor desc{};
   desc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&spirvDesc);

   return wgpuDeviceCreateShaderModule(_device, &desc);
}

void Device::Poll() const
{
   wgpuDevicePoll(_device, false, nullptr);
}
