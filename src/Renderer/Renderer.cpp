#include <pch.h>

WGPULogCallback GetLogCallback()
{
   return [](WGPULogLevel level, WGPUStringView message, void */*userdata*/)
   {
      switch (level)
      {
         case WGPULogLevel_Off:
            break;
         case WGPULogLevel_Error:
            std::cerr << "WGPU Error: " << message.data << "\n";
            DEBUG_BREAK();
            break;
         case WGPULogLevel_Warn:
            std::cerr << "WGPU Warning: " << message.data << "\n";
            break;
         case WGPULogLevel_Info:
            std::cout << "WGPU Info: " << message.data << "\n";
            break;
         case WGPULogLevel_Debug:
            std::cout << "WGPU Debug: " << message.data << "\n";
            break;
         case WGPULogLevel_Trace:
            std::cout << "WGPU Trace: " << message.data << "\n";
            break;
         case WGPULogLevel_Force32:
            std::cerr << "WGPU Force32: " << message.data << "\n";
            DEBUG_BREAK();
            break;
      }
   };
};

Renderer::Renderer(GLFWwindow *window)
   : _window(window),
     _device(Device()),
     _surface(Surface(_device, window)),
     _queue(_device)
{
   bool success = Initialize();
   assert(success);
}

Renderer::~Renderer()
{
   Terminate();
}

bool Renderer::Initialize()
{
   // Set log level and callback.
   {
#ifdef _DEBUG
      wgpuSetLogLevel(WGPULogLevel_Error);
#else
      wgpuSetLogLevel(WGPULogLevel_Info);
#endif
      wgpuSetLogCallback(GetLogCallback(), nullptr);
   }

   // Profiler.
   _profiler = new Profiler(_device, 6);

   // Configure surface.
   {
      WGPUSurfaceConfiguration config = {};
      config.device = _device.Get();
      config.width = Application::GetInstance().GetWindowWidth();
      config.height = Application::GetInstance().GetWindowHeight();
      config.format = WGPUTextureFormat_RGBA8Unorm;
      config.usage = WGPUTextureUsage_RenderAttachment;
      config.presentMode = WGPUPresentMode_Immediate;
      config.viewFormatCount = 0;
      config.viewFormats = nullptr;
      config.alphaMode = WGPUCompositeAlphaMode_Auto;
      _surface.ConfigureSurface(config);
   }

   // Set frame buffer size callback.
   Application::SetWindowResizeCallback([this](int width, int height)
   {
      OnWindowResize(width, height);
   });

   // Depth texture.
   {
      WGPUTextureDescriptor desc = {};
      desc.dimension = WGPUTextureDimension_2D;
      desc.format = WGPUTextureFormat_Depth24Plus;
      desc.usage = WGPUTextureUsage_RenderAttachment;
      desc.size = WGPUExtent3D{
         .width = static_cast<uint32_t>(Application::GetInstance().GetWindowWidth()),
         .height = static_cast<uint32_t>(Application::GetInstance().GetWindowHeight()),
         .depthOrArrayLayers = 1u
      };
      desc.mipLevelCount = 1;
      desc.sampleCount = 1;
      _depthTexture = new Texture(_device, &desc);
      _depthTextureView = new TextureView(_depthTexture->Get(), nullptr);
   }

   // Set up ImGui.
   ImGuiManager::CreateInstance(*this);
   ImGuiManager::GetInstance().Configure(*this);

   // Set up renderers.
   _gridRenderer = new GridRenderer(*this);
   _atmosphereRenderer = new AtmosphereRenderer(*this);
   _clothRenderer = new ClothRenderer(*this);
   _cloudRenderer = new CloudRenderer(*this, Application::GetInstance().GetRuntime().GetWeatherSystem().Model);

   return true;
}

void Renderer::Terminate() const
{
   // Destroy renderers.
   delete _gridRenderer;
   delete _atmosphereRenderer;
   delete _cloudRenderer;
   delete _clothRenderer;

   // Destroy ImGui.
   ImGuiManager::GetInstance().Shutdown();
   ImGuiManager::DestroyInstance();

   // Destroy depth texture and view.
   delete _depthTextureView;
   delete _depthTexture;

   // Destroy profiler.
   delete _profiler;

   // UnConfigure surface.
   _surface.UnConfigureSurface();
}

