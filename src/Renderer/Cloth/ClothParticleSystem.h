#pragma once

#define GRAVITY glm::vec3(0, -9.81f, 0)

using glm::vec2;
using glm::vec3;

class ClothParticleSystem
{
public:
   struct SystemSettings
   {
      int solverIterations = 10;
   };

   struct ParticleData
   {
      vec3 prevPosition;
      vec3 position;
      vec3 velocity;
      float mass;
      bool isFixed;

      float InvMass() const;
   };

   struct DistanceConstraint
   {
      ParticleData *p1;
      ParticleData *p2;
      float restLength;

      void Satisfy() const;
   };

private:
   std::vector<ParticleData> _particles;
   std::vector<DistanceConstraint> _constraints;
   vec2 _dimensions = {};

public:
   SystemSettings Settings = {};

public:
   void InitializeDemo(size_t width, size_t height);
   void FixedUpdate(float dt);

   void GetParticles(const ParticleData *&data, size_t &count) const;
   void AddParticle(const ParticleData &particle);

   void GetConstraints(const DistanceConstraint *&data, size_t &count) const;
   void AddConstraint(const DistanceConstraint &constraint);

   [[nodiscard]] vec2 GetDimensions() const;
};
