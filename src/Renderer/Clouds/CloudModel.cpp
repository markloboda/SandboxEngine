#include <pch.h>
#include <Renderer/Clouds/CloudModel.h>

#include <fbm/fbm.hpp>
#include <fbm/generate_normal_random.hpp>

CloudsModel::CloudsModel()
{
   GenerateNoiseTexture(128, 128, 128);
}

CloudsModel::~CloudsModel()
{}


void CloudsModel::GenerateNoiseTexture(uint32_t texWidth, uint32_t texHeight, uint32_t texDepth)
{
   _texWidth = texWidth;
   _texHeight = texHeight;
   _texDepth = texDepth;

   std::vector<double> result = fbm::fractional_brownian_bridge_3d(0.5, texWidth, fbm::generate_normal_random(texWidth * texHeight * texDepth), false);

   // Convert double to float
   _cloudFBM = std::vector<float>(result.size());
   for (size_t i = 0; i < result.size(); ++i)
   {
      _cloudFBM[i] = static_cast<float>(result[i]);
   }
}
