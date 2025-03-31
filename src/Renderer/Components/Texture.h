#pragma once

class Texture
{
private:
   WGPUTexture _texture;

public:
   Texture(Device* device, WGPUTextureDescriptor desc);
   ~Texture();

   WGPUTexture Get() const { return _texture; }
};
