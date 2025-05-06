#pragma once
#include "Renderer/UI/UIRenderer.h"

class FreeCamera;

class Editor : public UIRenderer
{
private:
   bool _showAtmosphere = true;
   bool _showGrid = true;
   bool _renderClouds = true;

   FreeCamera* _camera;

   Scene* _scene;

public:
   Editor();
   ~Editor() override;

   void RenderImGuiUI() override;
   void Update(float dt);

   FreeCamera& GetCamera() const
   {
      return *_camera;
   }

   Scene* GetScene() const
   {
      return _scene;
   }

   bool GetRenderAtmosphere() const
   {
      return _showAtmosphere;
   }

   bool GetRenderGrid() const
   {
      return _showGrid;
   }

   bool GetRenderClouds() const
   {
      return _renderClouds;
   }
};
