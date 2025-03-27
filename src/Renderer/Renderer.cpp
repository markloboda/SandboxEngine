#include <VolumetricClouds.h>
#include <Renderer/Renderer.h>


#include <GLFW/glfw3.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <Application/Application.h>

class Application;

Renderer::Renderer(GLFWwindow* window)
   : _window(window),
   _device(Device()),
   _surface(Surface(&_device, window)),
   _queue(&_device),
   _imGuiManager(_window)
{
   bool success = Initialize();
   assert(success);
}

Renderer::~Renderer()
{}

bool Renderer::Initialize()
{
   // Configure surface.
   {
      WGPUSurfaceConfiguration config = {};
      config.device = _device.Get();
      config.width = Application::Instance->GetWindowWidth();
      config.height = Application::Instance->GetWindowHeight();
      config.viewFormatCount = 0;
      config.viewFormats = nullptr;
      config.usage = WGPUTextureUsage_RenderAttachment;
      config.presentMode = WGPUPresentMode_Immediate;
      _surface.ConfigureSurface(config);
   }

   // Configure ImGui.
   _imGuiManager.Configure(&_device, _surface.GetFormat());

   return true;
}

void Renderer::Terminate()
{
   // UnConfigure ImGui.
   _imGuiManager.Shutdown();

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
   _imGuiManager.NewFrame();
   _imGuiManager.RenderUI();
   _imGuiManager.EndFrame(&renderPassEncoder);

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
