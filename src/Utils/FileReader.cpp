#include <Utils/FileReader.h>

std::vector<uint32_t> FileReader::LoadSPIRV(const std::string& path)
{
   std::ifstream file(path, std::ios::binary | std::ios::ate);
   std::streamsize size = file.tellg();
   file.seekg(0, std::ios::beg);

   std::vector<uint32_t> buffer(size / sizeof(uint32_t));
   file.read(reinterpret_cast<char*>(buffer.data()), size);
   return buffer;
}
