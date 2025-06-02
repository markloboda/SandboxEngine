#include <pch.h>
#include <Utils/FreeCamera.h>

double mouseX_;
double mouseY_;
double mouseDifX_;
double mouseDifY_;

double mouseWheel_;

FreeCamera::FreeCamera(vec3 position, vec3 up, float yaw, float pitch)
{
   _position = position;
   _worldUp = up;
   _yaw = yaw;
   _pitch = pitch;
   _forward = vec3(0.0f, 0.0f, -1.0f);
   _speed = 100.0f;
   _sensitivity = 0.1f;
   _zoom = 60.0f;
   UpdateCameraVectors();

   Input::SetCursorPositionCallback(
      [this](double xpos, double ypos)
      {
         mouseDifX_ = xpos - mouseX_;
         mouseDifY_ = ypos - mouseY_;
         mouseX_ = xpos;
         mouseY_ = ypos;

         this->ProcessMouseMovement(mouseDifX_, mouseDifY_);
      }
   );

   Input::SetMouseWheelCallback(
      [](double yoffset)
      {
         mouseWheel_ = yoffset;
      }
   );
}

mat4 FreeCamera::GetViewProjectionMatrix() const
{
   return GetProjectionMatrix() * GetViewMatrix();
}

mat4 FreeCamera::GetProjectionMatrix() const
{
   return perspective(radians(_zoom), 16.0f / 9.0f, 0.1f, 100000.0f);
}

mat4 FreeCamera::GetViewMatrix() const
{
   return lookAt(_position, _position + _forward, _worldUp);
}

void FreeCamera::Update(float dt)
{
   if (Input::IsKeyPressed(Input::MOUSE_BUTTON_RIGHT) || Input::IsKeyPressed(Input::KEY_RIGHT_CONTROL))
   {
      // Mouse wheel speed increase
      _speed = clamp(mouseWheel_ > 0 ? _speed * powf(1.1f, fabsf(static_cast<float>(mouseWheel_))) : _speed * powf(0.9f, fabsf(static_cast<float>(mouseWheel_))), 0.5f,
                     50000.0f);
      mouseWheel_ = 0;

      float speed = _speed;
      if (Input::IsKeyPressed(Input::KEY_LEFT_SHIFT))
      {
         speed *= 100;
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
         _position += _up * speed * dt;
      }
      if (Input::IsKeyPressed(Input::EInputKey::KEY_Q))
      {
         _position -= _up * speed * dt;
      }

      UpdateCameraVectors();
   }
}

void FreeCamera::ProcessMouseMovement(double xOffset, double yOffset, bool constrainPitch)
{
   if (Input::IsKeyPressed(Input::MOUSE_BUTTON_RIGHT) || Input::IsKeyPressed(Input::KEY_RIGHT_CONTROL))
   {
      xOffset *= _sensitivity;
      yOffset *= _sensitivity;
      _yaw += static_cast<float>(xOffset);
      _pitch -= static_cast<float>(yOffset);
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
