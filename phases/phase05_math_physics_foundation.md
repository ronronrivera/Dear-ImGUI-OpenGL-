# Phase 05: Math & Physics Foundation ⚙️

This phase creates the core structures that store physical state (bodies, vectors, shapes) and manages the fixed-timestep simulator clock.

## 📁 Filenames & Directory Structure

To separate physics simulation logic from the engine's core system components, files are placed in their respective modules:

* **Core Subsystem**:
  * `include/core/math_utils.hpp` — Custom `Vec2` structure and helper operators.
  * `include/core/clock.hpp` & `src/core/clock.cpp` — Semi-implicit fixed timestep clock accumulator.
* **Physics Subsystem**:
  * `include/physics/rigid_body.hpp` — State properties, Material, and configurations for physics bodies.
  * `src/physics/rigid_body.cpp` — Mass calculations and state initializations.
  * `include/physics/shape.hpp` — Base shape class and Circle/Box subclass boundary geometries.
  * `include/physics/world.hpp` & `src/physics/world.cpp` — Simulation workspace database containing rigid bodies and gravity properties.

## ⚙️ Core Variables

### `Vec2` Struct
* `float x, y` — Vector components.

### `Material` Struct
* `float Density` — Used to calculate mass based on area.
* `float Restitution` — Bounciness coefficient `[0.0, 1.0]`.
* `float Friction` — Friction coefficient `[0.0, 1.0]`.

### `RigidBody` Properties
* `Vec2 Position`, `Velocity`, `Force` — Translational properties.
* `float Rotation` (angle in radians), `AngularVelocity`, `Torque` — Rotational properties.
* `float Mass`, `InvMass` — Inertial translation (InvMass = 0 for static bodies).
* `float Inertia`, `InvInertia` — Resistance to angular acceleration.
* `ShapeType Type` — Circle or Box discriminator.
* `bool IsStatic` — Keeps body anchored in space.
* `bool IsSleeping` — Deactivates simulation updates when speed drops below a threshold.
* `float Motion` — Running average of kinetic energy to decide sleeping.

### `World` Context
* `std::vector<RigidBody*> Bodies` — Array of references to active bodies.
* `Vec2 Gravity` — Global gravity vector.

## 🏛️ Engine Component Role

* **Physics Database (World)**: Owns body allocations and handles system execution order.
* **Timekeeper (Clock)**: Decouples rendering framerate from physical tick rates to maintain simulation stability.

## 📝 Class Structures & Flow of Execution

### 1. `include/core/math_utils.hpp`
```cpp
#pragma once
#include <cmath>

namespace Physix {
    struct Vec2 {
        float x = 0.0f;
        float y = 0.0f;

        Vec2() = default;
        Vec2(float _x, float _y) : x(_x), y(_y) {}

        Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
        Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
        Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
        
        float LengthSq() const { return x*x + y*y; }
        float Length() const { return std::sqrt(x*x + y*y); }
        
        Vec2 Normalized() const {
            float len = Length();
            if (len > 0.0001f) return Vec2(x / len, y / len);
            return Vec2(0.0f, 0.0f);
        }
    };

    inline float Dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
    inline float Cross(const Vec2& a, const Vec2& b) { return a.x * b.y - a.y * b.x; }
    inline Vec2 Cross(const Vec2& v, float s) { return Vec2(s * v.y, -s * v.x); }
    inline Vec2 Cross(float s, const Vec2& v) { return Vec2(-s * v.y, s * v.x); }
}
```

### 2. `include/core/clock.hpp`
```cpp
#pragma once
#include <functional>

namespace Physix {
    class Clock {
    public:
        Clock();
        ~Clock();

        void Tick(float realDeltaTime, float physicsTimestep, std::function<void(float)> physicsStepCallback);

    private:
        float m_Accumulator = 0.0f;
    };
}
```

### 3. `src/core/clock.cpp`
```cpp
#include "core/clock.hpp"

namespace Physix {
    Clock::Clock() {}
    Clock::~Clock() {}

    void Clock::Tick(float realDeltaTime, float physicsTimestep, std::function<void(float)> physicsStepCallback) {
        if (realDeltaTime > 0.25f) realDeltaTime = 0.25f; // Avoid spiral of death

        m_Accumulator += realDeltaTime;
        
        while (m_Accumulator >= physicsTimestep) {
            physicsStepCallback(physicsTimestep);
            m_Accumulator -= physicsTimestep;
        }
    }
}
```

### 4. `include/physics/shape.hpp`
```cpp
#pragma once
#include "core/math_utils.hpp"

namespace Physix {
    enum class ShapeType {
        CIRCLE,
        BOX
    };

    class Shape {
    public:
        virtual ~Shape() = default;
        virtual ShapeType GetType() const = 0;
    };

    class CircleShape : public Shape {
    public:
        CircleShape(float radius) : m_Radius(radius) {}
        virtual ShapeType GetType() const override { return ShapeType::CIRCLE; }
        float GetRadius() const { return m_Radius; }

    private:
        float m_Radius;
    };

    class BoxShape : public Shape {
    public:
        BoxShape(float width, float height) : m_Size(width, height) {}
        virtual ShapeType GetType() const override { return ShapeType::BOX; }
        Vec2 GetSize() const { return m_Size; }

    private:
        Vec2 m_Size;
    };
}
```