void Renderer::Render()
{
   WGPUCommandEncoderDescriptor encoderDesc = {};
   std::string encoderLabel = "My Command Encoder (Renderer::Render())";
   encoderDesc.label = WGPUStringView{encoderLabel.c_str(), encoderLabel.size()};
   CommandEncoder encoder = CommandEncoder(_device, &encoderDesc);

   // Render pass.
   WGPUSurfaceTexture surfaceTexture;
   _surface.GetNextSurfaceTexture(surfaceTexture);
   const TextureView textureView = TextureView(surfaceTexture.texture, nullptr);

   // Renderers
   {
      ClearRenderPass(encoder, textureView, 0);

      if (RenderAtmosphere)
      {
         _atmosphereRenderer->Render(*this, encoder, textureView, 1);
      }

      if (RenderGrid)
      {
         _gridRenderer->Render(*this, encoder, textureView, 2);
      }

      if (RenderCloth)
      {
         _clothRenderer->Render(*this, encoder, textureView, 3);
      }

      if (RenderClouds)
      {
         _cloudRenderer->Render(*this, encoder, textureView, Application::GetInstance().GetRuntime().GetWeatherSystem().Model, 4);
      }

      // ImGui.
      ImGuiManager::GetInstance().Render(*this, encoder, textureView, 5);
   }

   if (_profilerEnabled)
   {
      _profiler->ResolveQuerySet(encoder);
   }

   CommandBuffer cmdBuffer = CommandBuffer(encoder);
   _queue.Submit(1, &cmdBuffer);
   _surface.Present();
   _device.Poll();

   if (_profilerEnabled)
   {
      _profiler->ReadResults();
      _profiler->ResetUsage();
      FetchProfileResults();
   }

   wgpuTextureRelease(surfaceTexture.texture);
}

bool Renderer::ShouldClose() const
{
   return glfwWindowShouldClose(_window);
}

void Renderer::OnWindowResize(int width, int height)
{
   // Depth texture
   {
      delete _depthTexture;
      delete _depthTextureView;

      // Recreate
      WGPUTextureDescriptor desc = {};
      desc.dimension = WGPUTextureDimension_2D;
      desc.format = WGPUTextureFormat_Depth24Plus;
      desc.usage = WGPUTextureUsage_RenderAttachment;
      desc.size = WGPUExtent3D{
         .width = static_cast<uint32_t>(width),
         .height = static_cast<uint32_t>(height),
         .depthOrArrayLayers = 1u
      };
      desc.mipLevelCount = 1;
      desc.sampleCount = 1;

      _depthTexture = new Texture(_device, &desc);
      _depthTextureView = new TextureView(_depthTexture->Get(), nullptr);
   }

   // Surface
   _surface.Resize(width, height);
}

void Renderer::UploadTextureData(const Texture &texture, const void *data, const size_t dataSize, const WGPUExtent3D *writeSize) const
{
   WGPUTexelCopyTextureInfo destination = {};
   destination.texture = texture.Get();
   destination.mipLevel = 0;
   destination.origin = {0, 0, 0};
   WGPUTexelCopyBufferLayout bufferLayout = {};
   bufferLayout.offset = 0;
   bufferLayout.bytesPerRow = static_cast<uint32_t>(dataSize / (writeSize->height * writeSize->depthOrArrayLayers));
   bufferLayout.rowsPerImage = writeSize->height;
   wgpuQueueWriteTexture(_queue.Get(), &destination, data, dataSize, &bufferLayout, writeSize);
}

void Renderer::UploadBufferData(const Buffer &buffer, const void *data, size_t size) const
{
   if (size > buffer.GetSize())
   {
      std::cerr << ("Data size exceeds buffer capacity");
   }

   wgpuQueueWriteBuffer(_queue.Get(), buffer.Get(), 0, data, size);
}

void Renderer::ClearRenderPass(const CommandEncoder &encoder, const TextureView &surfaceTextureView, int profilerIndex) const
{
   WGPURenderPassColorAttachment ca = {};
   ca.view = surfaceTextureView.Get();
   ca.loadOp = WGPULoadOp_Clear;
   ca.storeOp = WGPUStoreOp_Store;
   ca.clearValue = {0.1f, 0.1f, 0.1f, 1.0f};
   ca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
   WGPURenderPassDescriptor rpDesc = {};
   rpDesc.nextInChain = nullptr;
   rpDesc.colorAttachmentCount = 1;
   rpDesc.colorAttachments = &ca;
   rpDesc.depthStencilAttachment = nullptr;
   WGPURenderPassTimestampWrites rpTimestampWrites = {};
   _profiler->GetRenderPassTimestampWrites(profilerIndex, rpTimestampWrites);
   rpDesc.timestampWrites = &rpTimestampWrites;

   RenderPassEncoder clearPass = RenderPassEncoder(encoder.BeginRenderPass(&rpDesc));
   clearPass.EndPass();
}

void Renderer::FetchProfileResults()
{
   _gpuRenderStats.clearTime = _profiler->GetQueryData(0);
   _gpuRenderStats.atmosphereTime = _profiler->GetQueryData(1);
   _gpuRenderStats.gridTime = _profiler->GetQueryData(2);
   _gpuRenderStats.clothTime = _profiler->GetQueryData(3);
   _gpuRenderStats.cloudTime = _profiler->GetQueryData(4);
   _gpuRenderStats.uiTime = _profiler->GetQueryData(5);
   _gpuRenderStats.totalTime = _gpuRenderStats.clearTime + _gpuRenderStats.atmosphereTime +
                               _gpuRenderStats.gridTime + _gpuRenderStats.clothTime +
                               _gpuRenderStats.cloudTime + _gpuRenderStats.uiTime;
}
