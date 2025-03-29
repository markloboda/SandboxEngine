#include <pch.h>
#include <Application/Input.h>

#include <Application/Application.h>

bool Input::IsKeyPressed(EInputKey key)
{
   return glfwGetKey(Application::GetInstance().GetWindow(), static_cast<int>(key)) == GLFW_PRESS;
}

vec2 Input::GetCursorPos()
{
   double x, y;
   glfwGetCursorPos(Application::GetInstance().GetWindow(), &x, &y);
   return vec2(static_cast<float>(x), static_cast<float>(y));
}

vec2 Input::GetCursorDelta()
{
   static double lastX = 0.0;
   static double lastY = 0.0;
   double x, y;
   glfwGetCursorPos(Application::GetInstance().GetWindow(), &x, &y);
   vec2 delta = vec2(static_cast<float>(x - lastX), static_cast<float>(y - lastY));
   lastX = x;
   lastY = y;
   return delta;
}