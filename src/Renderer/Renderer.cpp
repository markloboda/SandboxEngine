#include <pch.h>
#include <Renderer/Renderer.h>
#include <Application/Application.h>
#include <Renderer/UI/ImGuiManager.h>


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
   // Configure surface.
   {
      WGPUSurfaceConfiguration config = {};
      config.device = _device.Get();
      config.width = Application::GetInstance().GetWindowWidth();
      config.height = Application::GetInstance().GetWindowHeight();
      config.viewFormatCount = 0;
      config.viewFormats = nullptr;
      config.usage = WGPUTextureUsage_RenderAttachment;
      config.presentMode = WGPUPresentMode_Immediate;
      _surface.ConfigureSurface(config);
   }

   // Set up ImGui.
   ImGuiManager::CreateInstance(_window);
   ImGuiManager::GetInstance().Configure(&_device, _surface.GetFormat());

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
   CommandEncoder encoder = CommandEncoder(&_device, nullptr);

   // Render pass.
   WGPUSurfaceTexture surfaceTexture = _surface.GetNextTexture();
   TextureView textureView = TextureView(surfaceTexture.texture, nullptr);

   WGPURenderPassColorAttachment renderPassColorAttachment = {};
   renderPassColorAttachment.view = textureView.Get();
   renderPassColorAttachment.resolveTarget = nullptr;
   renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
   renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
   renderPassColorAttachment.clearValue = WGPUColor{ 1.0, 1.0, 1.0, 1.0 };
   WGPURenderPassDescriptor renderPassDesc = {};
   renderPassDesc.nextInChain = nullptr;
   renderPassDesc.colorAttachmentCount = 1;
   renderPassDesc.colorAttachments = &renderPassColorAttachment;
   renderPassDesc.depthStencilAttachment = nullptr;
   renderPassDesc.timestampWrites = nullptr;
   RenderPassEncoder renderPassEncoder = RenderPassEncoder(&encoder, &renderPassDesc);

   // ImGui.
   ImGuiManager::GetInstance().NewFrame();
   ImGuiManager::GetInstance().RenderUI();
   ImGuiManager::GetInstance().EndFrame(&renderPassEncoder);

   // End render pass.
   renderPassEncoder.EndPass();
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
