#pragma once

class CloudsModel
{
public:
   struct CloudTextureData
   {
      uint8_t *data;
      uint32_t width;
      uint32_t height;
      uint32_t depth;
      int channels;

      ~CloudTextureData()
      {
         delete[] data;
      }
   };

private:
   CloudTextureData *_weatherMap;

public:
   explicit CloudsModel();
   ~CloudsModel();

   void LoadWeatherMapTexture(const std::string &filePath);

   [[nodiscard]] const CloudTextureData &GetWeatherMapTexture() const { return *_weatherMap; }

   static void CreateNewLowFreqNoiseTexture(CloudTextureData *&noise);
   static void GenerateBaseHighFreqNoiseTexture(CloudTextureData *&noise);
};
