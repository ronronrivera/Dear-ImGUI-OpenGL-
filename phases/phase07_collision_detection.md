# Phase 07: Collision Detection 💥

This phase sets up the collision pipeline: a Broadphase Spatial Hash Grid filters out non-overlapping bodies, and a Narrowphase Separating Axis Theorem (SAT) solver tests intersections and computes intersection depths and normals.

## 📁 Filenames & Directory Structure

* **Physics Subsystem**:
  * `include/physics/spatial_hash.hpp` & `src/physics/spatial_hash.cpp` — Broadphase Spatial Hash Grid filters out non-overlapping bodies.
  * `include/physics/collision.hpp` & `src/physics/collision.cpp` — Narrowphase formulas (Circle-Circle, Circle-Box, Box-Box) and contact point SAT routines.
  * `include/physics/manifold.hpp` — Data structure storing contact points, depth, and contact normals.

## ⚙️ Core Variables

### `AABB` Struct
* `Vec2 Min`, `Max` — Bounding coordinates.

### `CollisionManifold` Struct
* `RigidBody* BodyA`, `*BodyB` — Intersecting body pointers.
* `float PenetrationDepth` — Distance overlap.
* `Vec2 Normal` — Collision response vector pointing from A to B.
* `std::array<Vec2, 2> Contacts` — Contact point locations.
* `int ContactCount` — Number of contact points (1 or 2 for 2D rigid bodies).

### `SpatialHashGrid` Members
* `float CellSize` — Span size of grid cells (e.g. equal to average object diameter).
* `std::unordered_map<int, std::vector<RigidBody*>> Grid` — Map of grid cell keys to bodies.

## 🏛️ Engine Component Role

* **Broadphase**: Eliminates pairs that are too far apart, converting an $O(N^2)$ brute force search to a near-$O(N)$ lookup.
* **Narrowphase**: Resolves shape overlaps, outputting contact normal, penetration, and contact vertices needed by the impulse solver.

## 📝 Class Structures & Flow of Execution

### 1. `include/physics/manifold.hpp`
```cpp
#pragma once
#include "physics/rigid_body.hpp"
#include <array>

namespace Physix {
    struct SolverCache {
        Vec2 rA;
        Vec2 rB;
        float NormalMass = 0.0f;
        float TangentMass = 0.0f;
        float Bias = 0.0f;
        float AccumulatedNormalImpulse = 0.0f;
        float AccumulatedTangentImpulse = 0.0f;
    };

    struct CollisionManifold {
        RigidBody* BodyA = nullptr;
        RigidBody* BodyB = nullptr;
        float PenetrationDepth = 0.0f;
        Vec2 Normal;
        std::array<Vec2, 2> Contacts;
        int ContactCount = 0;

        std::array<SolverCache, 2> SolverCache;
    };
}
```

### 2. `include/physics/spatial_hash.hpp`
```cpp
#pragma once
#include "physics/rigid_body.hpp"
#include <vector>
#include <unordered_map>
#include <utility>

namespace Physix {
    struct AABB {
        Vec2 Min;
        Vec2 Max;
    };

    class SpatialHashGrid {
    public:
        SpatialHashGrid(float cellSize);
        ~SpatialHashGrid();

        void Update(const std::vector<RigidBody*>& bodies);
        std::vector<std::pair<RigidBody*, RigidBody*>> GetPairs() const;

    private:
        int GetCellKey(const Vec2& position) const;
        AABB GetBodyAABB(const RigidBody* body) const;

        float m_CellSize;
        std::unordered_map<int, std::vector<RigidBody*>> m_Grid;
    };
}
```

