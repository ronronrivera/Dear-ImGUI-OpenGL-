# Phase 12: Debug Draw 👁️

This phase implements rendering overlays to draw debugging information over the physics scene, such as bounding boxes (AABBs), velocity vectors, collision normals, contact coordinates, and sleeping/awake state colors.

## 📁 Filenames & Directory Structure

* **Debug Drawer**:
  * `include/renderer/debug_draw.hpp` & `src/renderer/debug_draw.cpp` — Defines flags and drawing hooks.
  * Links with `Renderer2D` for fast lines/quad primitives.

## ⚙️ Core Variables

### `DebugFlags` Bitmask Configuration
```cpp
enum DebugDrawFlags : unsigned int {
    DRAW_SHAPE        = 1 << 0, // Draw standard shape geometry
    DRAW_AABB         = 1 << 1, // Draw bounding box outlines
    DRAW_VELOCITY     = 1 << 2, // Draw velocity lines
    DRAW_CONTACTS     = 1 << 3, // Draw collision contact points
    DRAW_SLEEP_STATE  = 1 << 4  // Hue bodies depending on active/sleep state
};
```

### `DebugDraw` Class Properties
* `unsigned int m_Flags` — Configured bitmask representing active debug features.
* `std::shared_ptr<Renderer2D> m_Renderer` — Handles batch rendering requests.

## 🏛️ Engine Component Role

* **Diagnostic Visualizer**: Superimposes vector lines and colored wireframes directly onto the scene texture. This allows developers to see what is happening in the physics simulator (e.g. why shapes are overlapping, if spatial hashes are clipping, or why friction forces are sticking).

## 📝 Class Structures & Flow of Execution

### 1. `include/renderer/debug_draw.hpp`
```cpp
#pragma once
#include "physics/world.hpp"
#include "renderer/renderer2d.hpp"
#include <memory>
#include <vector>

namespace Physix {
    enum DebugDrawFlags : unsigned int {
        DRAW_SHAPE        = 1 << 0, // Draw standard shape geometry
        DRAW_AABB         = 1 << 1, // Draw bounding box outlines
        DRAW_VELOCITY     = 1 << 2, // Draw velocity lines
        DRAW_CONTACTS     = 1 << 3, // Draw collision contact points
        DRAW_SLEEP_STATE  = 1 << 4  // Hue bodies depending on active/sleep state
    };

    struct ContactPoint {
        float x, y;
    };

    struct CollisionManifold {
        RigidBody* BodyA = nullptr;
        RigidBody* BodyB = nullptr;
        Vec2 Normal;
        float PenetrationDepth = 0.0f;
        ContactPoint Contacts[2];
        int ContactCount = 0;
    };

    struct AABB {
        Vec2 Min;
        Vec2 Max;
    };

    class DebugDraw {
    public:
        DebugDraw(std::shared_ptr<Renderer2D> renderer);
        ~DebugDraw();

        void SetFlags(unsigned int flags) { m_Flags = flags; }
        unsigned int GetFlags() const { return m_Flags; }

        void DrawWorld(const World& world, const std::vector<CollisionManifold>& manifolds);

    private:
        unsigned int m_Flags = DRAW_SHAPE;
        std::shared_ptr<Renderer2D> m_Renderer;
    };
}
```

### 2. `src/renderer/debug_draw.cpp`
```cpp
#include "renderer/debug_draw.hpp"
#include <glm/glm.hpp>

namespace Physix {
    DebugDraw::DebugDraw(std::shared_ptr<Renderer2D> renderer) : m_Renderer(renderer) {}
    DebugDraw::~DebugDraw() {}

    void DebugDraw::DrawWorld(const World& world, const std::vector<CollisionManifold>& manifolds) {
        // 1. Iterate through all simulated Rigid Bodies
        for (RigidBody* body : world.Bodies) {
            
            // Define tint color based on sleeping status
            glm::vec4 bodyColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f); // Default Light Gray
            
            if (m_Flags & DRAW_SLEEP_STATE) {
                if (body->IsSleeping) {
                    bodyColor = glm::vec4(0.4f, 0.4f, 0.5f, 1.0f); // Blue-Gray for sleeping
                } else if (body->IsStatic) {
                    bodyColor = glm::vec4(0.3f, 0.7f, 0.3f, 1.0f); // Green for static ground
                } else {
                    bodyColor = glm::vec4(0.9f, 0.6f, 0.2f, 1.0f); // Orange for awake active bodies
                }
            }

            // Draw the shape borders
            if (m_Flags & DRAW_SHAPE) {
                if (body->GetType() == ShapeType::CIRCLE) {
                    auto circle = std::static_pointer_cast<CircleShape>(body->GetShape());
                    m_Renderer->DrawBox(body->Position, glm::vec2(circle->GetRadius() * 2.0f), body->Rotation, bodyColor); // Simplified circle drawing
                } else if (body->GetType() == ShapeType::BOX) {
                    auto box = std::static_pointer_cast<BoxShape>(body->GetShape());
                    m_Renderer->DrawBox(body->Position, box->GetSize(), body->Rotation, bodyColor);
                }
            }

            // Draw Axis-Aligned Bounding Box (AABB) outline
            if (m_Flags & DRAW_AABB) {
                // AABB drawing logic
            }

            // Draw Velocity Vectors
            if ((m_Flags & DRAW_VELOCITY) && !body->IsStatic && !body->IsSleeping) {
                glm::vec2 start = glm::vec2(body->Position.x, body->Position.y);
                glm::vec2 end = start + glm::vec2(body->Velocity.x, body->Velocity.y) * 0.2f;
                
                m_Renderer->DrawLine(start, end, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
            }
        }

        // 2. Iterate through collision manifolds to show contact forces
        if (m_Flags & DRAW_CONTACTS) {
            for (const CollisionManifold& m : manifolds) {
                for (int i = 0; i < m.ContactCount; ++i) {
                    glm::vec2 contactPoint = glm::vec2(m.Contacts[i].x, m.Contacts[i].y);
                    
                    float crossSize = 0.15f;
                    m_Renderer->DrawLine(contactPoint - glm::vec2(crossSize, 0), contactPoint + glm::vec2(crossSize, 0), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
                    m_Renderer->DrawLine(contactPoint - glm::vec2(0, crossSize), contactPoint + glm::vec2(0, crossSize), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

                    glm::vec2 normalStart = contactPoint;
                    glm::vec2 normalEnd = normalStart + glm::vec2(m.Normal.x, m.Normal.y) * m.PenetrationDepth * 5.0f;
                    
                    m_Renderer->DrawLine(normalStart, normalEnd, glm::vec4(1.0f, 0.3f, 0.3f, 0.8f));
                }
            }
        }
    }
}
```
```
