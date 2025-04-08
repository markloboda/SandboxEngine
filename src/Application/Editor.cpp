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
   int windowHeight = Application::GetInstance().GetWindowHeight();

   Renderer* renderer = Application::GetInstance().GetRenderer();


   // Editor settings
   {
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(250, static_cast<float>(windowHeight) / 2.0f), ImGuiCond_FirstUseEver);

      ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

      ImGui::Text("Editor Settings");

      ImGui::Separator();
      ImGui::Checkbox("Show Grid", &_showGrid);

      ImGui::Separator();
      ImGui::Checkbox("Render Clouds", &_renderClouds);

      ImGui::Separator();

      ImGui::Text("Camera");
      ImGui::Text("Position: (%.2f, %.2f, %.2f)", _camera->GetPosition().x, _camera->GetPosition().y, _camera->GetPosition().z);
      ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", _camera->GetEulerRotation().x, _camera->GetEulerRotation().y, _camera->GetEulerRotation().z);

      ImGui::End();
   }

   // Stats
   {
      Renderer::RenderStats stats = renderer->GetRenderStats();

      ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, 0), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Always);
      ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
      ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
      ImGui::Text("Frame Time: %.1f ms", 1000.0f / ImGui::GetIO().Framerate);

      if (ImGui::CollapsingHeader("Rendering Stats", ImGuiTreeNodeFlags_DefaultOpen))
      {
         ImGui::Text("Grid: %.1f ms", stats.gridTime);
         ImGui::Text("Cloud: %.1f ms", stats.cloudTime);
         ImGui::Text("UI: %.1f ms", stats.uiTime);
      }

      ImGui::End();
   }

   // Clouds
   {
      ImGui::Begin("Clouds", nullptr);
      {
         CloudRenderer::CloudRenderSettings* settings = &renderer->GetCloudRenderer()->Settings;

         // Cloud settings
         if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
         {
            ImGui::InputFloat("Start Height", &settings->cloudStartHeight);
            ImGui::InputFloat("End Height", &settings->cloudEndHeight);
         }

         if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen))
         {
            ImGui::SliderFloat("Density Multiplier", &settings->densityMultiplier, 0.01f, 5.0f);
            ImGui::SliderFloat("Density Threshold", &settings->densityThreshold, 0.01f, 1.0f);
         }

         if (ImGui::CollapsingHeader("Technical"))
         {
            ImGui::SliderInt("Cloud Steps", &settings->cloudNumSteps, 1, 512);
            ImGui::SliderInt("Light Steps", &settings->lightNumSteps, 1, 128);
            ImGui::SliderFloat("Max Step Size", &settings->cloudMaxStepSize, 0.01f, 100.0f);
         }
      }
      ImGui::End();
   }

}

void Editor::Update(float dt)
{
   // Update camera
   _camera->Update(dt);
}
