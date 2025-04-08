#include <pch.h>
#include <Application/Editor.h>

#include <Application/Application.h>
#include <Renderer/UI/ImGuiManager.h>
#include <Utils/FreeCamera.h>

#include <Renderer/Clouds/CloudRenderer.h>

#include "Renderer/Renderer.h"

Editor::Editor()
{
   // Add to ImGuiManager
   ImGuiManager::GetInstance().AddUIRenderer(this);

   // Create camera
   _camera = new FreeCamera();
   _camera->SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));

   // Create scene
   _scene = new Scene();
}

Editor::~Editor()
{
   // Delete scene
   delete _scene;

   // Remove from ImGuiManager
   ImGuiManager::GetInstance().RemoveUIRenderer(this);

   // Delete camera
   delete _camera;
}

void Editor::RenderImGuiUI()
{
   int windowWidth = Application::GetInstance().GetWindowWidth();
   int windowHeight = Application::GetInstance().GetWindowHeight();

   Renderer* renderer = Application::GetInstance().GetRenderer();

   // Clouds
   {
      ImGui::Begin("Clouds", nullptr);
      {
         CloudRenderer::CloudRenderSettings* settings = &renderer->GetCloudRenderer()->Settings;

         ImGui::InputFloat("Start Height", &settings->cloudStartHeight);
         ImGui::InputFloat("End Height", &settings->cloudEndHeight);
         ImGui::SliderFloat("Density Multiplier", &settings->densityMultiplier, 0.01f, 5.0f);
         ImGui::SliderFloat("Density Threshold", &settings->densityThreshold, 0.01f, 1.0f);
      }
      ImGui::End();
   }

   // Editor settings
   {
      ImGui::SetNextWindowPos(ImVec2(0, windowHeight / 2));
      ImGui::SetNextWindowSize(ImVec2(200, windowHeight / 2), ImGuiCond_FirstUseEver);

      ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

      ImGui::Text("Editor Settings");

      ImGui::Separator();
      ImGui::Checkbox("Show Grid", &_showGrid);

      ImGui::Separator();
      ImGui::Checkbox("Render Clouds", &_renderClouds);

      ImGui::End();
   }

   // Stats
   {
      Renderer::RenderStats stats = renderer->GetRenderStats();

      ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, 0), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Always);
      ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
      ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

      ImGui::BeginChild("Stats", ImVec2(200, 100));
      ImGui::Text("Render Stats:");
      ImGui::Text("Grid: %.1f ms", stats.gridTime);
      ImGui::Text("Cloud: %.1f ms", stats.cloudTime);
      ImGui::Text("UI: %.1f ms", stats.uiTime);
      ImGui::EndChild();

      ImGui::End();
   }
}

void Editor::Update(float dt)
{
   // Update camera
   _camera->Update(dt);
}
