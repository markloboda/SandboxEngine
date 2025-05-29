#pragma once

class Texture;
class Renderer;

class FileReader
{
public:
   // Get files in a directory.
   static std::vector<std::string> GetFilesInDirectory(const std::string &directoryPath, const std::string &extension = "");

   // Check if file exists.
   static bool FileExists(const std::string &filePath);

   // Check if directory exists.
   static bool DirectoryExists(const std::string &directoryPath);

   // Create directory if it does not exist.
   static void CreateDirectory(const std::string &directoryPath);

   // Ensure the directory exists, creating it if necessary.
   static void EnsureDirectoryExists(const std::string &directoryPath);

   // Load a SPIR-V shader data.
   static std::vector<uint32_t> LoadSPIRV(const std::string &path);

   // Load data of a 2D texture.
   static bool LoadTexture2D(const std::string &filePath, unsigned char **data, uint32_t *width, uint32_t *height, uint32_t *channels, uint32_t desiredChannels = 0);

   // Save a 2D texture to a file.
   static bool SaveTexture2D(const std::string &filePath, const unsigned char *data, uint32_t width, uint32_t height, uint32_t channels);

   // Save a 3D texture to a file.
   static bool SaveTexture3D(const std::string &filePath, const uint8_t *data, uint32_t width, uint32_t height, uint32_t depth, uint32_t channels);

   // Load a 3D texture from a file.
   static bool LoadTexture3D(const std::string &filePath, uint8_t **data, uint32_t *width, uint32_t *height, uint32_t *depth, uint32_t *channels);
};