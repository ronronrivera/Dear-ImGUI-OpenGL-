# Phase 11: Physics Problem Scenarios 🧪

This phase implements a Scenario/Preset Factory to instantly reset the world state and spawn specific physics configurations (e.g. projectile motion, double pendulums, liquid dam breaks, stack collapses, and friction ramps) for debugging and user interaction.

## 📁 Filenames & Directory Structure

* **Scenario Controllers**:
  * `include/procgen/problem_factory.hpp` & `src/procgen/problem_factory.cpp` — Declares preset presets. Called by ImGui buttons.

## ⚙️ Core Variables

### `ProblemFactory` Settings
* `enum class ScenarioType` — Scenario ID (`PROJECTILE_MOTION`, `PENDULUM_CHAIN`, `FLUID_PRESSURE`, `STACK_COLLAPSE`, `INCLINED_PLANE`).

### Helper structures for Joints (needed for the Pendulum Scenario)
If implementing distance constraints for pendulums:
```cpp
struct DistanceJoint {
    RigidBody* BodyA;
    RigidBody* BodyB;
    Vec2 AnchorLocalA; // Offset from BodyA center
    Vec2 AnchorLocalB; // Offset from BodyB center
    float TargetDistance;
    float Stiffness = 1.0f; // Spring strength
};
```

## 🏛️ Engine Component Role

* **Scene Orquestrator**: Instantiates groups of rigid bodies and constraints, resetting the clock accumulator and emptying old buffers to start a fresh simulation run.

## 📝 Class Structures & Flow of Execution

### 1. `include/procgen/problem_factory.hpp`
```cpp
#pragma once
#include "physics/world.hpp"
#include "physics/fluid_sim.hpp"

namespace Physix {
    enum class ScenarioType {
        PROJECTILE_MOTION,
        PENDULUM_CHAIN,
        FLUID_PRESSURE,
        STACK_COLLAPSE,
        INCLINED_PLANE
    };

    struct DistanceJoint {
        RigidBody* BodyA = nullptr;
        RigidBody* BodyB = nullptr;
        Vec2 AnchorLocalA; // Offset from BodyA center
        Vec2 AnchorLocalB; // Offset from BodyB center
        float TargetDistance = 0.0f;
        float Stiffness = 1.0f; // Spring strength
    };

    class ProblemFactory {
    public:
        ProblemFactory();
        ~ProblemFactory();

        static void LoadScenario(ScenarioType type, World& world, FluidSim& fluidSim);

    private:
        static void SetupProjectileScene(World& world);
        static void SetupPendulumScene(World& world);
        static void SetupFluidDamScene(World& world, FluidSim& fluidSim);
        static void SetupStackCollapseScene(World& world);
        static void SetupInclinedPlaneScene(World& world);
        static void CreateBoundaryTank(World& world, float width, float height);
    };
}
```

