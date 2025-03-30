#pragma once

class GridRenderer;
struct GLFWwindow;

class Renderer
{
public:
   Renderer(GLFWwindow* window);
   ~Renderer();

private:
   GLFWwindow* _window;
   Device _device;
   Surface _surface;
   Queue _queue;

   GridRenderer* _gridRenderer;

public:
   void Render();
   bool ShouldClose() const;

private:
   bool Initialize();
   void Terminate();
};