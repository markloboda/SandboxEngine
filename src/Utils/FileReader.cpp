#include <pch.h>

#include <stb/stb_image.h>

std::vector<uint32_t> FileReader::LoadSPIRV(const std::string &path)
{
   std::ifstream file(path, std::ios::binary | std::ios::ate);
   std::streamsize size = file.tellg();
   file.seekg(0, std::ios::beg);

   std::vector<uint32_t> buffer(size / sizeof(uint32_t));
   file.read(reinterpret_cast<char *>(buffer.data()), size);
   return buffer;
}

Texture *FileReader::LoadTexture2D(Renderer &renderer, const std::string &filePath, int *width, int *height)
{
   // Load the image using stb_image
   int x, y, channels;
   unsigned char *data = stbi_load(filePath.c_str(), &x, &y, &channels, 0);
   if (!data)
   {
      std::cerr << "Failed to load texture: " << filePath << std::endl;
      return nullptr;
   }

   // Create a texture descriptor
   WGPUTextureDescriptor textureDesc = {};
   textureDesc.label = {filePath.c_str(), WGPU_STRLEN};
   textureDesc.dimension = WGPUTextureDimension_2D;
   textureDesc.size.width = static_cast<uint32_t>(x);
   textureDesc.size.height = static_cast<uint32_t>(y);
   textureDesc.size.depthOrArrayLayers = 1;
   textureDesc.sampleCount = 1;
   textureDesc.mipLevelCount = 1;
   textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
   textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;

   // Create the texture
   Texture *tex = new Texture(renderer.GetDevice(), &textureDesc);
   if (!tex->IsValid())
   {
      std::cerr << "Failed to create texture: " << filePath << std::endl;
      stbi_image_free(data);
      return nullptr;
   }

   // Upload the image data to the texture
   WGPUExtent3D writeSize = {};
   writeSize.width = static_cast<uint32_t>(x);
   writeSize.height = static_cast<uint32_t>(y);
   writeSize.depthOrArrayLayers = 1;
   renderer.UploadTextureData(*tex, data, x * y * channels, &writeSize);

   // Free the image data
   stbi_image_free(data);

   *width = x;
   *height = y;
   return tex;
}
