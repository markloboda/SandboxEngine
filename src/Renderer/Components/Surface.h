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
   Surface(const Device &device, GLFWwindow *window);
   ~Surface();

   [[nodiscard]] WGPUSurface Get() const { return _surface; }
   [[nodiscard]] WGPUTextureFormat GetFormat() const { return _config.format; }
   void Resize(int width, int height);

   void ConfigureSurface(WGPUSurfaceConfiguration config);
   void UnConfigureSurface() const;

   void GetNextSurfaceTexture(WGPUSurfaceTexture &surfaceTexture) const;
   void Present() const;
};
