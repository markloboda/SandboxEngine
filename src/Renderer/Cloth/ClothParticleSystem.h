#pragma once

#define GRAVITY vec3(0, -9.81f, 0)
#include <execution>

class ClothParticleSystem
{
public:
   struct ParticleData
   {
      vec3 prevPosition;
      vec3 position;
      vec3 velocity;
      float mass;
      bool isFixed;

      float InvMass() const { return isFixed ? 0.0f : 1.0f / mass; }
   };

   struct DistanceConstraint
   {
      ParticleData *p1;
      ParticleData *p2;
      float restLength;

      void Satisfy() const
      {
         vec3 diff = p1->position - p2->position;
         float dist = length(diff);
         if (dist == 0.0f) return;

         vec3 dir = diff / dist;
         float C = dist - restLength;

         float w1 = p1->InvMass();
         float w2 = p2->InvMass();
         float wSum = w1 + w2;
         if (wSum == 0.0f) return;

         // Distribute correction based on inverse masses
         vec3 correction = C * dir;
         if (!p1->isFixed) p1->position -= (w1 / wSum) * correction; // ∆x1 = -C * dir
         if (!p2->isFixed) p2->position += (w2 / wSum) * correction; // ∆x2 = +C * dir
      }
   };

private:
   std::vector<ParticleData> _particles;
   std::vector<DistanceConstraint> _constraints;

public:
   void InitializeDemo(size_t width, size_t height)
   {
      _particles.clear();
      _particles.reserve(width * height);
      for (size_t i = 0; i < width; ++i)
      {
         for (size_t j = 0; j < height; ++j)
         {
            vec3 pos = vec3(15.0f, 5.0f, 0.0f) + vec3(j, 0.0f, i);

            ParticleData particle;
            particle.position = pos;
            particle.prevPosition = particle.position;
            particle.velocity = vec3(0);
            particle.mass = 1.0f;
            particle.isFixed = i == 0 && (j == 0 || j == height - 1);
            _particles.push_back(particle);
         }
      }

      _constraints.clear();
      _constraints.reserve((width - 1) * height + (height - 1) * width);
      for (size_t i = 0; i < width; ++i)
      {
         for (size_t j = 0; j < height; ++j)
         {
            if (i < width - 1)
            {
               DistanceConstraint constraint;
               constraint.p1 = &_particles[i * height + j];
               constraint.p2 = &_particles[(i + 1) * height + j];
               constraint.restLength = length(constraint.p1->position - constraint.p2->position);
               _constraints.push_back(constraint);
            }
            if (j < height - 1)
            {
               DistanceConstraint constraint;
               constraint.p1 = &_particles[i * height + j];
               constraint.p2 = &_particles[i * height + (j + 1)];
               constraint.restLength = 1.0f;
               _constraints.push_back(constraint);
            }
         }
      }
   }

   void Update(float dt)
   {
      const int solverIterations = 3;
      float solverDt = dt / (float) solverIterations;
      for (int iter = 0; iter < solverIterations; ++iter)
      {
         // Update particles
         for (auto &particle: _particles)
         {
            if (particle.isFixed)
               continue;

            particle.prevPosition = particle.position;
            particle.velocity += solverDt * GRAVITY / particle.mass;
            particle.position += solverDt * particle.velocity;
         }

         for (const auto &c: _constraints)
            c.Satisfy();

         for (auto &p: _particles)
         {
            if (p.isFixed)
               continue;

            p.velocity = (p.position - p.prevPosition) / solverDt;
         }
      }
   }

   void GetParticles(const ParticleData *&data, size_t &count) const
   {
      data = _particles.data();
      count = _particles.size();
   }

   void AddParticle(const ParticleData &particle)
   {
      _particles.push_back(particle);
   }

   void GetConstraints(const DistanceConstraint *&data, size_t &count) const
   {
      data = _constraints.data();
      count = _constraints.size();
   }

   void AddConstraint(const DistanceConstraint &constraint)
   {
      _constraints.push_back(constraint);
   }

   void GetVertexPositions(std::vector<vec3> &out) const
   {
      out.resize(_particles.size());
      for (size_t i = 0; i < _particles.size(); ++i)
         out[i] = _particles[i].position;
   }
};
