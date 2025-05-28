#pragma once

class WeatherSystem
{
public:
   struct WeatherState
   {
      float timeOfDay = 12.0f; // Time of day in hours (0.0 - 24.0)
   };

   WeatherState CurrentState = {};

public:
   void Update(float dt);

private:
   void FeedData() const;

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
