# Phase 06: Integrator ⚙️

This phase implements the numerical integrator (Semi-implicit Euler) that moves bodies under the influence of gravity, forces, torques, and damping. It also implements sleep mechanics to optimize simulation performance.

## 📁 Filenames & Directory Structure

* **Physics Subsystem**:
  * `include/physics/integrator.hpp` & `src/physics/integrator.cpp` — Numerical integration formulas (Semi-implicit Euler integration, sleep management).
  * In practice, these methods are driven inside the step loop in `src/physics/world.cpp`.

## ⚙️ Core Variables

### Engine Configuration Constants
* `const float SLEEP_LINEAR_THRESHOLD = 0.05f` — Linear velocity limit below which a body may sleep.
* `const float SLEEP_ANGULAR_THRESHOLD = 0.05f` — Angular velocity limit below which a body may sleep.
* `const float SLEEP_TIME_REQUIREMENT = 0.5f` — Duration (in seconds) velocity must remain below threshold before sleeping.

### RigidBody State Additions
* `float SleepTimer` — Counts time spent below velocity thresholds.
* `Vec2 ForceAccumulator` — Accumulated external forces.
* `float TorqueAccumulator` — Accumulated torque forces.
* `float LinearDamping` — Linear air resistance / drag coefficient `[0.0, 1.0]`.
* `float AngularDamping` — Rotational air resistance coefficient `[0.0, 1.0]`.

## 🏛️ Engine Component Role

* **Equations of Motion Solver**: Resolves position and velocity updates over time. Semi-implicit Euler is preferred over Explicit Euler because it preserves energy much better and matches constraint-based impulse solvers.

## 📝 Class Structures & Flow of Execution

### 1. `include/physics/integrator.hpp`
```cpp
#pragma once
#include "physics/rigid_body.hpp"

namespace Physix {
    class Integrator {
    public:
        static void IntegrateForces(RigidBody* body, const Vec2& gravity, float dt);
        static void IntegrateVelocities(RigidBody* body, float dt);
        static void UpdateSleepState(RigidBody* body, float dt);

        static constexpr float SLEEP_LINEAR_THRESHOLD = 0.05f;
        static constexpr float SLEEP_ANGULAR_THRESHOLD = 0.05f;
        static constexpr float SLEEP_TIME_REQUIREMENT = 0.5f;
    };
}
```

### 2. `src/physics/integrator.cpp`
```cpp
#include "physics/integrator.hpp"

namespace Physix {
    void Integrator::IntegrateForces(RigidBody* body, const Vec2& gravity, float dt) {
        if (body->IsStatic || body->IsSleeping) return;

        // F = ma => a = F/m
        // We assume body has standard linear damping of 0.1f or custom from material
        Vec2 gravityForce = gravity * body->Mass;
        Vec2 totalForce = body->ForceAccumulator + gravityForce;
        Vec2 acceleration = totalForce * body->InvMass;
        
        body->Velocity = body->Velocity + acceleration * dt;
        body->Velocity = body->Velocity * (1.0f / (1.0f + dt * 0.1f));

        // T = I * alpha => alpha = T/I
        float angularAcceleration = body->TorqueAccumulator * body->InvInertia;
        body->AngularVelocity += angularAcceleration * dt;
        body->AngularVelocity *= (1.0f / (1.0f + dt * 0.1f));
    }

    void Integrator::IntegrateVelocities(RigidBody* body, float dt) {
        if (body->IsStatic || body->IsSleeping) return;

        body->Position = body->Position + body->Velocity * dt;
        body->Rotation += body->AngularVelocity * dt;

        body->ForceAccumulator = Vec2(0.0f, 0.0f);
        body->TorqueAccumulator = 0.0f;
    }

    void Integrator::UpdateSleepState(RigidBody* body, float dt) {
        if (body->IsStatic) return;

        float velSq = body->Velocity.LengthSq();
        float angVelSq = body->AngularVelocity * body->AngularVelocity;
        
        float linThresholdSq = SLEEP_LINEAR_THRESHOLD * SLEEP_LINEAR_THRESHOLD;
        float angThresholdSq = SLEEP_ANGULAR_THRESHOLD * SLEEP_ANGULAR_THRESHOLD;

        if (velSq < linThresholdSq && angVelSq < angThresholdSq) {
            body->SleepTimer += dt;
            if (body->SleepTimer >= SLEEP_TIME_REQUIREMENT) {
                body->IsSleeping = true;
                body->Velocity = Vec2(0.0f, 0.0f);
                body->AngularVelocity = 0.0f;
            }
        } else {
            body->SleepTimer = 0.0f;
            body->IsSleeping = false;
        }
    }
}
```

### 3. Core World Tick Loop (`src/physics/world.cpp`)
```cpp
#include "physics/world.hpp"
#include "physics/integrator.hpp"

namespace Physix {
    // ... Constructor/Destructor and add/remove body code ...

    void World::Step(float dt) {
        // Phase A: Integrate Forces (Generate Velocities)
        for (RigidBody* body : Bodies) {
            Integrator::IntegrateForces(body, Gravity, dt);
        }

        // Phase B: Resolve Constraints & Collisions (Updates Velocities)
        // (Broadphase and SAT Narrowphase Solver added in Phase 07/08)

        // Phase C: Integrate Velocities (Update Positions)
        for (RigidBody* body : Bodies) {
            Integrator::IntegrateVelocities(body, dt);
            Integrator::UpdateSleepState(body, dt);
        }
    }
}
```
