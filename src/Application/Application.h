#pragma once

struct GLFWwindow;
class Renderer;

class Application
{
private:
   int _windowWidth = 1280;
   int _windowHeight = 720;

   GLFWwindow* _window = nullptr;
   Renderer* _renderer = nullptr;

public:
   bool Initialize();

   void Terminate() const;

   // Runs the main loop.
   void Run();

   [[nodiscard]] bool IsRunning() const;
};
