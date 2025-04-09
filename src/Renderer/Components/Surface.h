#pragma once

struct GLFWwindow;
class Device;
class TextureView;

class Surface
{
private:
   WGPUSurface _surface;
   WGPUSurfaceConfiguration _config;

public:
   Surface(Device* device, GLFWwindow* window);
   ~Surface();

   WGPUSurface Get() const { return _surface; }
   WGPUTextureFormat GetFormat() const { return _config.format; }
   void Resize(int width, int height);

   void ConfigureSurface(WGPUSurfaceConfiguration config);
   void UnConfigureSurface();

   void GetNextSurfaceTexture(WGPUSurfaceTexture* surfaceTexture) const;
   void Present() const;
};
