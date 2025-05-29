#include <pch.h>

void WeatherSystem::Update(float dt)
{
   constexpr vec3 windOffset = vec3(0.0f, 0.01f, 0.0f);

   State.sunDirection = GetSunDirection(Options.timeOfDay);
   State.detailNoiseOffset += Options.windSpeed * dt * windOffset;

   // Update current parameters of renderers
   FeedData();
}

void WeatherSystem::FeedData() const
{
   vec3 sunDirection = GetSunDirection(Options.timeOfDay);

   CloudRenderer &cloudRenderer = Application::GetInstance().GetRuntime().GetRenderer().GetCloudRenderer();
   cloudRenderer.Weather.sunDirection = sunDirection;
   cloudRenderer.Weather.detailNoiseOffset = State.detailNoiseOffset;

   AtmosphereRenderer &atmosphereRenderer = Application::GetInstance().GetRuntime().GetRenderer().GetAtmosphereRenderer();
   atmosphereRenderer.Weather.sunDirection = sunDirection;
}


