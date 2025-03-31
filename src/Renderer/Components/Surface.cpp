#include <pch.h>

Surface::Surface(Device* device, GLFWwindow* window) :
   _surface(glfwGetWGPUSurface(device->GetInstance(), window)),
   _config(WGPUSurfaceConfiguration{})
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

void Surface::Resize(int width, int height)
{
   _config.width = width;
   _config.height = height;
   ConfigureSurface(_config);
}

void Surface::ConfigureSurface(WGPUSurfaceConfiguration config)
{
   if (!config.format)
   {
      config.format = WGPUTextureFormat_RGBA8Unorm;
   }

   _config = config;
   wgpuSurfaceConfigure(_surface, &_config);
}

void Surface::UnConfigureSurface()
{
   wgpuSurfaceUnconfigure(_surface);
}

WGPUSurfaceTexture Surface::GetNextSurfaceTexture() const
{
   WGPUSurfaceTexture surfaceTexture;
   wgpuSurfaceGetCurrentTexture(_surface, &surfaceTexture);

   if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal)
   {
      std::cerr << "Warning: Surface texture is not in optimal state.\n";
   }
   else if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
   {
      throw std::runtime_error("Failed to get surface texture in optimal way.");
   }

   return surfaceTexture;
}

void Surface::Present() const
{
   wgpuSurfacePresent(_surface);
}
