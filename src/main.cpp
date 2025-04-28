#include <pch.h>
#include <Application/Application.h>

int main()
{
   if (!Application::GetInstance().Initialize())
   {
      return -1;
   }

   Application::GetInstance().Run();
   Application::GetInstance().Terminate();
   return 0;
}
