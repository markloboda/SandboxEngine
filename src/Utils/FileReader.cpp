#include <pch.h>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

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

bool FileReader::FileExists(const std::string &filePath)
{
   return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

bool FileReader::DirectoryExists(const std::string &directoryPath)
{
   return std::filesystem::exists(directoryPath) && std::filesystem::is_directory(directoryPath);
}

void FileReader::CreateDirectory(const std::string &directoryPath)
{
   if (!DirectoryExists(directoryPath))
   {
      std::error_code ec;
      std::filesystem::create_directories(directoryPath, ec);
      if (ec)
      {
         std::cerr << "Failed to create directory: " << directoryPath << " - " << ec.message() << std::endl;
      }
   }
}

void FileReader::EnsureDirectoryExists(const std::string &directoryPath)
{
   CreateDirectory(directoryPath);
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

bool FileReader::LoadTexture2D(const std::string &filePath, unsigned char **data, uint32_t *width, uint32_t *height, uint32_t *channels, uint32_t desiredChannels)
{
   // Load image.
   int _width, _height, _channels;
   unsigned char *imageData = stbi_load(filePath.c_str(), &_width, &_height, &_channels, desiredChannels);
   if (!imageData)
   {
      std::cerr << "Failed to load texture data: " << filePath << std::endl;
      return false;
   }

   *width = static_cast<uint32_t>(_width);
   *height = static_cast<uint32_t>(_height);
   *channels = static_cast<uint32_t>(_channels);
   if (desiredChannels > 0)
   {
      *channels = desiredChannels;
   }

   // Copy data to output parameter.
   *data = new unsigned char[*width * *height * *channels];
   std::memcpy(*data, imageData, *width * *height * *channels);
   stbi_image_free(imageData);
   return true;
}

bool FileReader::SaveTexture2D(const std::string &filePath, const unsigned char *data, uint32_t width, uint32_t height, uint32_t channels)
{
   // Determine file extension
   std::string extension;
   size_t dotPos = filePath.find_last_of('.');
   if (dotPos != std::string::npos)
      extension = filePath.substr(dotPos + 1);

   int success = 0;
   if (extension == "png")
   {
      success = stbi_write_png(filePath.c_str(), width, height, channels, data, width * channels);
   }
   else if (extension == "jpg" || extension == "jpeg")
   {
      success = stbi_write_jpg(filePath.c_str(), width, height, channels, data, 90); // 90 = quality
   }
   else if (extension == "bmp")
   {
      success = stbi_write_bmp(filePath.c_str(), width, height, channels, data);
   }
   else if (extension == "tga")
   {
      success = stbi_write_tga(filePath.c_str(), width, height, channels, data);
   }
   else
   {
      std::cerr << "Unsupported image format for writing: " << filePath << std::endl;
      return false;
   }

   if (!success)
   {
      std::cerr << "Failed to write image to file: " << filePath << std::endl;
      return false;
   }

   return true;
}

bool FileReader::SaveTexture3D(const std::string &filePath, const uint8_t *data, uint32_t width, uint32_t height, uint32_t depth, uint32_t channels)
{
   // Ensure the directory exists.
   std::string directory = std::filesystem::path(filePath).parent_path().string();
   CreateDirectory(directory);

   std::ofstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      std::cerr << "Failed to open file for writing: " << filePath << std::endl;
      return false;
   }

   // Write header
   file.write(reinterpret_cast<const char *>(&width), sizeof(uint32_t));
   file.write(reinterpret_cast<const char *>(&height), sizeof(uint32_t));
   file.write(reinterpret_cast<const char *>(&depth), sizeof(uint32_t));
   file.write(reinterpret_cast<const char *>(&channels), sizeof(uint32_t));

   // Write pixel data
   const size_t dataSize = static_cast<size_t>(width) * height * depth * channels;
   file.write(reinterpret_cast<const char *>(data), dataSize);

   return true;
}

bool FileReader::LoadTexture3D(const std::string &filePath, uint8_t **data, uint32_t *width, uint32_t *height, uint32_t *depth, uint32_t *channels)
{
   std::ifstream file(filePath, std::ios::binary);
   if (!file.is_open())
   {
      std::cerr << "Failed to open file for reading: " << filePath << std::endl;
      return false;
   }

   // Read header
   file.read(reinterpret_cast<char *>(width), sizeof(uint32_t));
   file.read(reinterpret_cast<char *>(height), sizeof(uint32_t));
   file.read(reinterpret_cast<char *>(depth), sizeof(uint32_t));
   file.read(reinterpret_cast<char *>(channels), sizeof(uint32_t));

   // Allocate and read data
   const size_t dataSize = static_cast<size_t>(*width) * (*height) * (*depth) * (*channels);
   *data = new uint8_t[dataSize];
   file.read(reinterpret_cast<char *>(*data), dataSize);

   return true;
}

