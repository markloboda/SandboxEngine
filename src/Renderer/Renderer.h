#pragma once

class GridRenderer;
struct GLFWwindow;

class Renderer
{
private:
   GLFWwindow* _window;
   Device _device;
   Surface _surface;
   Queue _queue;

   GridRenderer* _gridRenderer;

public:
   Renderer(GLFWwindow* window);
   ~Renderer();

public:
   void Render();
   bool ShouldClose() const;

   void OnWindowResize(int width, int height);

private:
   bool Initialize();
   void Terminate();
};