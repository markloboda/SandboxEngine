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
   int width, height, channels;
   unsigned char *data = nullptr;
   if (!FileReader::LoadTexture2DData(filePath, &data, &width, &height, &channels, 4))
   {
      std::cerr << "Failed to load weather map texture data from file: " << filePath << std::endl;
      return;
   }

   _weatherMap = new CloudTextureData{data, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1, channels};
}

void CloudsModel::CreateNewLowFreqNoiseTexture(CloudTextureData *&noise)
{
   if (noise)
   {
      delete noise;
      noise = nullptr;
   }

   // Base cloud density
   // Generate a low frequency Perlin Worlye (Perlin - Worley) noise and
   // a low frequency Worley noise
   u32vec3 dim = {128u, 128u, 128u};
   const FastNoiseLite perlinNoise = GetPerlinNoiseGenerator(0, 0.046f);
   const FastNoiseLite worleyNoise = GetWorleyNoiseGenerator(1, 0.012f);

   const FastNoiseLite layeredWorleyNoise0 = GetWorleyNoiseGenerator(2, 0.046f);
   const FastNoiseLite layeredWorleyNoise1 = GetWorleyNoiseGenerator(3, 0.13f);
   const FastNoiseLite layeredWorleyNoise2 = GetWorleyNoiseGenerator(4, 0.18f);

   // Generate the 3D noise texture
   uint8_t *data = new uint8_t[dim.x * dim.y * dim.z * 4];
   for (uint32_t z = 0; z < dim.z; ++z)
   {
      for (uint32_t y = 0; y < dim.y; ++y)
      {
         for (uint32_t x = 0; x < dim.x; ++x)
         {
            float perlinWorley = 0.0; {
               float perlin = Math::Remap(perlinNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               float worley = Math::Remap(worleyNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               perlinWorley = Math::Clamp(perlin * (1.0f - worley), 0.0f, 1.0f);
            }
            float layeredWorley0 = Math::ClampRemap(layeredWorleyNoise0.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                                                    -1.0f, 1.0f, 0.0f, 1.0f);
            float layeredWorley1 = Math::ClampRemap(layeredWorleyNoise1.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                                                    -1.0f, 1.0f, 0.0f, 1.0f);
            float layeredWorley2 = Math::ClampRemap(layeredWorleyNoise2.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                                                    -1.0f, 1.0f, 0.0f, 1.0f);

            uint32_t idx = (z * dim.x * dim.y + y * dim.x + x) * 4;
            data[idx + 0] = static_cast<uint8_t>(perlinWorley * 255.0f); // R channel
            data[idx + 1] = static_cast<uint8_t>((1.0f - layeredWorley0) * 255.0f); // G channel
            data[idx + 2] = static_cast<uint8_t>((1.0f - layeredWorley1) * 255.0f); // B channel
            data[idx + 3] = static_cast<uint8_t>((1.0f - layeredWorley2) * 255.0f); // A channel
         }
      }
   }

   noise = new CloudTextureData{data, dim.x, dim.y, dim.z, 4};
}

void CloudsModel::GenerateBaseHighFreqNoiseTexture(CloudTextureData *&noise)
{
   if (noise)
   {
      delete noise;
      noise = nullptr;
   }

   // Base cloud density
   // Generate a high frequency worley noise at increasing frequencies
   u32vec3 dim = {32u, 32u, 32u};
   const FastNoiseLite layeredWorleyNoise0 = GetWorleyNoiseGenerator(5, 0.18f);
   const FastNoiseLite layeredWorleyNoise1 = GetWorleyNoiseGenerator(6, 0.28f);
   const FastNoiseLite layeredWorleyNoise2 = GetWorleyNoiseGenerator(7, 0.38f);

   // Generate the 3D noise texture
   uint8_t *data = new uint8_t[dim.x * dim.y * dim.z * 4];
   for (uint32_t z = 0; z < dim.z; ++z)
   {
      for (uint32_t y = 0; y < dim.y; ++y)
      {
         for (uint32_t x = 0; x < dim.x; ++x)
         {
            float layeredWorley0 = Math::ClampRemap(layeredWorleyNoise0.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                                                    -1.0f, 1.0f, 0.0f, 1.0f);
            float layeredWorley1 = Math::ClampRemap(layeredWorleyNoise1.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                                                    -1.0f, 1.0f, 0.0f, 1.0f);
            float layeredWorley2 = Math::ClampRemap(layeredWorleyNoise2.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                                                    -1.0f, 1.0f, 0.0f, 1.0f);

            uint32_t idx = (z * dim.x * dim.y + y * dim.x + x) * 4;
            data[idx + 0] = static_cast<uint8_t>((1.0f - layeredWorley0) * 255.0f); // R channel
            data[idx + 1] = static_cast<uint8_t>((1.0f - layeredWorley1) * 255.0f); // G channel
            data[idx + 2] = static_cast<uint8_t>((1.0f - layeredWorley2) * 255.0f); // B channel
            data[idx + 3] = 0; // A channel
         }
      }
   }

   noise = new CloudTextureData{data, dim.x, dim.y, dim.z, 4};
}
