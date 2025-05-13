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
   FastNoiseLite lowFrequencyPerlinNoise;
   lowFrequencyPerlinNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
   lowFrequencyPerlinNoise.SetSeed(0);
   lowFrequencyPerlinNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
   lowFrequencyPerlinNoise.SetFrequency(0.1f);

   FastNoiseLite lowFrequencyWorleyNoise;
   lowFrequencyWorleyNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Cellular);
   lowFrequencyWorleyNoise.SetSeed(0);
   lowFrequencyWorleyNoise.SetFrequency(0.1f);

   // Generate the 3D noise texture
   std::vector<uint8_t> noiseData(_baseNoiseTextureDimensions.x * _baseNoiseTextureDimensions.y * _baseNoiseTextureDimensions.z * 4);
   for (uint32_t z = 0; z < _baseNoiseTextureDimensions.z; ++z)
   {
      for (uint32_t y = 0; y < _baseNoiseTextureDimensions.y; ++y)
      {
         for (uint32_t x = 0; x < _baseNoiseTextureDimensions.x; ++x)
         {
            float perlin = lowFrequencyPerlinNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
            float worley = lowFrequencyWorleyNoise.GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
            float invWorley = 1.0f - worley;

            float perlinWorley = perlin - invWorley;

            uint32_t idx = (z * _baseNoiseTextureDimensions.x * _baseNoiseTextureDimensions.y + y * _baseNoiseTextureDimensions.x + x) * 4;
            noiseData[idx + 0] = static_cast<uint8_t>(perlinWorley * 255.0f); // R channel
            noiseData[idx + 1] = 0; // G channel
            noiseData[idx + 2] = 0; // B channel
            noiseData[idx + 3] = 0; // A channel
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
