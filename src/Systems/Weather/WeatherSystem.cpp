#include <pch.h>

WeatherSystem::WeatherSystem():
   _currentWeatherMapPath(GetAvailableWeatherMaps()[0])
{
   ChangeWeatherMap(_currentWeatherMapPath);
}

WeatherSystem::~WeatherSystem()
{
}

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

void WeatherSystem::ChangeWeatherMap(const std::string &filePath)
{
   if (FileReader::FileExists(filePath))
   {
      _currentWeatherMapPath = filePath;
      Model.LoadWeatherMapTexture(_currentWeatherMapPath);
   }
   else
   {
      std::cerr << "WeatherSystem: Weather map file does not exist: " << filePath << std::endl;
   }
}


