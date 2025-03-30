#include <pch.h>
#include <Application/Input.h>

#include <Application/Application.h>

double mouseX_;
double mouseY_;

void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
   mouseX_ = xpos;
   mouseY_ = ypos;
}

void Input::Initialize()
{
   GLFWwindow* window = Application::GetInstance().GetWindow();
   glfwSetCursorPosCallback(window, CursorPositionCallback);
}

void Input::Update()
{
   Input& input = GetInstance();

   input._moveMouseX = mouseX_ - input._mouseX;
   input._moveMouseY = mouseY_ - input._mouseY;
   input._mouseX = mouseX_;
   input._mouseY = mouseY_;
}

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

vec2 Input::GetCursorDelta()
{
   Input& input = GetInstance();
   vec2 delta = vec2(static_cast<float>(input._moveMouseX), static_cast<float>(input._moveMouseY));
   return delta;
}