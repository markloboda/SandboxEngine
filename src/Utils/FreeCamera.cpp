#include <pch.h>
#include <Utils/FreeCamera.h>

#include <Application/Application.h>
#include <Application/Editor.h>

double mouseX_;
double mouseY_;
double mouseDifX_;
double mouseDifY_;

double mouseWheel_;

void CursorPositionCallback(double xpos, double ypos)
{
   mouseDifX_ = xpos - mouseX_;
   mouseDifY_ = ypos - mouseY_;
   mouseX_ = xpos;
   mouseY_ = ypos;

   Application::GetInstance().GetEditor()->GetCamera().ProcessMouseMovement(mouseDifX_, mouseDifY_);
}

void MouseWheelCallback(double yoffset)
{
   mouseWheel_ = yoffset;
}

FreeCamera::FreeCamera(vec3 position, vec3 up, float yaw, float pitch)
{
   _position = position;
   _worldUp = up;
   _yaw = yaw;
   _pitch = pitch;
   _forward = vec3(0.0f, 0.0f, -1.0f);
   _speed = 0.05f;
   _sensitivity = 0.1f;
   _zoom = 45.0f;
   UpdateCameraVectors();

   Input::SetCursorPositionCallback(CursorPositionCallback);
   Input::SetMouseWheelCallback(MouseWheelCallback);
}

mat4 FreeCamera::GetViewProjectionMatrix()
{
   return GetProjectionMatrix() * GetViewMatrix();
}

mat4 FreeCamera::GetProjectionMatrix()
{
   return perspective(radians(_zoom), 16.0f / 9.0f, 0.1f, 100000.0f);
}

mat4 FreeCamera::GetViewMatrix()
{
   return lookAt(_position, _position + _forward, _worldUp);
}

void FreeCamera::Update(float dt)
{
   if (Input::IsKeyPressed(Input::MOUSE_BUTTON_RIGHT))
   {
      // Mouse wheel speed increase
      _speed = clamp(mouseWheel_ > 0 ? _speed + 0.01f : _speed - 0.01f, 0.5f, 1000.0f);

      float speed = _speed;
      if (Input::IsKeyPressed(Input::KEY_LEFT_SHIFT))
      {
         speed *= 5.0f;
      }

      // Movement
      if (Input::IsKeyPressed(Input::EInputKey::KEY_W))
      {
         _position += _forward * speed * dt;
      }
      if (Input::IsKeyPressed(Input::EInputKey::KEY_S))
      {
         _position -= _forward * speed * dt;
      }
      if (Input::IsKeyPressed(Input::EInputKey::KEY_A))
      {
         _position -= _right * speed * dt;
      }
      if (Input::IsKeyPressed(Input::EInputKey::KEY_D))
      {
         _position += _right * speed * dt;
      }
      if (Input::IsKeyPressed(Input::EInputKey::KEY_E))
      {
         _position += _worldUp * speed * dt;
      }
      if (Input::IsKeyPressed(Input::EInputKey::KEY_Q))
      {
         _position -= _worldUp * speed * dt;
      }

      UpdateCameraVectors();
   }
}

void FreeCamera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
   if (Input::IsKeyPressed(Input::MOUSE_BUTTON_RIGHT))
   {
      xOffset *= _sensitivity;
      yOffset *= _sensitivity;
      _yaw += xOffset;
      _pitch -= yOffset;
      if (constrainPitch)
      {
         _pitch = clamp(_pitch, -89.0f, 89.0f);
      }
      UpdateCameraVectors();
   }
}

void FreeCamera::UpdateCameraVectors()
{
   vec3 front;
   front.x = cos(radians(_yaw)) * cos(radians(_pitch));
   front.y = sin(radians(_pitch));
   front.z = sin(radians(_yaw)) * cos(radians(_pitch));
   _forward = normalize(front);
   _right = normalize(cross(_forward, _worldUp));
   _up = normalize(cross(_right, _forward));
}
