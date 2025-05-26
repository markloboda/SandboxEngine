#pragma once

class CloudsModel
{
private:
   highp_u32vec2 _weatherMapTextureDimensions;
   Texture* _weatherMapTexture;

   highp_u32vec3 _baseLowFreqNoiseTextureDimensions;
   Texture *_baseLowFreqNoiseTexture;

   highp_u32vec3 _baseHighFreqNoiseTextureDimensions;
   Texture *_baseHighFreqNoiseTexture;

public:
   explicit CloudsModel(Renderer &renderer);
   ~CloudsModel();

   void LoadWeatherMapTexture(Renderer &renderer, const std::string &filePath);
   [[nodiscard]] Texture &GetWeatherMapTexture() const { return *_weatherMapTexture; }
   [[nodiscard]] Texture &GetBaseLowFreqNoiseTexture() const { return *_baseLowFreqNoiseTexture; }
   [[nodiscard]] Texture &GetBaseHighFreqNoiseTexture() const { return *_baseHighFreqNoiseTexture; }

private:
   void GenerateWeatherMapTexture(Renderer &renderer);
   void GenerateBaseLowFreqNoiseTexture(Renderer &renderer);
   void GenerateBaseHighFreqNoiseTexture(Renderer &renderer);
};
