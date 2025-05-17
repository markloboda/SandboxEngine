#include <pch.h>
#include <Renderer/Sky/CloudModel.h>
#include <Utils/FileReader.h>

#include <FastNoiseLite.h>

CloudsModel::CloudsModel(Renderer *renderer)
{
   LoadWeatherMapTexture(renderer, "assets/weather.png");
   GenerateBaseNoiseTexture(renderer);
}

CloudsModel::~CloudsModel()
{
   delete _baseNoiseTexture;
   delete _weatherMapTexture;
}

void CloudsModel::LoadWeatherMapTexture(Renderer *renderer, const std::string &filePath)
{
   int width, height;
   _weatherMapTexture = FileReader::LoadTexture2D(renderer, filePath, &width, &height);
   _weatherMapTextureDimensions = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
   if (!_weatherMapTexture)
   {
      std::cerr << "Failed to load weather map texture from file: " << filePath << std::endl;
      return;
   }
}

void CloudsModel::GenerateWeatherMapTexture(Renderer *)
{
}

void CloudsModel::GenerateBaseNoiseTexture(Renderer *renderer)
{
   _baseNoiseTextureDimensions = {128u, 128u, 128u};

   // Base cloud density
   // Generate a low frequency Perlin Worlye (Perlin - Worley) noise and
   // a low frequency Worley noise
   FastNoiseLite perlinNoise;
   perlinNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
   perlinNoise.SetSeed(0);
   perlinNoise.SetFrequency(0.046f);
   perlinNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
   perlinNoise.SetFractalOctaves(3);
   perlinNoise.SetFractalLacunarity(2.88f);
   perlinNoise.SetFractalGain(0.36f);
   perlinNoise.SetFractalWeightedStrength(0.0f);

   FastNoiseLite worleyNoise;
   worleyNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Cellular);
   worleyNoise.SetSeed(0);
   worleyNoise.SetFrequency(0.075f);
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

   FastNoiseLite highFreqWorleyNoise1;
   highFreqWorleyNoise1.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Cellular);
   highFreqWorleyNoise1.SetSeed(512);
   highFreqWorleyNoise1.SetFrequency(0.13f);
   highFreqWorleyNoise1.SetFractalType(FastNoiseLite::FractalType::FractalType_Ridged);
   highFreqWorleyNoise1.SetFractalOctaves(3);
   highFreqWorleyNoise1.SetFractalLacunarity(2.68f);
   highFreqWorleyNoise1.SetFractalGain(0.3f);
   highFreqWorleyNoise1.SetFractalWeightedStrength(0.1f);
   highFreqWorleyNoise1.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq);
   highFreqWorleyNoise1.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_Distance);
   highFreqWorleyNoise1.SetCellularJitter(1.6f);
   highFreqWorleyNoise1.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
   highFreqWorleyNoise1.SetDomainWarpAmp(1.0f);

   FastNoiseLite highFreqWorleyNoise2;
   highFreqWorleyNoise2.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Cellular);
   highFreqWorleyNoise2.SetSeed(124);
   highFreqWorleyNoise2.SetFrequency(0.18f);
   highFreqWorleyNoise2.SetFractalType(FastNoiseLite::FractalType::FractalType_Ridged);
   highFreqWorleyNoise2.SetFractalOctaves(3);
   highFreqWorleyNoise2.SetFractalLacunarity(2.68f);
   highFreqWorleyNoise2.SetFractalGain(0.3f);
   highFreqWorleyNoise2.SetFractalWeightedStrength(0.1f);
   highFreqWorleyNoise2.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq);
   highFreqWorleyNoise2.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_Distance);
   highFreqWorleyNoise2.SetCellularJitter(1.6f);
   highFreqWorleyNoise2.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
   highFreqWorleyNoise2.SetDomainWarpAmp(1.0f);

   // Generate the 3D noise texture
   std::vector<uint8_t> noiseData(_baseNoiseTextureDimensions.x * _baseNoiseTextureDimensions.y * _baseNoiseTextureDimensions.z * 4);
   for (uint32_t z = 0; z < _baseNoiseTextureDimensions.z; ++z)
   {
      for (uint32_t y = 0; y < _baseNoiseTextureDimensions.y; ++y)
      {
         for (uint32_t x = 0; x < _baseNoiseTextureDimensions.x; ++x)
         {
            float perlinWorley = 0.0; {
               float perlin = Math::Remap(perlinNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               // float worley = Math::Remap(worleyNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), -1.0f, 1.0f, 0.0f, 1.0f);
               perlinWorley = Math::Clamp(perlin, 0.0f, 1.0f);
            }
            float worley = Math::Clamp(
               Math::Remap(worleyNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                           -1.0f, 1.0f, 0.0f, 1.0f),
               0.0f, 1.0f);
            float highFreqWorley1 = Math::Clamp(
               Math::Remap(highFreqWorleyNoise1.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                           -1.0f, 1.0f, 0.0f, 1.0f),
               0.0f, 1.0f);
            float highFreqWorley2 = Math::Clamp(
               Math::Remap(highFreqWorleyNoise2.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)),
                           -1.0f, 1.0f, 0.0f, 1.0f),
               0.0f, 1.0f);

            uint32_t idx = (z * _baseNoiseTextureDimensions.x * _baseNoiseTextureDimensions.y + y * _baseNoiseTextureDimensions.x + x) * 4;
            noiseData[idx + 0] = static_cast<uint8_t>(perlinWorley * 255.0f); // R channel
            noiseData[idx + 1] = static_cast<uint8_t>(1.0f - worley * 255.0f); // G channel
            noiseData[idx + 2] = static_cast<uint8_t>(1.0f - highFreqWorley1 * 255.0f); // B channel
            noiseData[idx + 3] = static_cast<uint8_t>(1.0f - highFreqWorley2 * 255.0f); // A channel
         }
      }
   }

   // Create the texture
   WGPUTextureDescriptor textureDesc = {};
   textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
   textureDesc.dimension = WGPUTextureDimension_3D;
   textureDesc.size.width = _baseNoiseTextureDimensions.x;
   textureDesc.size.height = _baseNoiseTextureDimensions.y;
   textureDesc.size.depthOrArrayLayers = _baseNoiseTextureDimensions.z;
   textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
   textureDesc.mipLevelCount = 1;
   textureDesc.sampleCount = 1;
   _baseNoiseTexture = new Texture(renderer->GetDevice(), &textureDesc);
   if (!_baseNoiseTexture->IsValid())
   {
      std::cerr << "Failed to create base noise texture." << std::endl;
      return;
   }

   // Upload data to the texture
   WGPUExtent3D copyExtent = {_baseNoiseTextureDimensions.x, _baseNoiseTextureDimensions.y, _baseNoiseTextureDimensions.z};
   renderer->UploadTextureData(_baseNoiseTexture, noiseData.data(), noiseData.size() * sizeof(uint8_t), &copyExtent);
}
