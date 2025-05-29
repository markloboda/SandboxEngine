#pragma once

class Texture;
class Renderer;

class FileReader
{
public:
   // Get files in a directory.
   static std::vector<std::string> GetFilesInDirectory(const std::string &directoryPath, const std::string &extension = "");

   // Load a SPIR-V shader data.
   static std::vector<uint32_t> LoadSPIRV(const std::string &path);

   // Load data of a 2D texture.
   static bool LoadTexture2DData(const std::string &filePath, unsigned char **data, int *width, int *height, int *channels);
};