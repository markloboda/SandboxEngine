#include <pch.h>
#include <Renderer/Sky/AtmosphereRenderer.h>

AtmosphereRenderer::AtmosphereRenderer(Renderer *renderer)
{
   bool success = Initialize(renderer);
   assert(success);
}

AtmosphereRenderer::~AtmosphereRenderer()
{
   Terminate();
}

bool AtmosphereRenderer::Initialize(Renderer *renderer)
{
   Device *device = renderer->GetDevice();

   // Shader modules
   ShaderModule fragmentShader = ShaderModule::LoadShaderModule(device, "atmosphere.frag");

   return true;
}

void AtmosphereRenderer::Terminate()
{
}

void AtmosphereRenderer::Render(Renderer *renderer, CommandEncoder *encoder, TextureView *surfaceTextureView)
{
}
