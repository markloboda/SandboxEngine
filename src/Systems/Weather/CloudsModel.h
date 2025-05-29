#pragma once

class CloudsModel
{
public:
   struct CloudTextureData
   {
      const uint8_t *data;
      const uint32_t width;
      const uint32_t height;
      const uint32_t depth;
      const int channels;
      const size_t hash;

      CloudTextureData(unsigned char *data, uint32_t width, uint32_t height, uint32_t depth, int channels):
         data(data), width(width), height(height), depth(depth), channels(channels), hash(ComputeHash()) {}

      ~CloudTextureData()
      {
         delete[] data;
      }

   private:
      size_t ComputeHash() const
      {
         size_t h = HashUtils::HashFields(width, height, depth, channels);
         const size_t size = static_cast<size_t>(width) * height * depth * channels;
         HashUtils::HashCombine(h, HashUtils::HashBuffer(data, size));
         return h;
      }
   };

private:
   CloudTextureData *_weatherMap = nullptr;

public:
   ~CloudsModel();

   void LoadWeatherMapTexture(const std::string &filePath);

   [[nodiscard]] const CloudTextureData &GetWeatherMapTexture() const { return *_weatherMap; }

   static void CreateNewLowFreqNoiseTexture(CloudTextureData *&noise);
   static void GenerateBaseHighFreqNoiseTexture(CloudTextureData *&noise);
};
