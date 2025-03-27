#pragma once

struct GLFWwindow;
class Renderer;

class Application
{
public:
   static inline Application* Instance = nullptr;

private:
   int _windowWidth = 1280;
   int _windowHeight = 720;

   GLFWwindow* _window = nullptr;
   Renderer* _renderer = nullptr;

public:
   // Getters.
   [[nodiscard]] bool IsRunning() const;

   [[nodiscard]] int GetWindowWidth() const
   {
      return _windowWidth;
   }

   [[nodiscard]] int GetWindowHeight() const
   {
      return _windowHeight;
   }

   // Manages the application.
   bool Initialize();
   void Terminate() const;
   void Run();


};
