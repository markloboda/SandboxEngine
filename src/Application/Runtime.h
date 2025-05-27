#pragma once

#include <Application/Editor.h>

class Runtime
{
private:
   Editor *_editor = nullptr;
   Renderer* _renderer = nullptr;

   ClothParticleSystem *_clothParticleSystem = nullptr;

public:
   explicit Runtime();
   ~Runtime();

   bool Initialize();
   void Terminate() const;

   void FixedUpdate(float dt);
   void Update(float dt);
   void Render();

public:
   // Getters
   [[nodiscard]] Renderer &GetRenderer() const
   {
      return *_renderer;
   }

   [[nodiscard]] FreeCamera &GetActiveCamera() const
   {
      return _editor->GetCamera();
   }


   [[nodiscard]] ClothParticleSystem &GetClothParticleSystem() const
   {
      return *_clothParticleSystem;
   }
};

