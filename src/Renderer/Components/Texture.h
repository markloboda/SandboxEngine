#pragma once

class Texture
{
private:
   WGPUTexture _texture;
   WGPUTextureFormat _format;

public:
   Texture(Device* device, WGPUTextureDescriptor* desc);
   ~Texture();

   WGPUTexture Get() const { return _texture; }
   bool IsValid() const { return _texture != nullptr; }
   WGPUTextureFormat GetFormat() const { return _format; }
};
