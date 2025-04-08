#pragma once

class CloudsModel
{
private:
   Device* _device;
   Queue* _queue;

   // R - channel: Cloud Coverage (Worley Noise)
   highp_u32vec2 _weatherMapTextureDimensions;
   Texture* _weatherMapTexture;

   highp_u32vec3 _baseNoiseTextureDimensions;
   Texture* _baseNoiseTexture;

public:
   CloudsModel(Device* device, Queue* queue);
   ~CloudsModel();

   Texture* GetWeatherMapTexture() const { return _weatherMapTexture; }
   Texture* GetBaseNoiseTexture() const { return _baseNoiseTexture; }
private:
   void GenerateWeatherMapTexture();
   void GenerateBaseNoiseTexture();
};