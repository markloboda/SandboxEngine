#include <pch.h>
#include <Utils/FreeCamera.h>

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
}

mat4 FreeCamera::GetViewMatrix()
{
   return lookAt(_position, _position + _forward, _worldUp);
}

void FreeCamera::ProcessInput()
{
   // Movement
   if (Input::IsKeyPressed(Input::EInputKey::KEY_W))
   {
      _position += _forward * _speed;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_S))
   {
      _position -= _forward * _speed;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_A))
   {
      _position -= _right * _speed;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_D))
   {
      _position += _right * _speed;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_E))
   {
      _position += _worldUp * _speed;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_Q))
   {
      _position -= _worldUp * _speed;
   }

   // Rotation
   if (Input::IsKeyPressed(Input::EInputKey::MOUSE_BUTTON_RIGHT))
   {
      vec2 mouseOffset = Input::GetCursorDelta();
      _yaw += mouseOffset.x * _sensitivity;
      _pitch += mouseOffset.y * _sensitivity;
      if (_pitch > 89.0f)
      {
         _pitch = 89.0f;
      }
      if (_pitch < -89.0f)
      {
         _pitch = -89.0f;
      }
   }

   UpdateCameraVectors();
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
