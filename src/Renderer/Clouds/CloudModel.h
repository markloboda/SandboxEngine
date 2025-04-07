#pragma once

class CloudsModel
{
private:
   uint32_t _texWidth;
   uint32_t _texHeight;
   uint32_t _texDepth;

   // Data
   std::vector<float> _cloudFBM;

public:
   CloudsModel();
   ~CloudsModel();

   const float* GetCloudFBM() const { return _cloudFBM.data(); }
   size_t GetCloudFBMSize() const { return _cloudFBM.size() * sizeof(float); }

   vec3 GetTextureDimensions() const { return vec3(_texWidth, _texHeight, _texDepth); }
private:
   void GenerateNoiseTexture(uint32_t texWidth, uint32_t texHeight, uint32_t texDepth);
};