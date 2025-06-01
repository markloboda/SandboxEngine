#include <pch.h>

void Input::CursorPosChanged(GLFWwindow * /*window*/, double xpos, double ypos)
{
   Input &instace = GetInstance();
   for (auto &callback: instace._cursorPositionCallback)
   {
      callback(xpos, ypos);
   }
}

void Input::MouseWheelChanged(GLFWwindow * /*window*/, double /*xoffset*/, double yoffset)
{
   Input &instace = GetInstance();
   for (auto &callback: instace._mouseWheelCallback)
   {
      callback(yoffset);
   }
}

void Input::Initialize()
{
   Application &app = Application::GetInstance();
   glfwSetCursorPosCallback(app.GetWindow(), CursorPosChanged);
   glfwSetScrollCallback(app.GetWindow(), MouseWheelChanged);
}

void Input::Terminate()
{
   GetInstance()._cursorPositionCallback.clear();
   glfwSetCursorPosCallback(Application::GetInstance().GetWindow(), nullptr);
}

void Input::Update()
{
   Input &instace = GetInstance();
   for (auto &[key, state]: instace._previousState)
   {
      state = IsKeyPressed(key);
   }
}

bool Input::IsKeyPressed(EInputKey key)
{
   GLFWwindow *window = Application::GetInstance().GetWindow();

   if (key >= 0 && key <= 7)
   {
      return glfwGetMouseButton(window, key) == GLFW_PRESS;
   }

   return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
}

bool Input::IsKeyClicked(EInputKey key)
{
   Input &instace = GetInstance();

   bool isPressed = IsKeyPressed(key);
   bool wasPressed = instace._previousState[key];

   bool clicked = wasPressed && !isPressed;

   return clicked;
}

vec2 Input::GetCursorPos()
{
   double x, y;
   glfwGetCursorPos(Application::GetInstance().GetWindow(), &x, &y);
   return {static_cast<float>(x), static_cast<float>(y)};
}

void Input::SetCursorPositionCallback(const CursorPositionCallback &callback)
{
   GetInstance()._cursorPositionCallback.push_back(callback);
}

void Input::SetMouseWheelCallback(const MouseWheelCallback &callback)
{
   GetInstance()._mouseWheelCallback.push_back(callback);
}
