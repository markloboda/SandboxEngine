#pragma once

class Application
{
public:
   // Singleton instance access
   static Application& GetInstance()
   {
      static Application instance;
      return instance;
   }

   using WindowResizeCallback = std::function<void(int, int)>;

private:
   int _windowWidth = 1920;
   int _windowHeight = 1060;

   GLFWwindow* _window = nullptr;
   Runtime *_runtime = nullptr;

   std::vector<WindowResizeCallback> _windowResizeCallbacks;

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

   [[nodiscard]] Runtime &GetRuntime() const
   {
      return *_runtime;
   }

   [[nodiscard]] int GetWindowWidth() const
   {
      return _windowWidth;
   }

   [[nodiscard]] int GetWindowHeight() const
   {
      return _windowHeight;
   }

   static void SetWindowResizeCallback(const WindowResizeCallback &callback)
   {
      GetInstance()._windowResizeCallbacks.push_back(callback);
   }

   // Manages the application.
   bool Initialize();
   void Terminate() const;
   void Run();
};
