#include <pch.h>
#include <Application/Input.h>

#include <Application/Application.h>

void Input::CursorPosChanged(GLFWwindow * /*window*/, double xpos, double ypos)
{
   for (auto &callback: GetInstance()._cursorPositionCallback)
   {
      callback(xpos, ypos);
   }
}

void Input::MouseWheelChanged(GLFWwindow * /*window*/, double /*xoffset*/, double yoffset)
{
   for (auto &callback: GetInstance()._mouseWheelCallback)
   {
      callback(yoffset);
   }
}

void Input::Initialize()
{
   glfwSetCursorPosCallback(Application::GetInstance().GetWindow(), CursorPosChanged);
   glfwSetScrollCallback(Application::GetInstance().GetWindow(), MouseWheelChanged);
}

void Input::Terminate()
{
   GetInstance()._cursorPositionCallback.clear();
   glfwSetCursorPosCallback(Application::GetInstance().GetWindow(), nullptr);
}

bool Input::IsKeyPressed(EInputKey key)
{
   GLFWwindow *window = Application::GetInstance().GetWindow();

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

void Input::SetCursorPositionCallback(const CursorPositionCallback &callback)
{
   GetInstance()._cursorPositionCallback.push_back(callback);
}

void Input::SetMouseWheelCallback(const MouseWheelCallback &callback)
{
   GetInstance()._mouseWheelCallback.push_back(callback);
}
