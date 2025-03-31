#pragma once

class FreeCamera
{
private:
   vec3 _position;
   vec3 _forward;
   vec3 _up;
   vec3 _right;
   vec3 _worldUp;
   float _yaw;
   float _pitch;
   float _speed;
   float _sensitivity;
   float _zoom;

public:
   FreeCamera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = 0, float pitch = 0);

   mat4 GetViewProjectionMatrix();
   mat4 GetProjectionMatrix();
   mat4 GetViewMatrix();

   void Update(float dt);
   void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
   void UpdateCameraVectors();

   vec3 GetPosition() const { return _position; }
   void SetPosition(vec3 position) { _position = position; }
};