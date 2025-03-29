
#pragma once

class UIRenderer
{
public:
   virtual ~UIRenderer() = default;
   virtual void RenderImGuiUI() = 0;
};
