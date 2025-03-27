#pragma once

#include <Renderer/BindGroup.h>
#include <Renderer/Buffer.h>
#include <Renderer/CommandBuffer.h>
#include <Renderer/CommandEncoder.h>
#include <Renderer/Device.h>
#include <Renderer/Queue.h>
#include <Renderer/RenderPassEncoder.h>
#include <Renderer/Surface.h>
#include <Renderer/Queue.h>
#include <Renderer/TextureView.h>

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