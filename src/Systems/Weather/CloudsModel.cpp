#include <pch.h>

#include <FastNoiseLite/FastNoiseLite.h>

FastNoiseLite GetPerlinNoiseGenerator(int seed = 0, float frequency = 0.1f)
{
   FastNoiseLite perlinNoise;
   perlinNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
   perlinNoise.SetSeed(seed);
   perlinNoise.SetFrequency(frequency);
   perlinNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
   perlinNoise.SetFractalOctaves(3);
   perlinNoise.SetFractalLacunarity(2.88f);
   perlinNoise.SetFractalGain(0.36f);
   perlinNoise.SetFractalWeightedStrength(0.0f);
   return perlinNoise;
}

FastNoiseLite GetWorleyNoiseGenerator(int seed = 0, float frequency = 0.1f)
{
   FastNoiseLite worleyNoise;
   worleyNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Cellular);
   worleyNoise.SetSeed(seed);
   worleyNoise.SetFrequency(frequency);
   worleyNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_Ridged);
   worleyNoise.SetFractalOctaves(3);
   worleyNoise.SetFractalLacunarity(2.68f);
   worleyNoise.SetFractalGain(0.3f);
   worleyNoise.SetFractalWeightedStrength(0.1f);
   worleyNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq);
   worleyNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_Distance);
   worleyNoise.SetCellularJitter(1.6f);
   worleyNoise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
   worleyNoise.SetDomainWarpAmp(1.0f);
   return worleyNoise;
}

CloudsModel::~CloudsModel()
{
   delete _weatherMap;
}

void CloudsModel::LoadWeatherMapTexture(const std::string &filePath)
{
   uint32_t width, height, channels;
   unsigned char *data = nullptr;
   if (!FileReader::LoadTexture2D(filePath, &data, &width, &height, &channels, 4))
   {
      std::cerr << "Failed to load weather map texture data from file: " << filePath << std::endl;
      return;
   }

   _weatherMap = new CloudTextureData{data, width, height, 1, channels};
}

void CloudsModel::CreateNewLowFreqNoiseTexture(CloudTextureData *&noise)
{
   if (noise)
   {
      delete noise;
      noise = nullptr;
   }

   const std::string filePath = "assets/noise/low_freq_cloud_noise.tex3d";
   uint8_t *data = nullptr;
   uint32_t width = 0, height = 0, depth = 0, channels = 0;

   if (FileReader::FileExists(filePath))
   {
      if (!FileReader::LoadTexture3D(filePath, &data, &width, &height, &depth, &channels))
      {
         std::cerr << "Failed to load low frequency noise texture from " << filePath << std::endl;
         return;
      }
   }
   else
   {
      u32vec3 dim = {128u, 128u, 128u};
      width = dim.x;
      height = dim.y;
      depth = dim.z;
      channels = 4;

      const FastNoiseLite pNoise = GetPerlinNoiseGenerator(0, 0.046f);
      const FastNoiseLite wNoise = GetWorleyNoiseGenerator(1, 0.012f);

      const FastNoiseLite w0Noise = GetWorleyNoiseGenerator(2, 0.046f);
      const FastNoiseLite w1Noise = GetWorleyNoiseGenerator(3, 0.13f);
      const FastNoiseLite w2Noise = GetWorleyNoiseGenerator(4, 0.18f);

      data = new uint8_t[width * height * depth * channels];

      for (uint32_t z = 0; z < depth; ++z)
      {
         for (uint32_t y = 0; y < height; ++y)
         {
            for (uint32_t x = 0; x < width; ++x)
            {
               float p = Math::Remap(pNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               float w = Math::Remap(wNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               float pw = Math::Clamp(p * (1.0f - w), 0.0f, 1.0f);

               float w0 = Math::ClampRemap(w0Noise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               float w1 = Math::ClampRemap(w1Noise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               float w2 = Math::ClampRemap(w2Noise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);

               uint32_t idx = (z * width * height + y * width + x) * channels;
               data[idx + 0] = static_cast<uint8_t>(pw * 255.0f);
               data[idx + 1] = static_cast<uint8_t>((1.0f - w0) * 255.0f);
               data[idx + 2] = static_cast<uint8_t>((1.0f - w1) * 255.0f);
               data[idx + 3] = static_cast<uint8_t>((1.0f - w2) * 255.0f);
            }
         }
      }

      FileReader::SaveTexture3D(filePath, data, width, height, depth, channels);
   }

   noise = new CloudTextureData{data, width, height, depth, channels};
}

void CloudsModel::GenerateBaseHighFreqNoiseTexture(CloudTextureData *&noise)
{
   if (noise)
   {
      delete noise;
      noise = nullptr;
   }

   const std::string filePath = "assets/noise/high_freq_cloud_noise.tex3d";
   uint8_t *data = nullptr;
   uint32_t width = 0, height = 0, depth = 0, channels = 0;

   if (FileReader::FileExists(filePath))
   {
      if (!FileReader::LoadTexture3D(filePath, &data, &width, &height, &depth, &channels))
      {
         std::cerr << "Failed to load high frequency noise texture from " << filePath << std::endl;
         return;
      }
   }
   else
   {
      u32vec3 dim = {32u, 32u, 32u};
      width = dim.x;
      height = dim.y;
      depth = dim.z;
      channels = 4;

      const FastNoiseLite w0Noise = GetWorleyNoiseGenerator(5, 0.18f);
      const FastNoiseLite w1Noise = GetWorleyNoiseGenerator(6, 0.28f);
      const FastNoiseLite w2Noise = GetWorleyNoiseGenerator(7, 0.38f);

      data = new uint8_t[width * height * depth * channels];

      for (uint32_t z = 0; z < depth; ++z)
      {
         for (uint32_t y = 0; y < height; ++y)
         {
            for (uint32_t x = 0; x < width; ++x)
            {
               float w0 = Math::ClampRemap(w0Noise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               float w1 = Math::ClampRemap(w1Noise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               float w2 = Math::ClampRemap(w2Noise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);

               uint32_t idx = (z * width * height + y * width + x) * channels;
               data[idx + 0] = static_cast<uint8_t>((1.0f - w0) * 255.0f);
               data[idx + 1] = static_cast<uint8_t>((1.0f - w1) * 255.0f);
               data[idx + 2] = static_cast<uint8_t>((1.0f - w2) * 255.0f);
               data[idx + 3] = 0;
            }
         }
      }

      FileReader::SaveTexture3D(filePath, data, width, height, depth, channels);
   }

   noise = new CloudTextureData{data, width, height, depth, channels};
}
