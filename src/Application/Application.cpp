#include <pch.h>

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
   _window = glfwCreateWindow(_windowWidth, _windowHeight, "Sandbox Engine", nullptr, nullptr);
   if (!_window)
   {
      std::cerr << "Could not open window!\n";
      glfwTerminate();
      return false;
   }

   // Set window callbacks.
   glfwSetFramebufferSizeCallback(_window, [](GLFWwindow * /*window*/, int width, int height)
   {
      for (auto &callback: GetInstance()._windowResizeCallbacks)
      {
         callback(width, height);
      }
   });

   Input::Initialize();

   // Initialize runtime.
   _runtime = new Runtime();
   if (!_runtime->Initialize())
   {
      std::cerr << "Runtime initialization failed!\n";
      delete _runtime;
      _runtime = nullptr;
      glfwDestroyWindow(_window);
      glfwTerminate();
      return false;
   }

   return true;
}

void Application::Terminate() const
{
   _runtime->Terminate();
   Input::Terminate();
   glfwDestroyWindow(_window);
   glfwTerminate();
}

void Application::Run()
{
   constexpr double targetFrameTime = 1.0 / 60.0; // 60 FPS
   constexpr int maxFixedIterations = 3;
   double lastTime = glfwGetTime();
   double accumulator = 0.0;

   while (IsRunning())
   {
      const double currentTime = glfwGetTime();
      double frameTime = currentTime - lastTime;
      lastTime = currentTime;

      glfwPollEvents();

      accumulator += frameTime;
      int iterations = 0;
      while (accumulator >= targetFrameTime && iterations < maxFixedIterations)
      {
         _runtime->FixedUpdate(static_cast<float>(targetFrameTime));

         accumulator -= targetFrameTime;
         ++iterations;
      }

      _runtime->Update(static_cast<float>(frameTime));
      _runtime->Render();
   }
}

bool Application::IsRunning() const
{
   return !glfwWindowShouldClose(_window);
}
