#include <VolumetricClouds.h>
#include <Renderer/Renderer.h>


#include <GLFW/glfw3.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

Renderer::Renderer(GLFWwindow* window)
   : _window(window),
   _device(Device()),
   _surface(Surface(_device.GetInstance(), window)),
   _queue(_device.Get())
{
   bool success = Initialize();
   assert(success);
}

Renderer::~Renderer()
{

}

bool Renderer::Initialize()
{
   // Configure surface.

   return true;
}

void Renderer::Render()
{}

bool Renderer::ShouldClose() const
{
   return glfwWindowShouldClose(_window);
}
