#include <pch.h>
#include <Application/Editor.h>

#include <Renderer/UI/ImGuiManager.h>

Editor::Editor()
{
   // Add to ImGuiManager
   ImGuiManager::GetInstance().AddUIRenderer(this);
}

Editor::~Editor()
{
   // Remove from ImGuiManager
   ImGuiManager::GetInstance().RemoveUIRenderer(this);
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
