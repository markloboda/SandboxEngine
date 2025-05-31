#pragma once

class WeatherSystem
{
public:
   struct WeatherOptions
   {
      float timeOfDay = 11.2f; // Time of day in hours (0.0 - 24.0)s
      float windSpeed = 3.3f; // Wind speed in m/s
   };

   struct WeatherState
   {
      vec3 sunDirection;
      vec3 detailNoiseOffset;
   };

   WeatherOptions Options = {};
   WeatherState State = {};
   std::string _currentWeatherMapPath;
   CloudsModel Model;

private:

public:
   explicit WeatherSystem();
   ~WeatherSystem();

   void Update(float dt);
   void FeedData() const;

   static std::vector<std::string> GetAvailableWeatherMaps() { return FileReader::GetFilesInDirectory("assets/weather", "png"); }
   const std::string &GetCurrentWeatherMap() const { return _currentWeatherMapPath; }
   void ChangeWeatherMap(const std::string &filePath);

public:
   static vec3 GetSunDirection(float timeOfDay)
   {
      float solarAngle = (timeOfDay / 24.0f) * 2.0f * PI - PI; // -π at midnight, 0 at noon, π at next midnight
      float elevation = radians(45.0f); // approximate sun elevation (change based on season/lat)

      return normalize(vec3(
         cos(solarAngle) * cos(elevation),
         sin(elevation),
         sin(solarAngle) * cos(elevation)
      ));
   }
};
