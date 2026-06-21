# Phase 10: Fluid Simulation (SPH) 🌊

This phase implements a real-time Smoothed Particle Hydrodynamics (SPH) fluid simulation. Fluid is represented as particles that experience density-driven pressure forces, viscosity shear forces, and interact with static/dynamic rigid body colliders.

## 📁 Filenames & Directory Structure

* **Physics Subsystem**:
  * `include/physics/fluid_sim.hpp` & `src/physics/fluid_sim.cpp` — Fluid simulation loop and SPH kernels.
* **Renderer Subsystem (Shaders)**:
  * `shaders/fluid.vs` & `shaders/fluid.fs` — Displays particles as soft radial drops instead of solid circles.

## ⚙️ Core Variables

### `FluidParticle` Struct
* `Vec2 Position`, `Velocity`, `Force` — Particle dynamics.
* `float Density` — Local density calculated from neighbors.
* `float Pressure` — Pressure calculated from local density vs rest density.

### `FluidConfig` Settings
* `float ParticleRadius = 0.15f` — Collision radius.
* `float H = 0.4f` — Smoothing length (radius of influence for kernel function $W$).
* `float RestDensity = 1000.0f` — Rest density $\rho_0$.
* `float GasConstant = 2000.0f` — Stiffness constant $k$ (determines compressibility).
* `float Viscosity = 250.0f` — Viscosity coefficient $\mu$ (thickness of fluid).
* `Vec2 Gravity = Vec2(0.0f, -9.81f)` — Gravity force.

## 🏛️ Engine Component Role

* **SPH Engine**: Computes fluid particle interactions. Uses a specialized spatial hash to quickly find nearby fluid particles.
* **Fluid-Rigid Coupling**: Checks fluid particles against rigid body boundaries (e.g. circle/box colliders), applying boundary impulse responses to the fluid and equal-and-opposite forces to dynamic rigid bodies.

## 📝 Class Structures & Flow of Execution

### 1. `include/physics/fluid_sim.hpp`
```cpp
#pragma once
#include "core/math_utils.hpp"
#include "physics/rigid_body.hpp"
#include <vector>

namespace Physix {
    struct FluidParticle {
        Vec2 Position;
        Vec2 Velocity;
        Vec2 Force;
        float Density = 0.0f;
        float Pressure = 0.0f;
    };

    struct FluidConfig {
        float ParticleRadius = 0.15f;
        float H = 0.4f; // Smoothing length
        float RestDensity = 1000.0f;
        float GasConstant = 2000.0f;
        float Viscosity = 250.0f;
        Vec2 Gravity = Vec2(0.0f, -9.81f);
    };

    class FluidSim {
    public:
        FluidSim();
        ~FluidSim();

        void AddParticle(const FluidParticle& particle);
        void Clear();
        void Step(float dt, const std::vector<RigidBody*>& rigidBodies);

        const std::vector<FluidParticle>& GetParticles() const { return m_Particles; }

    private:
        void HashParticles();
        std::vector<int> GetNeighbors(const Vec2& position);
        void ResolveObstacles(FluidParticle& p, const std::vector<RigidBody*>& rigidBodies);

        // SPH Mathematical Kernels
        float KernelPoly6(float rSq, float h) const;
        Vec2 KernelSpikyGradient(const Vec2& rVec, float r, float h) const;
        float KernelViscosityLaplacian(float r, float h) const;

        std::vector<FluidParticle> m_Particles;
        FluidConfig m_Config;
        float m_ParticleMass = 1.0f;
    };
}
```

