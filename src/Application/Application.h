#pragma once

class Renderer;
class Editor;

class Application
{
public:
   // Singleton instance access
   static Application& GetInstance()
   {
      static Application instance;
      return instance;
   }

private:
   int _windowWidth = 1920;
   int _windowHeight = 1060;

   GLFWwindow* _window = nullptr;
   Renderer* _renderer = nullptr;
   Editor* _editor = nullptr;

   // Private constructor for singleton
   Application() = default;
   ~Application() = default;

public:
   // Prevent copy/move construction
   Application(const Application&) = delete;
   Application& operator=(const Application&) = delete;

   // Prevent copy/move assignment
   Application(Application&&) = delete;
   Application& operator=(Application&&) = delete;

   // Getters.
   [[nodiscard]] bool IsRunning() const;

   GLFWwindow* GetWindow() const
   {
      return _window;
   }

   [[nodiscard]] int GetWindowWidth() const
   {
      return _windowWidth;
   }

   [[nodiscard]] int GetWindowHeight() const
   {
      return _windowHeight;
   }

   Editor &GetEditor() const
   {
      return *_editor;
   }

   Renderer &GetRenderer() const
   {
      return *_renderer;
   }

   // Manages the application.
   bool Initialize();
   void Terminate() const;
   void Run();
};
