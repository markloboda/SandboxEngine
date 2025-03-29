#include <pch.h>
#include <Application/Application.h>
#include <Renderer/Renderer.h>
#include <Application/Editor.h>

bool Application::Initialize()
{
   // Initialize GLFW.
   if (!glfwInit())
   {
      std::cerr << "Could not initialize GLFW!" << std::endl;
      return false;
   }

   // Open window.
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
   _window = glfwCreateWindow(_windowWidth, _windowHeight, "Volumetric Clouds", nullptr, nullptr);
   if (!_window)
   {
      std::cerr << "Could not open window!" << std::endl;
      glfwTerminate();
      return false;
   }

   // Initialize renderer.
   _renderer = new Renderer(_window);
   if (!_renderer)
   {
      std::cerr << "Could not initialize renderer!" << std::endl;
      return false;
   }

   // Initialize editor.
   _editor = new Editor();

   return true;
}

void Application::Terminate() const
{
   glfwDestroyWindow(_window);
   glfwTerminate();
}

void Application::Run()
{
   double lastTime = glfwGetTime();
   while (IsRunning())
   {
      const double currentTime = glfwGetTime();
      const float dt = static_cast<float>(currentTime - lastTime);
      lastTime = currentTime;

      // Render.
      _renderer->Render();

      glfwPollEvents();
   }
}

bool Application::IsRunning() const
{
   return !glfwWindowShouldClose(_window);
}