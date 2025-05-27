#pragma once

class Editor : public UIRenderer
{
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
};
