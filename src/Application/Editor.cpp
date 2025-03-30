#include <pch.h>
#include <Application/Editor.h>

#include <Renderer/UI/ImGuiManager.h>
#include <Utils/FreeCamera.h>

Editor::Editor()
{
   // Add to ImGuiManager
   ImGuiManager::GetInstance().AddUIRenderer(this);

   // Create camera
   _camera = new FreeCamera();
   _camera->SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));
}

Editor::~Editor()
{
   // Remove from ImGuiManager
   ImGuiManager::GetInstance().RemoveUIRenderer(this);

   // Delete camera
   delete _camera;
}

void Editor::RenderImGuiUI()
{
   // Editor window with settings always at left side and stretch to height

   // Editor settings
   {
      ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
      ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_FirstUseEver);
      ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

      ImGui::Text("Editor Settings");
      ImGui::Separator();
      ImGui::Checkbox("Show Grid", &_showGrid);

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
