#include <pch.h>

#include <stb/stb_image.h>

std::vector<std::string> FileReader::GetFilesInDirectory(const std::string &directoryPath, const std::string &extension)
{
   std::vector<std::string> files;
   for (const auto &entry: std::filesystem::directory_iterator(directoryPath))
   {
      if (entry.is_regular_file())
      {
         const std::string filePath = entry.path().string();
         if (extension.empty() || filePath.ends_with(extension))
         {
            files.push_back(filePath);
         }
      }
   }
   return files;
}

std::vector<uint32_t> FileReader::LoadSPIRV(const std::string &path)
{
   std::ifstream file(path, std::ios::binary | std::ios::ate);
   std::streamsize size = file.tellg();
   file.seekg(0, std::ios::beg);

   std::vector<uint32_t> buffer(size / sizeof(uint32_t));
   file.read(reinterpret_cast<char *>(buffer.data()), size);
   return buffer;
}

bool FileReader::LoadTexture2DData(const std::string &filePath, unsigned char **data, int *width, int *height, int *channels)
{
   // Load image.
   unsigned char *imageData = stbi_load(filePath.c_str(), width, height, channels, 0);
   if (!imageData)
   {
      std::cerr << "Failed to load texture data: " << filePath << std::endl;
      return false;
   }

   // Copy data to output parameter.
   *data = new unsigned char[*width * *height * *channels];
   std::memcpy(*data, imageData, *width * *height * *channels);
   stbi_image_free(imageData);
   return true;
}
