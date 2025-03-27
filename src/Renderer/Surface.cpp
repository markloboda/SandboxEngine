#include <VolumetricClouds.h>
#include <Renderer/Surface.h>

#include <glfw3webgpu.h>

#include <Renderer/Device.h>

Surface::Surface(Device* device, GLFWwindow* window) :
   _surface(glfwGetWGPUSurface(device->GetInstance(), window))
{
   if (!_surface)
   {
      throw std::runtime_error("Failed to create surface");
   }
}

Surface::~Surface()
{
   if (_surface)
   {
      wgpuSurfaceRelease(_surface);
   }
}

void Surface::ConfigureSurface(WGPUSurfaceConfiguration config)
{
   if (!config.format)
   {
      config.format = WGPUTextureFormat_RGBA8Unorm;
   }

   wgpuSurfaceConfigure(_surface, &config);
   _format = config.format;
}

void Surface::UnConfigureSurface()
{
   wgpuSurfaceUnconfigure(_surface);
}

WGPUSurfaceTexture Surface::GetNextTexture() const
{
   WGPUSurfaceTexture surfaceTexture;
   wgpuSurfaceGetCurrentTexture(_surface, &surfaceTexture);
   if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
   {
      throw std::runtime_error("Failed to get surface texture in optimal way.");
   }

   return surfaceTexture;
}

void Surface::Present() const
{
   wgpuSurfacePresent(_surface);
}

