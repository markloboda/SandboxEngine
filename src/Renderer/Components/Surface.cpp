#include <pch.h>

Surface::Surface(const Device &device, GLFWwindow *window) :
   _surface(glfwCreateWindowWGPUSurface(device.GetInstance(), window)),
   _config(WGPUSurfaceConfiguration{})
{
   if (!_surface)
   {
      std::cerr << ("Failed to create surface");
   }
}

Surface::~Surface()
{
   if (_surface)
   {
      wgpuSurfaceRelease(_surface);
   }
}

void Surface::Resize(const int width, const int height)
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

   if (config.width == 0)
   {
      config.width = 1;
   }

   if (config.height == 0)
   {
      config.height = 1;
   }

   _config = config;
   wgpuSurfaceConfigure(_surface, &_config);
}

void Surface::UnConfigureSurface() const
{
   wgpuSurfaceUnconfigure(_surface);
}

void Surface::GetNextSurfaceTexture(WGPUSurfaceTexture &surfaceTexture) const
{
   wgpuSurfaceGetCurrentTexture(_surface, &surfaceTexture);

   if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal)
   {
      std::cerr << "Warning: Surface texture is not in optimal state.\n";
   }
   else if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
   {
      std::cerr << ("Failed to get surface texture in optimal way.");
   }
}

void Surface::Present() const
{
   WGPUStatus status = wgpuSurfacePresent(_surface);
   if (status != WGPUStatus_Success)
   {
      std::cerr << ("Failed to present surface.");
   }
}
