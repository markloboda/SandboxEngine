#include <pch.h>

void WeatherSystem::Update(float /*dt*/)
{
   // Update current parameters of renderers
   FeedData();
}

void WeatherSystem::FeedData() const
{
   vec3 sunDirection = GetSunDirection(CurrentState.timeOfDay);

   CloudRenderer &cloudRenderer = Application::GetInstance().GetRuntime().GetRenderer().GetCloudRenderer();
   cloudRenderer.Weather.sunDirection = sunDirection;

   AtmosphereRenderer &atmosphereRenderer = Application::GetInstance().GetRuntime().GetRenderer().GetAtmosphereRenderer();
   atmosphereRenderer.Weather.sunDirection = sunDirection;
}