### 2. `src/procgen/problem_factory.cpp`
```cpp
#include "procgen/problem_factory.hpp"
#include "physics/rigid_body.hpp"
#include <glm/glm.hpp>
#include <cmath>

namespace Physix {
    ProblemFactory::ProblemFactory() {}
    ProblemFactory::~ProblemFactory() {}

    void ProblemFactory::LoadScenario(ScenarioType type, World& world, FluidSim& fluidSim) {
        // Clear all existing bodies in the world (assuming a clear mechanism)
        // world.Clear(); 
        fluidSim.Clear();
        world.Gravity = Vec2(0.0f, -9.81f);

        switch (type) {
            case ScenarioType::PROJECTILE_MOTION:
                SetupProjectileScene(world);
                break;
            case ScenarioType::PENDULUM_CHAIN:
                SetupPendulumScene(world);
                break;
            case ScenarioType::FLUID_PRESSURE:
                SetupFluidDamScene(world, fluidSim);
                break;
            case ScenarioType::STACK_COLLAPSE:
                SetupStackCollapseScene(world);
                break;
            case ScenarioType::INCLINED_PLANE:
                SetupInclinedPlaneScene(world);
                break;
        }
    }

    void ProblemFactory::SetupProjectileScene(World& world) {
        // Ground (Static Box)
        auto groundShape = std::make_shared<BoxShape>(100.0f, 2.0f);
        RigidBody* ground = new RigidBody(groundShape, Vec2(0.0f, -5.0f));
        ground->IsStatic = true;
        ground->ComputeMassProperties();
        world.AddBody(ground);

        // Cannonball (Circle)
        auto ballShape = std::make_shared<CircleShape>(1.0f);
        RigidBody* ball = new RigidBody(ballShape, Vec2(-20.0f, 5.0f));
        ball->Velocity = Vec2(25.0f, 15.0f);
        ball->Material.Density = 2.0f;
        ball->Material.Restitution = 0.5f;
        ball->ComputeMassProperties();
        world.AddBody(ball);
    }

    void ProblemFactory::SetupPendulumScene(World& world) {
        // Fixed anchor box
        auto anchorShape = std::make_shared<BoxShape>(1.0f, 1.0f);
        RigidBody* anchor = new RigidBody(anchorShape, Vec2(0.0f, 15.0f));
        anchor->IsStatic = true;
        anchor->ComputeMassProperties();
        world.AddBody(anchor);

        RigidBody* previous = anchor;
        int linkCount = 5;
        float linkDistance = 2.0f;

        for (int i = 0; i < linkCount; ++i) {
            auto linkShape = std::make_shared<CircleShape>(0.5f);
            RigidBody* link = new RigidBody(linkShape, Vec2(previous->Position.x + linkDistance, previous->Position.y));
            link->IsStatic = false;
            link->Material.Density = 1.0f;
            link->ComputeMassProperties();
            world.AddBody(link);

            // Distance joint registration would happen here if implemented in the physics engine
            DistanceJoint joint;
            joint.BodyA = previous;
            joint.BodyB = link;
            joint.AnchorLocalA = Vec2(0.0f, 0.0f);
            joint.AnchorLocalB = Vec2(0.0f, 0.0f);
            joint.TargetDistance = linkDistance;
            // world.AddJoint(joint);

            previous = link;
        }
    }

    void ProblemFactory::CreateBoundaryTank(World& world, float width, float height) {
        float thickness = 1.0f;
        // Bottom boundary
        auto bottomShape = std::make_shared<BoxShape>(width, thickness);
        RigidBody* bottom = new RigidBody(bottomShape, Vec2(0.0f, -height / 2.0f));
        bottom->IsStatic = true;
        bottom->ComputeMassProperties();
        world.AddBody(bottom);

        // Left boundary
        auto leftShape = std::make_shared<BoxShape>(thickness, height);
        RigidBody* left = new RigidBody(leftShape, Vec2(-width / 2.0f, 0.0f));
        left->IsStatic = true;
        left->ComputeMassProperties();
        world.AddBody(left);

        // Right boundary
        auto rightShape = std::make_shared<BoxShape>(thickness, height);
        RigidBody* right = new RigidBody(rightShape, Vec2(width / 2.0f, 0.0f));
        right->IsStatic = true;
        right->ComputeMassProperties();
        world.AddBody(right);
    }

    void ProblemFactory::SetupFluidDamScene(World& world, FluidSim& fluidSim) {
        CreateBoundaryTank(world, 20.0f, 15.0f);

        int rows = 30;
        int cols = 15;
        float spacing = 0.3f;
        Vec2 startPos(-8.0f, -4.0f);

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                FluidParticle p;
                p.Position = startPos + Vec2(c * spacing, r * spacing);
                p.Velocity = Vec2(0.0f, 0.0f);
                fluidSim.AddParticle(p);
            }
        }

        // Floating box
        auto boxShape = std::make_shared<BoxShape>(4.0f, 4.0f);
        RigidBody* floatBox = new RigidBody(boxShape, Vec2(5.0f, -2.0f));
        floatBox->Material.Density = 0.5f;
        floatBox->ComputeMassProperties();
        world.AddBody(floatBox);
    }

    void ProblemFactory::SetupStackCollapseScene(World& world) {
        // Floor
        auto floorShape = std::make_shared<BoxShape>(50.0f, 2.0f);
        RigidBody* floor = new RigidBody(floorShape, Vec2(0.0f, -5.0f));
        floor->IsStatic = true;
        floor->ComputeMassProperties();
        world.AddBody(floor);

        // Grid of boxes
        float size = 1.0f;
        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 5; ++c) {
                auto boxShape = std::make_shared<BoxShape>(size, size);
                RigidBody* b = new RigidBody(boxShape, Vec2(-2.0f + c * 1.1f, -3.0f + r * 1.1f));
                b->IsStatic = false;
                b->Material.Density = 1.0f;
                b->ComputeMassProperties();
                world.AddBody(b);
            }
        }
    }

    void ProblemFactory::SetupInclinedPlaneScene(World& world) {
        // Tilted ramp (Box tilted 25 degrees)
        auto rampShape = std::make_shared<BoxShape>(25.0f, 1.0f);
        RigidBody* ramp = new RigidBody(rampShape, Vec2(-5.0f, 2.0f));
        ramp->Rotation = 0.4363f; // ~25 degrees in radians
        ramp->IsStatic = true;
        ramp->ComputeMassProperties();
        world.AddBody(ramp);

        // Sliding Box A (Slick, low friction)
        auto boxAShape = std::make_shared<BoxShape>(1.5f, 1.5f);
        RigidBody* sliderA = new RigidBody(boxAShape, Vec2(-10.0f, 7.0f));
        sliderA->Rotation = 0.4363f;
        sliderA->Material.Friction = 0.05f;
        sliderA->ComputeMassProperties();
        world.AddBody(sliderA);

        // Sliding Box B (Rough, high friction)
        auto boxBShape = std::make_shared<BoxShape>(1.5f, 1.5f);
        RigidBody* sliderB = new RigidBody(boxBShape, Vec2(-8.0f, 8.0f));
        sliderB->Rotation = 0.4363f;
        sliderB->Material.Friction = 0.8f;
        sliderB->ComputeMassProperties();
        world.AddBody(sliderB);
    }
}
```
```
