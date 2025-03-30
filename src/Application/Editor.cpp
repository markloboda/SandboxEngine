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
   ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_FirstUseEver);
   ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

   // Editor settings
   ImGui::Text("Editor Settings");
   ImGui::Separator();
   ImGui::Checkbox("Show Grid", &_ShowGrid);

   ImGui::End();
}

void Editor::Update(float dt)
{
   // Update camera
   _camera->Update(dt);
}
