#pragma once
#include "Renderer/UI/UIRenderer.h"

class FreeCamera;

class Editor : public UIRenderer
{
private:
   bool _renderGrid = true;

   bool _renderAtmosphere = true;
   bool _renderClouds = true;

   bool _renderCloths = false;

   FreeCamera* _camera;

   Scene* _scene;

public:
   Editor();
   ~Editor() override;

   void RenderImGuiUI() override;
   void Update(float dt) const;

   FreeCamera &GetCamera() const
   {
      return *_camera;
   }

   Scene &GetScene() const
   {
      return *_scene;
   }

   bool GetRenderAtmosphere() const
   {
      return _renderAtmosphere;
   }

   bool GetRenderGrid() const
   {
      return _renderGrid;
   }

   bool GetRenderClouds() const
   {
      return _renderClouds;
   }

   bool GetRenderCloths() const
   {
      return _renderCloths;
   }
};
