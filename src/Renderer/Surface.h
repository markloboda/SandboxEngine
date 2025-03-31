#pragma once

struct GLFWwindow;
class Device;
class TextureView;

class Surface
{
public:
   Surface(Device* device, GLFWwindow* window);
   ~Surface();

private:
   WGPUSurface _surface;
   WGPUSurfaceConfiguration _config;

public:
   WGPUSurface Get() const { return _surface; }
   WGPUTextureFormat GetFormat() const { return _config.format; }
   void Resize(int width, int height);

   void ConfigureSurface(WGPUSurfaceConfiguration config);
   void UnConfigureSurface();

   WGPUSurfaceTexture GetSurfaceTexture() const;
   void Present() const;
};
