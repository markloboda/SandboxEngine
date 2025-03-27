#pragma once
#include <webgpu/webgpu.h>

struct GLFWwindow;

class Surface
{
public:
   Surface(WGPUInstance instance, GLFWwindow* window);
   ~Surface();

private:
   WGPUSurface _surface;

public:
   WGPUSurface Get() const { return _surface; }
   void ConfigureSurface(WGPUSurfaceConfiguration config);
   WGPUSurfaceTexture GetNextSurfaceTexture() const;
};
