#pragma once

class CloudsModel
{
private:
   // R - channel: Cloud Coverage (Worley Noise)
   highp_u32vec2 _weatherMapTextureDimensions;
   Texture* _weatherMapTexture;

   // R - channel: Cloud Density (Perlin Noise)
   highp_u32vec3 _baseNoiseTextureDimensions;
   Texture* _baseNoiseTexture;

public:
   CloudsModel(Renderer* renderer);
   ~CloudsModel();

   Texture* GetWeatherMapTexture() const { return _weatherMapTexture; }
   Texture* GetBaseNoiseTexture() const { return _baseNoiseTexture; }
private:
   void GenerateWeatherMapTexture(Renderer* renderer);
   void GenerateBaseNoiseTexture(Renderer* renderer);
};