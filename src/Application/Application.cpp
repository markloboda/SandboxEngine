#include <pch.h>
#include <Application/Application.h>
#include <Renderer/Renderer.h>
#include <Application/Editor.h>

bool Application::Initialize()
{
   // Initialize GLFW.
   if (!glfwInit())
   {
      std::cerr << "Could not initialize GLFW!\n";
      return false;
   }

   // Open window.
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
   _window = glfwCreateWindow(_windowWidth, _windowHeight, "Volumetric Clouds", nullptr, nullptr);
   if (!_window)
   {
      std::cerr << "Could not open window!\n";
      glfwTerminate();
      return false;
   }

   Input::Initialize();

   // Initialize renderer.
   _renderer = new Renderer(_window);
   if (!_renderer)
   {
      std::cerr << "Could not initialize renderer!\n";
      return false;
   }

   // Initialize editor.
   _editor = new Editor();

   return true;
}

void Application::Terminate() const
{
   delete _editor;
   delete _renderer;
   Input::Terminate();
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

      glfwPollEvents();

      // Update.
      _editor->Update(dt);

      // Render.
      _renderer->Render();
   }
}

bool Application::IsRunning() const
{
   return !glfwWindowShouldClose(_window);
}
