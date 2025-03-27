#include <VolumetricClouds.h>
#include <Renderer/ImGuiManager.h>

ImGuiManager::ImGuiManager(GLFWwindow* window) :
   _window(window)
{
   // Create ImGui context and setup backend bindings.
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOther(_window, true);
   ImGui::StyleColorsClassic();
}

ImGuiManager::~ImGuiManager()
{
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
}

void ImGuiManager::Configure(Device* device, WGPUTextureFormat renderTargetFormat)
{
   _renderTargetFormat = renderTargetFormat;

   // Initialize WGPU ImGui backend
   ImGui_ImplWGPU_InitInfo initInfo = {};
   initInfo.Device = device->Get();
   initInfo.NumFramesInFlight = 3;
   initInfo.RenderTargetFormat = _renderTargetFormat;
   initInfo.DepthStencilFormat = WGPUTextureFormat_Undefined;
   initInfo.PipelineMultisampleState.count = 1;
   initInfo.PipelineMultisampleState.mask = UINT32_MAX;
   ImGui_ImplWGPU_Init(&initInfo);

   // Setup ImGui style
   SetupStyle();
}

void ImGuiManager::Shutdown()
{
   ImGui_ImplWGPU_Shutdown();
}

void ImGuiManager::SetupStyle()
{
   ImGuiStyle& style = ImGui::GetStyle();
   style.WindowRounding = 5.0f;
   style.FrameRounding = 4.0f;
   style.GrabRounding = 3.0f;
   style.WindowBorderSize = 1.0f;
   style.FrameBorderSize = 1.0f;

   ImVec4 bgColor = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);  // Semi-transparent dark
   ImVec4 accentColor = ImVec4(0.9f, 0.3f, 0.3f, 1.0f); // Red accent

   style.Colors[ImGuiCol_WindowBg] = bgColor;
   style.Colors[ImGuiCol_TitleBg] = accentColor;
   style.Colors[ImGuiCol_TitleBgActive] = accentColor;
   style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
   style.Colors[ImGuiCol_Button] = accentColor;
   style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
}

void ImGuiManager::NewFrame()
{
   ImGui_ImplWGPU_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
}

void ImGuiManager::EndFrame(RenderPassEncoder* encoder)
{
   ImGui::Render();
   ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), encoder->Get());
}

void ImGuiManager::RenderUI()
{
   ImGui::ShowDemoWindow();
}
