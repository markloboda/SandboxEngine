#include <pch.h>
#include <Application/Input.h>

#include <Application/Application.h>

bool Input::IsKeyPressed(EInputKey key)
{
   GLFWwindow* window = Application::GetInstance().GetWindow();

   if (key >= 0 && key <= 7)
   {
      return glfwGetMouseButton(window, key) == GLFW_PRESS;
   }

   return glfwGetKey(Application::GetInstance().GetWindow(), static_cast<int>(key)) == GLFW_PRESS;
}

vec2 Input::GetCursorPos()
{
   double x, y;
   glfwGetCursorPos(Application::GetInstance().GetWindow(), &x, &y);
   return vec2(static_cast<float>(x), static_cast<float>(y));
}