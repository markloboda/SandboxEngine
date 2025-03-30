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

mat4 FreeCamera::GetViewProjectionMatrix()
{
   return perspective(radians(_zoom), 16.0f / 9.0f, 0.1f, 100.0f) * GetViewMatrix();
}

mat4 FreeCamera::GetViewMatrix()
{
   return lookAt(_position, _position + _forward, _worldUp);
}

void FreeCamera::Update(float dt)
{
   // Movement
   if (Input::IsKeyPressed(Input::EInputKey::KEY_W))
   {
      _position += _forward * _speed * dt;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_S))
   {
      _position -= _forward * _speed * dt;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_A))
   {
      _position -= _right * _speed * dt;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_D))
   {
      _position += _right * _speed * dt;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_E))
   {
      _position += _worldUp * _speed * dt;
   }
   if (Input::IsKeyPressed(Input::EInputKey::KEY_Q))
   {
      _position -= _worldUp * _speed * dt;
   }

   // Rotation
   if (Input::IsKeyPressed(Input::EInputKey::MOUSE_BUTTON_RIGHT))
   {
      vec2 mouseOffset = Input::GetCursorDelta();
      float dYaw = radians(mouseOffset.x * _sensitivity);
      float dPitch = radians(mouseOffset.y * _sensitivity);

      _yaw += dYaw;
      _pitch = clamp(_pitch + dPitch, -PI / 2.0f + 0.01f, PI / 2.0f - 0.01f);

      // Print mouse offset.
      std::cout << "Mouse offset: " << mouseOffset.x << ", " << mouseOffset.y << std::endl;
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
