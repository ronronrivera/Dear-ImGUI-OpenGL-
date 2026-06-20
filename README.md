# Road to Physics: Learning OpenGL & Dear ImGui 🚀

Welcome to my learning repository! This project serves as the foundational sandbox where I am learning **OpenGL** and **Dear ImGui** in C++ from scratch. 

The ultimate goal of this journey is to acquire the necessary graphics, windowing, and GUI tooling skills to build and visualize a custom **Physics Engine** with real-time interactive parameter control.

---

## 🎯 The Vision
Before jumping straight into collision algorithms, rigid body dynamics, and constraints, a robust visualization and debugging interface is essential. 

Using **OpenGL** gives direct access to hardware-accelerated rendering, while **Dear ImGui** provides a powerful, bloat-free immediate mode graphical user interface. Together, they will allow me to:
- Render physical elements (particles, bounds, rigid bodies, contact points, and vectors).
- Adjust physics variables dynamically in real-time (gravity, friction, coefficient of restitution, time-step).
- Inspect entity details, frame rates, solver iterations, and engine performance metrics.
- Pause, step-frame, and reset the simulation workspace.

---

## 🗺️ Roadmap & Milestones

### **Phase 1: Windowing & Render Loop (Current)**
- [x] Initialize GLFW and load OpenGL function pointers with GLAD.
- [x] Create a viewport-resizable window context.
- [x] Basic game loop and screen clearing.

### **Phase 2: Dear ImGui Integration**
- [x] Initialize ImGui contexts for GLFW and OpenGL 3.
- [x] Set up the ImGui render frame within the main loop.
- [x] Create a basic control panel window to change the viewport background color.

### **Phase 3: 2D Render Abstraction**
- [ ] Write vertex and fragment shaders.
- [ ] Set up VAO, VBO, and EBO abstractions for shapes (circles, boxes, lines).
- [ ] Add projection matrix support to map world space coordinates to pixel space coordinates.

### **Phase 4: Physics Engine Foundations**
- [ ] Design structural math types (`Vector2D`, `Matrix2x2`, `Matrix3x3`, etc.).
- [ ] Implement an explicit Euler or Verlet integration loop.
- [ ] Simulate particle physics (gravity, drag, wind, and basic boundaries).

### **Phase 5: Rigid Body & Collision Solver**
- [ ] Define rigid body shapes (circles, convex polygons).
- [ ] Add collision detection (Broad phase spatial hashing + Narrow phase SAT/GJK).
- [ ] Implement impulse resolution for elastic and inelastic collisions.
- [ ] Add visual contact point debugging.

---

## 📂 Project Structure

```text
├── src/
│   └── main.cpp           # Main entry point (GLFW window & OpenGL setup)
├── tools/
│   ├── glad.c             # OpenGL 3.3 loader source
│   ├── glad/ & KHR/       # OpenGL loader header dependencies
│   └── imgui/             # Dear ImGui library source & headers (v1.xx)
│       └── backends/      # ImGui GLFW and OpenGL3 bindings
├── Makefile               # GNU Make script configuration
└── README.md              # Project documentation
```

---

## 🛠️ Build and Setup

### Prerequisites
To build and run this project, make sure you have standard development tools and GLFW installed:

On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential pkg-config libglfw3-dev libgl1-mesa-dev
```

### Building the Project
The project uses a custom [Makefile](Makefile) to compile the source code along with GLAD and Dear ImGui.

1. **Compile the program:**
   ```bash
   make
   ```
   This compiles all dependencies (including ImGui and GLAD source files) and creates the executable `imgui_tutorial`.

2. **Run the executable:**
   ```bash
   ./imgui_tutorial
   ```

3. **Clean build files:**
   ```bash
   make clean
   ```

---

## 📚 References & Resources
- [Learn OpenGL](https://learnopengl.com/) - Excellent tutorial for understanding graphics fundamentals.
- [Dear ImGui Repository](https://github.com/ocornut/imgui) - Library documentation and demo window reference.
- [Game Physics Cookbook](https://github.com/moth3r/Game-Physics-Cookbook) & [Erin Catto's Physics Lectures](https://box2d.org/publications/) - Inspiration and math behind modern 2D physics engines.
