#include <VolumetricClouds.h>
#include <Renderer/Surface.h>

#include <glfw3webgpu.h>

Surface::Surface(WGPUInstance instance, GLFWwindow* window)
{
   // Create a surface
   _surface = glfwGetWGPUSurface(instance, window);
   if (_surface == nullptr)
   {
      throw std::runtime_error("Failed to create surface");
   }
}

Surface::~Surface()
{
   wgpuSurfaceUnconfigure(_surface);
   wgpuSurfaceRelease(_surface);
}

void Surface::ConfigureSurface(WGPUSurfaceConfiguration config)
{
   wgpuSurfaceConfigure(_surface, &config);
}

WGPUSurfaceTexture Surface::GetNextSurfaceTexture() const
{
   WGPUSurfaceTexture surfaceTexture;
   wgpuSurfaceGetCurrentTexture(_surface, &surfaceTexture);
   if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
   {
      throw std::runtime_error("Failed to get surface texture in optimal way.");
   }

   return surfaceTexture;
}
