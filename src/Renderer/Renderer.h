#pragma once

#include <Renderer/ImGuiManager.h>

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

   ImGuiManager _imGuiManager;

public:
   void Render();
   bool ShouldClose() const;

private:
   bool Initialize();
   void Terminate();
};