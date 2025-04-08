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

   // Clouds
   {
      ImGui::Begin("Clouds", nullptr);
      {
         CloudRenderer::CloudRenderSettings* settings = &Application::GetInstance().GetRenderer()->GetCloudRenderer()->Settings;

         ImGui::InputFloat("Start Height", &settings->cloudStartHeight);
         ImGui::InputFloat("End Height", &settings->cloudEndHeight);
         ImGui::InputFloat("Cloud Scale", &settings->cloudScale);
         ImGui::InputFloat("Density Multiplier", &settings->densityMultiplier);
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
      ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, 0), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Always);
      ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
      ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
      ImGui::End();
   }
}

void Editor::Update(float dt)
{
   // Update camera
   _camera->Update(dt);
}
