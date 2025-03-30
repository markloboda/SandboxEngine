#pragma once

class GLFWwindow;
class Device;
class TextureView;

class Surface
{
public:
   Surface(Device* device, GLFWwindow* window);
   ~Surface();

private:
   WGPUSurface _surface;
   WGPUTextureFormat _format = WGPUTextureFormat_Undefined;

public:
   WGPUSurface Get() const { return _surface; }
   WGPUTextureFormat GetFormat() const { return _format; }

   void ConfigureSurface(WGPUSurfaceConfiguration config);
   void UnConfigureSurface();

   WGPUSurfaceTexture GetNextTexture() const;
   void Present() const;

private:
};
