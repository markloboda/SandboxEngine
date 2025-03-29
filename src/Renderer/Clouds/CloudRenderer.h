#pragma once

class CloudRenderer
{
public:
   CloudRenderer();
   ~CloudRenderer();

   [[nodiscard]] bool Initialize();
   void Render(CommandEncoder* encoder);
   void Terminate();
};