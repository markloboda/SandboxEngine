#include <pch.h>
#include <Application/Editor.h>

#include <Application/Application.h>
#include <Renderer/Cloth/ClothParticleSystem.h>
#include <Renderer/Cloth/ClothRenderer.h>
#include <Renderer/UI/ImGuiManager.h>
#include <Utils/FreeCamera.h>

#include <Renderer/Sky/CloudRenderer.h>

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

   Renderer &renderer = Application::GetInstance().GetRenderer();

   // Editor settings
   {
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(250, static_cast<float>(windowHeight) / 2.0f), ImGuiCond_FirstUseEver);

      ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

      ImGui::Text("Editor Settings");
      ImGui::Separator();

      ImGui::Checkbox("Show Grid", &_renderGrid);
      ImGui::Separator();
      ImGui::Separator();

      ImGui::Checkbox("Show Atmosphere", &_renderAtmosphere);
      ImGui::Separator();

      ImGui::Checkbox("Render Clouds", &_renderClouds);
      ImGui::Separator();
      ImGui::Separator();

      ImGui::Checkbox("Show Cloths", &_renderCloths);
      ImGui::Separator();

      ImGui::Text("Camera");
      ImGui::Text("Position: (%.2f, %.2f, %.2f)", _camera->GetPosition().x, _camera->GetPosition().y, _camera->GetPosition().z);
      ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", _camera->GetEulerRotation().x, _camera->GetEulerRotation().y, _camera->GetEulerRotation().z);

      ImGui::End();
   }

   // Stats
   {
      const Renderer::GPURenderStats &stats = renderer.GetGPURenderStats();

      ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, 0), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Always);
      ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
      ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
      ImGui::Text("Frame Time: %.1f ms", 1000.0f / ImGui::GetIO().Framerate);

      if (renderer.IsProfilerEnabled())
      {
         if (ImGui::CollapsingHeader("Rendering Stats", ImGuiTreeNodeFlags_DefaultOpen))
         {
            ImGui::Text("Total GPU; %.1f ms", stats.totalTime);
            ImGui::Text("Clear: %.1f ms", stats.clearTime);
            ImGui::Text("Atmosphere: %.1f ms", stats.atmosphereTime);
            ImGui::Text("Grid: %.1f ms", stats.gridTime);
            ImGui::Text("Cloth: %.1f ms", stats.clothTime);
            ImGui::Text("Cloud: %.1f ms", stats.cloudTime);
            ImGui::Text("UI: %.1f ms", stats.uiTime);
         }
      }

      ImGui::End();
   }

   // Clouds
   {
      if (_renderClouds)
      {
         ImGui::Begin("Clouds", nullptr); {
            CloudRenderer::CloudRenderSettings &settings = renderer.GetCloudRenderer().Settings;

            // Cloud settings
            if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
            {
               ImGui::InputFloat("Start Height", &settings.cloudStartHeight);
               ImGui::InputFloat("End Height", &settings.cloudEndHeight);
            }

            if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen))
            {
               ImGui::SliderFloat("Light Absorption", &settings.lightAbsorption, 0.1f, 2.0f);
               ImGui::SliderFloat("Coverage Multiplier", &settings.coverageMultiplier, 0.0f, 10.0f);
               ImGui::SliderFloat("Phase Eccentricity", &settings.phaseEccentricity, -1.0f, 1.0f);
               ImGui::SliderFloat("Detail Strength", &settings.detailStrength, 0.0f, 1.0f);
               ImGui::SliderFloat("Density Multiplier", &settings.densityMultiplier, 0.0f, 10.0f);
            }

            if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
            {
               ImGui::InputInt("Cloud Raymarch Steps", &settings.cloudRaymarchSteps, 1, 1000);
               ImGui::InputInt("Light Raymarch Steps", &settings.lightRaymarchSteps, 1, 1000);
               ImGui::InputFloat("Light Step Length", &settings.lightStepLength, 0.1f, 500.0f, "%.1f");
            }
         }
         ImGui::End();
      }
   }
}

void Editor::Update(const float dt) const
{
   // Update camera
   _camera->Update(dt);
}
