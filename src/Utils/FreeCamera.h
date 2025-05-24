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

   mat4 GetViewProjectionMatrix() const;
   mat4 GetProjectionMatrix() const;
   mat4 GetViewMatrix() const;

   void Update(float dt);
   void ProcessMouseMovement(double xOffset, double yOffset, bool constrainPitch = true);
   void UpdateCameraVectors();

   vec3 GetPosition() const { return _position; }
   void SetPosition(vec3 position) { _position = position; }

   vec3 GetEulerRotation() const { return vec3(_pitch, _yaw, 0.0f); }
   void SetEulerRotation(vec3 rotation) { _pitch = rotation.x; _yaw = rotation.y; }
};