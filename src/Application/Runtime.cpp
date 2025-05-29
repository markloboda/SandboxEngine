#include <pch.h>

bool Runtime::Initialize()
{
   _clothParticleSystem = new ClothParticleSystem();
   _clothParticleSystem->InitializeDemo(50, 50);

   _weatherSystem = new WeatherSystem();

   _renderer = new Renderer(Application::GetInstance().GetWindow());
   _editor = new Editor();

   return true;
}

void Runtime::Terminate() const
{
   delete _editor;
   delete _renderer;
}

void Runtime::FixedUpdate(float dt)
{
   // TODO: This is a hack to disable cloth rendering when not visible.
   if (_renderer->RenderCloth)
   {
      _clothParticleSystem->FixedUpdate(dt);
   }
}

void Runtime::Update(float dt)
{
   _editor->Update(dt);
   _weatherSystem->Update(dt);
}

void Runtime::Render()
{
   _renderer->Render();
}

