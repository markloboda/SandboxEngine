#pragma once

class Node
{
private:
   std::string _name;
   mat4x4 _matrix = mat4x4(1.0f);

public:
   virtual ~Node() = default;

   Node() = default;
   Node(std::string name, const mat4x4& matrix) :
      _name(name),
      _matrix(matrix)
   {}

   void SetMatrix(const mat4x4& localMatrix) { _matrix = localMatrix; }
   const mat4x4& GetMatrix() const { return _matrix; }

   vec3 GetPosition() const { return vec3(_matrix[3]); }
   void SetPosition(const vec3& position) { _matrix[3] = vec4(position, 1.0f); }

   std::string GetName() const { return _name; }
   void SetName(const std::string& name) { _name = name; }
};