### 3. `src/physics/spatial_hash.cpp`
```cpp
#include "physics/spatial_hash.hpp"
#include <cmath>
#include <set>
#include <algorithm>

namespace Physix {
    SpatialHashGrid::SpatialHashGrid(float cellSize) : m_CellSize(cellSize) {}
    SpatialHashGrid::~SpatialHashGrid() {}

    int SpatialHashGrid::GetCellKey(const Vec2& position) const {
        int ix = static_cast<int>(std::floor(position.x / m_CellSize));
        int iy = static_cast<int>(std::floor(position.y / m_CellSize));
        // Primes blend keys to prevent collisions
        return ix * 73856093 ^ iy * 19349663;
    }

    AABB SpatialHashGrid::GetBodyAABB(const RigidBody* body) const {
        AABB aabb;
        if (body->GetType() == ShapeType::CIRCLE) {
            auto circle = std::static_pointer_cast<CircleShape>(body->GetShape());
            float r = circle->GetRadius();
            aabb.Min = body->Position - Vec2(r, r);
            aabb.Max = body->Position + Vec2(r, r);
        } else {
            auto box = std::static_pointer_cast<BoxShape>(body->GetShape());
            Vec2 halfSize = box->GetSize() * 0.5f;
            float r = halfSize.Length();
            aabb.Min = body->Position - Vec2(r, r);
            aabb.Max = body->Position + Vec2(r, r);
        }
        return aabb;
    }

    void SpatialHashGrid::Update(const std::vector<RigidBody*>& bodies) {
        m_Grid.clear();
        for (RigidBody* b : bodies) {
            AABB aabb = GetBodyAABB(b);
            int startX = static_cast<int>(std::floor(aabb.Min.x / m_CellSize));
            int endX   = static_cast<int>(std::floor(aabb.Max.x / m_CellSize));
            int startY = static_cast<int>(std::floor(aabb.Min.y / m_CellSize));
            int endY   = static_cast<int>(std::floor(aabb.Max.y / m_CellSize));

            for (int x = startX; x <= endX; ++x) {
                for (int y = startY; y <= endY; ++y) {
                    int key = GetCellKey(Vec2(static_cast<float>(x) * m_CellSize, static_cast<float>(y) * m_CellSize));
                    m_Grid[key].push_back(b);
                }
            }
        }
    }

    std::vector<std::pair<RigidBody*, RigidBody*>> SpatialHashGrid::GetPairs() const {
        std::vector<std::pair<RigidBody*, RigidBody*>> pairs;
        std::set<std::pair<RigidBody*, RigidBody*>> uniquePairs;

        for (auto& [key, list] : m_Grid) {
            if (list.size() < 2) continue;
            for (size_t i = 0; i < list.size(); ++i) {
                for (size_t j = i + 1; j < list.size(); ++j) {
                    RigidBody* a = list[i];
                    RigidBody* b = list[j];
                    if (a == b) continue;
                    if (a->IsStatic && b->IsStatic) continue;

                    auto pair = std::make_pair(std::min(a, b), std::max(a, b));
                    if (uniquePairs.find(pair) == uniquePairs.end()) {
                        uniquePairs.insert(pair);
                        pairs.push_back(pair);
                    }
                }
            }
        }
        return pairs;
    }
}
```

### 4. `include/physics/collision.hpp`
```cpp
#pragma once
#include "physics/rigid_body.hpp"
#include "physics/manifold.hpp"

namespace Physix {
    class Collision {
    public:
        static bool TestCollision(RigidBody* a, RigidBody* b, CollisionManifold& outManifold);

    private:
        static bool CircleToCircle(RigidBody* a, RigidBody* b, CollisionManifold& m);
        static bool CircleToBox(RigidBody* a, RigidBody* b, CollisionManifold& m);
        static bool BoxToBox(RigidBody* a, RigidBody* b, CollisionManifold& m);
        static void GenerateContactPoints(CollisionManifold& m);
    };
}
```

