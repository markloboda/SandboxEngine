#pragma once

class Texture;
class Renderer;

class FileReader
{
public:
   static std::vector<uint32_t> LoadSPIRV(const std::string &path);

   static Texture *LoadTexture2D(Renderer &renderer, const std::string &filePath, int *width, int *height);
};