#include <pch.h>
#include <Renderer/UI/ImGuiManager.h>
#include <Renderer/UI/UIRenderer.h>

// Define the static member
ImGuiManager* ImGuiManager::_instance = nullptr;

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

bool ImGuiManager::CreateInstance(GLFWwindow* window)
{
   if (_instance != nullptr)
   {
      throw std::runtime_error("ImGuiManager instance already exists.");
   }
   _instance = new ImGuiManager(window);
   return true;
}

ImGuiManager& ImGuiManager::GetInstance()
{
   if (_instance == nullptr)
   {
      throw std::runtime_error("ImGuiManager instance has not been created.");
   }

   return *_instance;
}

void ImGuiManager::DestroyInstance()
{
   delete _instance;
   _instance = nullptr;
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

void ImGuiManager::Render(CommandEncoder* encoder, TextureView* surfaceTextureView)
{
   // Render pass
   WGPURenderPassDescriptor renderPassDesc = {};
   renderPassDesc.colorAttachmentCount = 1;
   WGPURenderPassColorAttachment renderPassColorAttachment = {};
   renderPassColorAttachment.view = surfaceTextureView->Get();
   renderPassColorAttachment.loadOp = WGPULoadOp_Load;
   renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
   renderPassDesc.colorAttachments = &renderPassColorAttachment;
   RenderPassEncoder renderPassEncoder = RenderPassEncoder(encoder, &renderPassDesc);

   GetInstance().NewFrame();
   GetInstance().RenderUI();
   GetInstance().EndFrame(&renderPassEncoder);

   renderPassEncoder.EndPass();
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
   for (UIRenderer* uiRenderer : _uiRenderers)
   {
      uiRenderer->RenderImGuiUI();
   }
}

void ImGuiManager::AddUIRenderer(UIRenderer* uiRenderer)
{
   _uiRenderers.push_back(uiRenderer);
}

bool ImGuiManager::RemoveUIRenderer(UIRenderer* uiRenderer)
{
   auto it = std::find(_uiRenderers.begin(), _uiRenderers.end(), uiRenderer);
   if (it != _uiRenderers.end())
   {
      _uiRenderers.erase(it);
      return true;
   }
   return false;
}