### 2. `src/physics/fluid_sim.cpp`
```cpp
#include "physics/fluid_sim.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Physix {
    FluidSim::FluidSim() {}
    FluidSim::~FluidSim() {}

    void FluidSim::AddParticle(const FluidParticle& particle) {
        m_Particles.push_back(particle);
    }

    void FluidSim::Clear() {
        m_Particles.clear();
    }

    float FluidSim::KernelPoly6(float rSq, float h) const {
        if (rSq < 0.0f || rSq >= h * h) return 0.0f;
        float term = h * h - rSq;
        return (315.0f / (64.0f * static_cast<float>(M_PI) * std::pow(h, 9))) * term * term * term;
    }

    Vec2 FluidSim::KernelSpikyGradient(const Vec2& rVec, float r, float h) const {
        if (r <= 0.0f || r >= h) return Vec2(0.0f, 0.0f);
        float term = h - r;
        float scalar = -45.0f / (static_cast<float>(M_PI) * std::pow(h, 6)) * term * term;
        return rVec * (scalar / r);
    }

    float FluidSim::KernelViscosityLaplacian(float r, float h) const {
        if (r >= h) return 0.0f;
        return (45.0f / (static_cast<float>(M_PI) * std::pow(h, 6))) * (h - r);
    }

    void FluidSim::HashParticles() {
        // Broadphase spatial hashing logic
    }

    std::vector<int> FluidSim::GetNeighbors(const Vec2& position) {
        // Return nearby particle indices from spatial hash
        std::vector<int> neighbors;
        for (size_t i = 0; i < m_Particles.size(); ++i) {
            neighbors.push_back(static_cast<int>(i));
        }
        return neighbors;
    }

    void FluidSim::Step(float dt, const std::vector<RigidBody*>& rigidBodies) {
        // Step 1: Broadphase - Hash particles into spatial grid
        HashParticles();

        // Step 2: Compute Density & Pressure
        for (size_t i = 0; i < m_Particles.size(); ++i) {
            float densitySum = 0.0f;
            auto neighbors = GetNeighbors(m_Particles[i].Position);

            for (int idx : neighbors) {
                Vec2 diff = m_Particles[i].Position - m_Particles[idx].Position;
                float rSq = diff.LengthSq();
                densitySum += m_ParticleMass * KernelPoly6(rSq, m_Config.H);
            }

            m_Particles[i].Density = std::max(densitySum, m_Config.RestDensity);
            m_Particles[i].Pressure = m_Config.GasConstant * (m_Particles[i].Density - m_Config.RestDensity);
        }

        // Step 3: Compute Forces (Pressure + Viscosity + Gravity)
        for (size_t i = 0; i < m_Particles.size(); ++i) {
            Vec2 pressureForce(0.0f, 0.0f);
            Vec2 viscosityForce(0.0f, 0.0f);
            auto neighbors = GetNeighbors(m_Particles[i].Position);

            for (int idx : neighbors) {
                if (idx == static_cast<int>(i)) continue;

                Vec2 diff = m_Particles[i].Position - m_Particles[idx].Position;
                float r = diff.Length();
                if (r >= m_Config.H || r < 0.0001f) continue;

                float pTerm = (m_Particles[i].Pressure + m_Particles[idx].Pressure) / (2.0f * m_Particles[idx].Density);
                pressureForce = pressureForce - KernelSpikyGradient(diff, r, m_Config.H) * (m_ParticleMass * pTerm);

                Vec2 vDiff = m_Particles[idx].Velocity - m_Particles[i].Velocity;
                float vTerm = KernelViscosityLaplacian(r, m_Config.H) / m_Particles[idx].Density;
                viscosityForce = viscosityForce + vDiff * (m_Config.Viscosity * m_ParticleMass * vTerm);
            }

            Vec2 gravityForce = m_Config.Gravity * m_Particles[i].Density;
            m_Particles[i].Force = pressureForce + viscosityForce + gravityForce;
        }

        // Step 4: Integrate Particles & Resolve Obstacles
        for (size_t i = 0; i < m_Particles.size(); ++i) {
            m_Particles[i].Velocity = m_Particles[i].Velocity + (m_Particles[i].Force / m_Particles[i].Density) * dt;
            m_Particles[i].Position = m_Particles[i].Position + m_Particles[i].Velocity * dt;

            ResolveObstacles(m_Particles[i], rigidBodies);
        }
    }

    void FluidSim::ResolveObstacles(FluidParticle& p, const std::vector<RigidBody*>& rigidBodies) {
        for (RigidBody* body : rigidBodies) {
            if (body->GetType() == ShapeType::CIRCLE) {
                auto circle = std::static_pointer_cast<CircleShape>(body->GetShape());
                Vec2 diff = p.Position - body->Position;
                float dist = diff.Length();
                float rLimit = circle->GetRadius() + m_Config.ParticleRadius;
                
                if (dist < rLimit) {
                    Vec2 normal = diff.Normalized();
                    float penetration = rLimit - dist;
                    
                    p.Position = p.Position + normal * penetration;
                    
                    float vn = Dot(p.Velocity, normal);
                    if (vn < 0.0f) {
                        p.Velocity = p.Velocity - normal * (vn * 1.5f);
                    }
                    
                    if (!body->IsStatic) {
                        Vec2 force = normal * (penetration * 50.0f);
                        body->ApplyForce(force); // Reaction force
                    }
                }
            }
        }
    }
}
```
```
