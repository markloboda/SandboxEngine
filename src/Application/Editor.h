#pragma once
#include "Renderer/UI/UIRenderer.h"

class Editor : public UIRenderer
{
private:
   bool _ShowGrid = false;

public:
   Editor();
   ~Editor();

   void RenderImGuiUI() override;
};
