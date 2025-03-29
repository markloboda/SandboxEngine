#include <pch.h>
#include <Renderer/Clouds/CloudRenderer.h>

CloudRenderer::CloudRenderer()
{
   bool success = Initialize();
}

CloudRenderer::~CloudRenderer()
{
   Terminate();
}

bool CloudRenderer::Initialize()
{
   return true;
}

void CloudRenderer::Render(CommandEncoder* encoder)
{}

void CloudRenderer::Terminate()
{}