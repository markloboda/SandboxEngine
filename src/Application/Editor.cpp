#include <pch.h>

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

   Runtime &runtime = Application::GetInstance().GetRuntime();
   Renderer &renderer = runtime.GetRenderer();

   // Editor settings
   {
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(250, static_cast<float>(windowHeight) / 2.0f), ImGuiCond_FirstUseEver);

      ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

      ImGui::Text("Editor Settings");
      ImGui::Separator();

      ImGui::Checkbox("Show Grid", &renderer.RenderGrid);
      ImGui::Separator();
      ImGui::Separator();

      ImGui::Checkbox("Show Atmosphere", &renderer.RenderAtmosphere);
      ImGui::Separator();

      ImGui::Checkbox("Render Clouds", &renderer.RenderClouds);
      ImGui::Separator();
      ImGui::Separator();

      ImGui::Checkbox("Show Cloths", &renderer.RenderCloth);
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

   // Weather
   {
      ImGui::Begin("Weather", nullptr); {
         WeatherSystem::WeatherOptions &options = runtime.GetWeatherSystem().Options;

         // Weather map selection
         {
            std::vector<std::string> weatherMaps = WeatherSystem::GetAvailableWeatherMaps();
            std::string currentWeatherMap = runtime.GetWeatherSystem().GetCurrentWeatherMap();
            if (ImGui::BeginCombo("Weather Map", currentWeatherMap.c_str()))
            {
               for (int i = 0; i < weatherMaps.size(); ++i)
               {
                  bool isSelected = (currentWeatherMap == weatherMaps[i]);
                  if (ImGui::Selectable(weatherMaps[i].c_str(), isSelected))
                  {
                     runtime.GetWeatherSystem().ChangeWeatherMap(weatherMaps[i]);
                  }
                  if (isSelected)
                     ImGui::SetItemDefaultFocus();
               }
               ImGui::EndCombo();
            }
         }

         ImGui::SliderFloat("Time of Day", &options.timeOfDay, 0.0f, 24.0f, "%.1f h");
         ImGui::SliderFloat("Wind Speed", &options.windSpeed, 0.0f, 50.0f, "%.1f m/s");
      }
      ImGui::End();
   }

   // Clouds
   {
      if (renderer.RenderClouds)
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
               // Sliders for cloud settings
               ImGui::Text("Densities");
               ImGui::SliderFloat("Coverage Multiplier", &settings.coverageMultiplier, 0.0f, 10.0f);
               ImGui::SliderFloat("Density Multiplier", &settings.densityMultiplier, 0.0f, 10.0f);
               ImGui::SliderFloat("High Frequency Threshold", &settings.highFreqThreshold, 0.0f, 1.0f);
               ImGui::SliderFloat("Detail Blend", &settings.detailBlendStrength, 0.0f, 1.0f, "%.2f");

               ImGui::Separator();

               ImGui::Text("Lighting");
               ImGui::SliderFloat("Ambient Light", &settings.ambientLight, 0.0f, 1.0f);
               ImGui::SliderFloat("Light Absorption", &settings.lightAbsorption, 0.1f, 2.0f);
               ImGui::SliderFloat("Henyey Greenstein", &settings.henyeyGreensteinStrength, 0.0f, 1.0f, "%.1f");
               ImGui::SliderFloat("Phase Eccentricity", &settings.phaseEccentricity, -1.0f, 1.0f);
               ImGui::SliderFloat("Light Ray Cone Angle", &settings.lightRayConeAngle, 0.0f, glm::pi<float>() / 2.0f, "%.1f rad");
               ImGui::SliderFloat("Multiple Scattering", &settings.multipleScatteringStrength, 0.0f, 1.0f, "%.1f");

               ImGui::Separator();

               ImGui::Text("Textures");
               ImGui::SliderFloat("Low Freq Texture Scale", &settings.lowFreqTextureScale, 0.1f, 10.0f, "%.1f");
               ImGui::SliderFloat("High Freq Texture Scale", &settings.highFreqTextureScale, 0.1f, 10.0f, "%.1f");

               ImGui::Separator();

               ImGui::Text("Post Processing");
               ImGui::SliderFloat("Tone Mapping", &settings.toneMappingStrength, 0.0f, 1.0f, "%.01f");
               ImGui::SliderFloat("Contrast Gamma", &settings.contrastGamma, 0.1f, 5.0f, "%.01f");
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