### 5. `include/physics/rigid_body.hpp`
```cpp
#pragma once
#include "core/math_utils.hpp"
#include "physics/shape.hpp"
#include <memory>

namespace Physix {
    struct Material {
        float Density = 1.0f;
        float Restitution = 0.5f;
        float Friction = 0.3f;
    };

    class RigidBody {
    public:
        RigidBody(std::shared_ptr<Shape> shape, const Vec2& position = {0.0f, 0.0f});
        ~RigidBody();

        void ComputeMassProperties();
        void ApplyForce(const Vec2& force);
        void ApplyTorque(float torque);

        // State properties
        Vec2 Position;
        Vec2 Velocity;
        Vec2 ForceAccumulator;

        float Rotation = 0.0f;
        float AngularVelocity = 0.0f;
        float TorqueAccumulator = 0.0f;

        float Mass = 0.0f;
        float InvMass = 0.0f;
        float Inertia = 0.0f;
        float InvInertia = 0.0f;

        Material Material;
        bool IsStatic = false;
        bool IsSleeping = false;
        float SleepTimer = 0.0f;

        std::shared_ptr<Shape> GetShape() const { return m_Shape; }
        ShapeType GetType() const { return m_Shape->GetType(); }

    private:
        std::shared_ptr<Shape> m_Shape;
    };
}
```

### 6. `src/physics/rigid_body.cpp`
```cpp
#include "physics/rigid_body.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Physix {
    RigidBody::RigidBody(std::shared_ptr<Shape> shape, const Vec2& position)
        : m_Shape(shape), Position(position) {
        ComputeMassProperties();
    }

    RigidBody::~RigidBody() {}

    void RigidBody::ComputeMassProperties() {
        if (IsStatic) {
            Mass = 0.0f;
            InvMass = 0.0f;
            Inertia = 0.0f;
            InvInertia = 0.0f;
            return;
        }

        if (GetType() == ShapeType::CIRCLE) {
            auto circle = std::static_pointer_cast<CircleShape>(m_Shape);
            float r = circle->GetRadius();
            float area = static_cast<float>(M_PI) * r * r;
            Mass = area * Material.Density;
            InvMass = Mass > 0.0f ? 1.0f / Mass : 0.0f;
            Inertia = 0.5f * Mass * r * r;
            InvInertia = Inertia > 0.0f ? 1.0f / Inertia : 0.0f;
        } 
        else if (GetType() == ShapeType::BOX) {
            auto box = std::static_pointer_cast<BoxShape>(m_Shape);
            Vec2 size = box->GetSize();
            float area = size.x * size.y;
            Mass = area * Material.Density;
            InvMass = Mass > 0.0f ? 1.0f / Mass : 0.0f;
            Inertia = (1.0f / 12.0f) * Mass * (size.x * size.x + size.y * size.y);
            InvInertia = Inertia > 0.0f ? 1.0f / Inertia : 0.0f;
        }
    }

    void RigidBody::ApplyForce(const Vec2& force) {
        if (IsStatic) return;
        ForceAccumulator = ForceAccumulator + force;
        IsSleeping = false;
        SleepTimer = 0.0f;
    }

    void RigidBody::ApplyTorque(float torque) {
        if (IsStatic) return;
        TorqueAccumulator += torque;
        IsSleeping = false;
        SleepTimer = 0.0f;
    }
}
```

### 7. `include/physics/world.hpp`
```cpp
#pragma once
#include <vector>
#include "physics/rigid_body.hpp"

namespace Physix {
    class World {
    public:
        World(const Vec2& gravity);
        ~World();

        void AddBody(RigidBody* body);
        void RemoveBody(RigidBody* body);
        void Step(float dt);

        std::vector<RigidBody*> Bodies;
        Vec2 Gravity;
    };
}
```

### 8. `src/physics/world.cpp`
```cpp
#include "physics/world.hpp"
#include <algorithm>

namespace Physix {
    World::World(const Vec2& gravity) : Gravity(gravity) {}

    World::~World() {
        for (RigidBody* body : Bodies) {
            delete body;
        }
        Bodies.clear();
    }

    void World::AddBody(RigidBody* body) {
        Bodies.push_back(body);
    }

    void World::RemoveBody(RigidBody* body) {
        auto it = std::find(Bodies.begin(), Bodies.end(), body);
        if (it != Bodies.end()) {
            delete *it;
            Bodies.erase(it);
        }
    }

    void World::Step(float dt) {
        // Will accumulate step physics updates here in Phase 06
    }
}
```
