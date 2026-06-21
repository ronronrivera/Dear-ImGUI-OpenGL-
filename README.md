# Physix 🧪

A 2.5D physics engine built from scratch in C++ and OpenGL. 2D rigid body simulation — collision detection, impulse resolution, and fluid sim — viewed through a 3D perspective camera with a Blender-inspired Dear ImGui editor. Built as a hands-on way to learn physics by breaking it in real time.

---

## 🗺️ Track Record

### **Phase 1: Window & Context**
- [x] GLFW window + OpenGL 3.3 core context
- [x] GLAD function pointer loading
- [x] Basic game loop and screen clearing

### **Phase 2: Dear ImGui Shell**
- [x] ImGui GLFW + OpenGL3 backends
- [x] Blender-inspired layout — left panel, viewport, right panel, bottom bar
- [x] Framebuffer Object — physics scene renders into ImGui viewport as a texture

### **Phase 3: 2D Render Abstraction**
- [ ] Vertex + fragment shaders (flat, debug, terrain, fluid)
- [ ] VAO/VBO abstractions for circles, boxes, lines
- [ ] Instanced rendering for large body counts
- [ ] Projection + view matrix pipeline via GLM

### **Phase 4: Camera**
- [ ] 3D perspective camera on the XY physics plane
- [ ] Orbit, pan, zoom controls
- [ ] Mouse picking — ray cast from screen to world space

### **Phase 5: Math & Physics Foundation**
- [ ] `Vec2` — position, velocity, force math for the physics layer
- [ ] `Clock` — fixed timestep with accumulator
- [ ] `RigidBody` — pos, vel, mass, shape, material, sleep state
- [ ] `World` — simulation root, owns all bodies and drives the tick

### **Phase 6: Integrator**
- [ ] Semi-implicit Euler integration
- [ ] Gravity, damping, force accumulation
- [ ] Fixed timestep loop with accumulator

### **Phase 7: Collision Detection**
- [ ] Broadphase — spatial hash grid, AABB pairs
- [ ] Narrowphase — SAT for boxes, circle-circle, circle-box
- [ ] Manifold generation — normal, depth, contact points

### **Phase 8: Impulse Resolution**
- [ ] Sequential impulse solver
- [ ] Restitution (elastic + inelastic)
- [ ] Friction
- [ ] Contact point debug draw

### **Phase 9: Procedural Generation**
- [ ] Perlin noise heightmap terrain
- [ ] Static terrain body generation
- [ ] Random object spawner with layout patterns
- [ ] Regenerate on demand from UI

### **Phase 10: Fluid Simulation**
- [ ] SPH (Smoothed Particle Hydrodynamics) particles
- [ ] Pressure + viscosity forces
- [ ] Fluid-rigid body interaction
- [ ] Fluid debug draw

### **Phase 11: Physics Problem Scenarios**
- [ ] Projectile motion
- [ ] Pendulum chain
- [ ] Fluid pressure
- [ ] Stack collapse
- [ ] Inclined plane

### **Phase 12: Debug Draw**
- [ ] AABB overlay
- [ ] Contact normals
- [ ] Velocity vectors
- [ ] Sleep state indicators

---

## 📂 Project Structure

To keep the codebase clean, modular, and maintainable, files are organized into designated subsystem folders separating headers (`include/`) and source files (`src/`):

```text
Physix/
├── include/               # All header files (.hpp)
│   ├── core/              # Core lifecycle (App, Window, Clock, InputManager, MathUtils)
│   ├── renderer/          # Shaders, Camera, Buffers, Framebuffer, Renderer2D, DebugDraw
│   ├── physics/           # RigidBody, World, Integrator, SAT Collision, Impulse Solver, SPH Fluid
│   ├── procgen/           # TerrainGen, ObjectSpawner, ProblemFactory scenario preset definitions
│   └── ui/                # ImGui workspace, viewports, panels, inspectors
│
├── src/                   # All C++ implementations (.cpp) matching include/ structure
│   ├── core/
│   ├── renderer/
│   ├── physics/
│   ├── procgen/
│   ├── ui/
│   └── main.cpp           # Thin entry point instantiating and running the App
│
├── shaders/               # GLSL source files (flat shaders, fluid rendering)
│
├── tools/                 # External dependencies (ImGui, GLAD, stb_perlin.h)
│   ├── imgui/
│   ├── glad/
│   └── stb/
│
└── Makefile               # Compiler and linker configuration
```

---

## 🛠️ Build & Run

### Prerequisites

```bash
sudo apt update
sudo apt install build-essential pkg-config libglfw3-dev libgl1-mesa-dev
```

### Build

```bash
make        # compile
./physix    # run
make clean  # clean build files
```

