#pragma once

class CloudBounds : public Node
{
public:
   vec3 Extents = vec3(1.0f);

   CloudBounds() = default;
   CloudBounds(std::string name, vec3 position, vec3 extents) : Node(name, identity<mat4x4>()), Extents(extents)
   {
      SetPosition(position);
   }

   vec3 GetMin() const { return GetPosition() - Extents; }
   vec3 GetMax() const { return GetPosition() + Extents; }
   vec3 GetSize() const { return Extents * 2.0f; }
   vec3 GetCenter() const { return GetPosition(); }
   vec3 GetHalfSize() const { return Extents; }
};