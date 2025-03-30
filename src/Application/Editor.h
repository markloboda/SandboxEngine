#pragma once
#include "Renderer/UI/UIRenderer.h"

class FreeCamera;

class Editor : public UIRenderer
{
private:
   bool _ShowGrid = false;

   FreeCamera* _camera;

public:
   Editor();
   ~Editor() override;

   void RenderImGuiUI() override;
   void Update(float dt);

   FreeCamera& GetCamera() const
   {
      return *_camera;
   }
};