### 5. `src/physics/collision.cpp`
```cpp
#include "physics/collision.hpp"
#include <algorithm>
#include <limits>

namespace Physix {
    bool Collision::TestCollision(RigidBody* a, RigidBody* b, CollisionManifold& outManifold) {
        ShapeType typeA = a->GetType();
        ShapeType typeB = b->GetType();

        if (typeA == ShapeType::CIRCLE && typeB == ShapeType::CIRCLE) {
            return CircleToCircle(a, b, outManifold);
        }
        else if (typeA == ShapeType::CIRCLE && typeB == ShapeType::BOX) {
            return CircleToBox(a, b, outManifold);
        }
        else if (typeA == ShapeType::BOX && typeB == ShapeType::CIRCLE) {
            bool hit = CircleToBox(b, a, outManifold);
            if (hit) {
                outManifold.Normal = outManifold.Normal * -1.0f;
            }
            return hit;
        }
        else if (typeA == ShapeType::BOX && typeB == ShapeType::BOX) {
            return BoxToBox(a, b, outManifold);
        }
        return false;
    }

    bool Collision::CircleToCircle(RigidBody* a, RigidBody* b, CollisionManifold& m) {
        auto circleA = std::static_pointer_cast<CircleShape>(a->GetShape());
        auto circleB = std::static_pointer_cast<CircleShape>(b->GetShape());

        Vec2 diff = b->Position - a->Position;
        float distSq = diff.LengthSq();
        float rSum = circleA->GetRadius() + circleB->GetRadius();

        if (distSq > rSum * rSum) return false;

        float dist = std::sqrt(distSq);
        m.BodyA = a;
        m.BodyB = b;
        m.PenetrationDepth = rSum - dist;
        m.Normal = dist > 0.0001f ? diff * (1.0f / dist) : Vec2(1.0f, 0.0f);
        m.ContactCount = 1;
        m.Contacts[0] = a->Position + m.Normal * circleA->GetRadius();
        return true;
    }

    bool Collision::CircleToBox(RigidBody* circle, RigidBody* box, CollisionManifold& m) {
        auto circleShape = std::static_pointer_cast<CircleShape>(circle->GetShape());
        auto boxShape = std::static_pointer_cast<BoxShape>(box->GetShape());
        float r = circleShape->GetRadius();
        Vec2 halfSize = boxShape->GetSize() * 0.5f;

        // 1. Transform Circle Center into Box's local coordinate space
        Vec2 relativePos = circle->Position - box->Position;
        float theta = -box->Rotation;
        Vec2 localCircleCenter(
            relativePos.x * std::cos(theta) - relativePos.y * std::sin(theta),
            relativePos.x * std::sin(theta) + relativePos.y * std::cos(theta)
        );

        // 2. Find closest point on Box bounds to the local center
        Vec2 localClosestPoint(
            std::clamp(localCircleCenter.x, -halfSize.x, halfSize.x),
            std::clamp(localCircleCenter.y, -halfSize.y, halfSize.y)
        );

        // 3. Distance vector
        Vec2 localDiff = localCircleCenter - localClosestPoint;
        float distSq = localDiff.LengthSq();

        if (distSq > r * r) return false;

        float dist = std::sqrt(distSq);
        Vec2 localNormal;
        float depth;

        if (distSq > 0.0001f) {
            localNormal = localDiff * (1.0f / dist);
            depth = r - dist;
        } else {
            // Circle center inside the Box.
            float dx = halfSize.x - std::abs(localCircleCenter.x);
            float dy = halfSize.y - std::abs(localCircleCenter.y);
            
            if (dx < dy) {
                localNormal = Vec2(localCircleCenter.x > 0 ? 1.0f : -1.0f, 0.0f);
                depth = r + dx;
            } else {
                localNormal = Vec2(0.0f, localCircleCenter.y > 0 ? 1.0f : -1.0f);
                depth = r + dy;
            }
        }

        // 4. Transform local coordinates back to world space
        m.BodyA = circle;
        m.BodyB = box;
        m.PenetrationDepth = depth;
        
        float cosTheta = std::cos(box->Rotation);
        float sinTheta = std::sin(box->Rotation);
        m.Normal = Vec2(
            localNormal.x * cosTheta - localNormal.y * sinTheta,
            localNormal.x * sinTheta + localNormal.y * cosTheta
        ) * -1.0f; // Point Normal from A to B

        m.ContactCount = 1;
        m.Contacts[0] = circle->Position + m.Normal * r;
        return true;
    }

    bool Collision::BoxToBox(RigidBody* a, RigidBody* b, CollisionManifold& m) {
        // Separating Axis Theorem (SAT) for Box vs Box stub
        return false;
    }

    void Collision::GenerateContactPoints(CollisionManifold& m) {
        // Multi-point clipping calculations
    }
}
```
