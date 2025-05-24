#pragma once

class Texture
{
private:
   WGPUTexture _texture;
   WGPUTextureFormat _format;

public:
   Texture(const Device &device, const WGPUTextureDescriptor *desc);
   ~Texture();

   [[nodiscard]] WGPUTexture Get() const { return _texture; }
   [[nodiscard]] bool IsValid() const { return _texture != nullptr; }
   [[nodiscard]] WGPUTextureFormat GetFormat() const { return _format; }
};
