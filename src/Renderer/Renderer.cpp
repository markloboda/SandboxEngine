#include <pch.h>
#include <Renderer/Renderer.h>
#include <Application/Application.h>
#include <Application/Editor.h>
#include <Renderer/UI/ImGuiManager.h>

#include <Renderer/GridRenderer.h>
#include <Renderer/Clouds/CloudRenderer.h>

WGPULogCallback GetLogCallback()
{
   return [](WGPULogLevel level, WGPUStringView message, void*)
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

Renderer::Renderer(GLFWwindow* window)
   : _window(window),
   _device(Device()),
   _surface(Surface(&_device, window)),
   _queue(&_device)
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
   glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* /*window*/, int width, int height)
   {
      Application::GetInstance().GetRenderer()->OnWindowResize(width, height);
   });

   // Set up ImGui.
   ImGuiManager::CreateInstance(this);
   ImGuiManager::GetInstance().Configure(this);

   // Set up renderers.
   _gridRenderer = new GridRenderer(this);
   _cloudRenderer = new CloudRenderer(this);

   return true;
}

void Renderer::Terminate()
{
   // Destroy ImGui.
   ImGuiManager::GetInstance().Shutdown();
   ImGuiManager::DestroyInstance();

   // UnConfigure surface.
   _surface.UnConfigureSurface();
}

void Renderer::Render()
{
   WGPUCommandEncoderDescriptor encoderDesc = {};
   std::string encoderLabel = "My Command Encoder (Renderer::Render())";
   encoderDesc.label = WGPUStringView{ encoderLabel.c_str(), encoderLabel.size() };
   CommandEncoder encoder = CommandEncoder(&_device, &encoderDesc);

   // Render pass.
   WGPUSurfaceTexture surfaceTexture;
   _surface.GetNextSurfaceTexture(&surfaceTexture);
   TextureView textureView = TextureView(surfaceTexture.texture, nullptr);

   std::chrono::high_resolution_clock::time_point start, end;
   // Renderers
   {
      ClearRenderPass(&encoder, &textureView);

      start = std::chrono::high_resolution_clock::now();
      bool renderGrid = Application::GetInstance().GetEditor()->GetRenderGrid();
      if (renderGrid)
      {
         _gridRenderer->Render(this, &encoder, &textureView);
      }
      end = std::chrono::high_resolution_clock::now();
      _stats.gridTime = std::chrono::duration<float, std::milli>(end - start).count();

      start = std::chrono::high_resolution_clock::now();
      bool renderClouds = Application::GetInstance().GetEditor()->GetRenderClouds();
      if (renderClouds)
      {
         _cloudRenderer->Render(this, &encoder, &textureView);
      }
      end = std::chrono::high_resolution_clock::now();
      _stats.cloudTime = std::chrono::duration<float, std::milli>(end - start).count();

      // ImGui.
      start = std::chrono::high_resolution_clock::now();
      ImGuiManager::GetInstance().Render(this, &encoder, &textureView);
      end = std::chrono::high_resolution_clock::now();
      _stats.uiTime = std::chrono::duration<float, std::milli>(end - start).count();
   }

   CommandBuffer cmdBuffer = CommandBuffer(encoder.Finish());
   _queue.Submit(1, &cmdBuffer);
   _surface.Present();
   _device.Poll();

   wgpuTextureRelease(surfaceTexture.texture);
}

bool Renderer::ShouldClose() const
{
   return glfwWindowShouldClose(_window);
}

void Renderer::OnWindowResize(int width, int height)
{
   _surface.Resize(width, height);
}

void Renderer::UploadTextureData(Texture* texture, const void* data, size_t dataSize, const WGPUExtent3D* writeSize)
{
   WGPUTexelCopyTextureInfo destination = {};
   destination.texture = texture->Get();
   destination.mipLevel = 0;
   destination.origin = { 0, 0, 0 };
   WGPUTexelCopyBufferLayout bufferLayout = {};
   bufferLayout.offset = 0;
   bufferLayout.bytesPerRow = static_cast<uint32_t>(dataSize / (writeSize->height * writeSize->depthOrArrayLayers));
   bufferLayout.rowsPerImage = writeSize->height;
   wgpuQueueWriteTexture(_queue.Get(), &destination, data, dataSize, &bufferLayout, writeSize);
}

void Renderer::UploadBufferData(Buffer* buffer, const void* data, size_t size)
{
   if (size > buffer->GetSize())
   {
      std::cerr << ("Data size exceeds buffer capacity");
   }

   wgpuQueueWriteBuffer(_queue.Get(), buffer->Get(), 0, data, size);
}

void Renderer::ClearRenderPass(CommandEncoder* encoder, TextureView* surfaceTextureView)
{
   WGPURenderPassColorAttachment ca = {};
   ca.view = surfaceTextureView->Get();
   ca.loadOp = WGPULoadOp_Clear;
   ca.storeOp = WGPUStoreOp_Store;
   ca.clearValue = { 0.1f, 0.1f, 0.1f, 1.0f };
   ca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
   WGPURenderPassDescriptor rpDesc = {};
   rpDesc.nextInChain = nullptr;
   rpDesc.colorAttachmentCount = 1;
   rpDesc.colorAttachments = &ca;
   rpDesc.depthStencilAttachment = nullptr;
   RenderPassEncoder clearPass = RenderPassEncoder(encoder->BeginRenderPass(&rpDesc));
   clearPass.EndPass();
}
