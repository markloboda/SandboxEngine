#include <pch.h>
#include <Renderer/Clouds/CloudModel.h>

#include <FastNoiseLite.h>

CloudsModel::CloudsModel(Device* device, Queue* queue) :
   _device(device),
   _queue(queue)
{
   GenerateWeatherMapTexture();
   GenerateBaseNoiseTexture();
}

CloudsModel::~CloudsModel()
{
   delete _baseNoiseTexture;
   delete _weatherMapTexture;
}

void CloudsModel::GenerateWeatherMapTexture()
{
   _weatherMapTextureDimensions = { 128u, 128u };

   FastNoiseLite noise;
   // R-channel (Worley noise)
   noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Cellular);
   noise.SetSeed(0);
   noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Euclidean);
   noise.SetCellularReturnType(FastNoiseLite::CellularReturnType::CellularReturnType_CellValue);
   noise.SetCellularJitter(0.5f);
   noise.SetFrequency(0.1f);
   noise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
   noise.SetFractalOctaves(4);

   // Generate the noise texture
   std::vector<uint32_t> noiseData(_weatherMapTextureDimensions.x * _weatherMapTextureDimensions.y);
   for (uint32_t y = 0; y < _weatherMapTextureDimensions.y; ++y)
   {
      for (uint32_t x = 0; x < _weatherMapTextureDimensions.x; ++x)
      {
         float noiseValue = noise.GetNoise((float)x, (float)y);
         noiseData[y * _weatherMapTextureDimensions.x + x] = static_cast<uint32_t>((noiseValue + 1.0f) * 127.5f);
      }
   }

   // Create the texture
   WGPUTextureDescriptor textureDesc = {};
   textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
   textureDesc.dimension = WGPUTextureDimension_2D;
   textureDesc.size.width = _weatherMapTextureDimensions.x;
   textureDesc.size.height = _weatherMapTextureDimensions.y;
   textureDesc.size.depthOrArrayLayers = 1;
   textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
   textureDesc.mipLevelCount = 1;
   textureDesc.sampleCount = 1;
   _weatherMapTexture = new Texture(_device, &textureDesc);
   if (!_weatherMapTexture->IsValid())
   {
      std::cerr << "Failed to create weather map texture." << std::endl;
      return;
   }

   // Upload data to the texture
   WGPUExtent3D copyExtent = { _weatherMapTextureDimensions.x, _weatherMapTextureDimensions.y, 1 };
   _weatherMapTexture->UploadData(_queue, noiseData.data(), noiseData.size() * sizeof(uint32_t), &copyExtent);
}

void CloudsModel::GenerateBaseNoiseTexture()
{
   _baseNoiseTextureDimensions = { 128u, 128u, 128u };

   FastNoiseLite noise;

   // R-channel (Perlin-Worley noise)
   noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
   noise.SetSeed(0);
   noise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);

   noise.SetFrequency(0.1f);

   // Generate the noise texture
   std::vector<uint32_t> noiseData(_baseNoiseTextureDimensions.x * _baseNoiseTextureDimensions.y * _baseNoiseTextureDimensions.z);
   for (uint32_t z = 0; z < _baseNoiseTextureDimensions.z; ++z)
   {
      for (uint32_t y = 0; y < _baseNoiseTextureDimensions.y; ++y)
      {
         for (uint32_t x = 0; x < _baseNoiseTextureDimensions.x; ++x)
         {
            float noiseValue = noise.GetNoise((float)x, (float)y, (float)z);
            noiseData[(z * _baseNoiseTextureDimensions.y + y) * _baseNoiseTextureDimensions.x + x] = static_cast<uint32_t>((noiseValue + 1.0f) * 127.5f);
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
   _baseNoiseTexture = new Texture(_device, &textureDesc);
   if (!_baseNoiseTexture->IsValid())
   {
      std::cerr << "Failed to create base noise texture." << std::endl;
      return;
   }

   // Upload data to the texture
   WGPUExtent3D copyExtent = { _baseNoiseTextureDimensions.x,_baseNoiseTextureDimensions.y,_baseNoiseTextureDimensions.z };
   _baseNoiseTexture->UploadData(_queue, noiseData.data(), noiseData.size() * sizeof(uint32_t), &copyExtent);
}
