# Phase 01: Window & Context 🎮

This phase initializes the application's base window and configures OpenGL 3.3 Core Profile.

## 📁 Filenames & Directory Structure

To keep the engine modular, we avoid putting all initialization and game loop logic into `main.cpp`. Instead, we decouple the window management and application loop:

* **Entry Point**: `src/main.cpp` (a clean main function that instantiates and runs the `App`)
* **Core Subsystem**:
  * `include/core/window.hpp` — Header declaring GLFW window wrapper, resizing, and input callbacks.
  * `src/core/window.cpp` — Implementation of GLFW window initialization and context creation.
  * `include/core/app.hpp` — Header declaring the main `App` engine class.
  * `src/core/app.cpp` — Implementation of the main loop and coordinator of updates and rendering.
* **OpenGL Headers**: `tools/glad/glad.h`, `tools/glad.c`

## ⚙️ Core Variables

### `Window` Class Members (`include/core/window.hpp`)
* `GLFWwindow* m_GLFWWindow` — Handles window events, keyboard/mouse capture, and context swap buffers.
* `int m_Width = 800` — Window frame width.
* `int m_Height = 600` — Window frame height.
* `float m_Scale = 1.0f` — DPI scaling factor for HighDPI displays.

### `App` Class Members (`include/core/app.hpp`)
* `std::unique_ptr<Window> m_Window` — The window object managing our OpenGL context.
* `bool m_Running = true` — Flag to control the main loop execution.

## 🏛️ Engine Component Role

* **Window Manager**: Handles communication with the Operating System window server via GLFW.
* **OpenGL Loader**: GLAD retrieves driver-specific OpenGL function pointers.
* **Application Loop**: Coordinates updating simulation state (physics clock tick) and redrawing the screen.

## 📝 Class Structures & Flow of Execution

### 1. `include/core/window.hpp`
```cpp
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Physix {
    class Window {
    public:
        Window(int width, int height, const char* title);
        ~Window();

        bool ShouldClose() const;
        void SwapBuffers();
        void PollEvents();

        GLFWwindow* GetGLFWWindow() const { return m_GLFWWindow; }
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        float GetScale() const { return m_Scale; }

    private:
        GLFWwindow* m_GLFWWindow;
        int m_Width;
        int m_Height;
        float m_Scale;
    };
}
```

### 2. `src/core/window.cpp`
```cpp
#include "core/window.hpp"
#include <iostream>

namespace Physix {
    Window::Window(int width, int height, const char* title) : m_Width(width), m_Height(height), m_Scale(1.0f) {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(-1);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_GLFWWindow = glfwCreateWindow(m_Width, m_Height, title, nullptr, nullptr);
        if (!m_GLFWWindow) {
            std::cerr << "Failed to create GLFW Window" << std::endl;
            glfwTerminate();
            exit(-1);
        }
        glfwMakeContextCurrent(m_GLFWWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to load OpenGL pointers" << std::endl;
            exit(-1);
        }

        // Configure Viewport
        glViewport(0, 0, m_Width, m_Height);
        glfwSetWindowUserPointer(m_GLFWWindow, this);
        
        // Handle resizing
        glfwSetFramebufferSizeCallback(m_GLFWWindow, [](GLFWwindow* w, int width, int height) {
            auto* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
            window->m_Width = width;
            window->m_Height = height;
            glViewport(0, 0, width, height);
        });

        // Determine Scale/DPI (optional placeholder for ImGui fonts)
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        if (primaryMonitor) {
            float xscale, yscale;
            glfwGetMonitorContentScale(primaryMonitor, &xscale, &yscale);
            m_Scale = xscale;
        }
    }

    Window::~Window() {
        glfwDestroyWindow(m_GLFWWindow);
        glfwTerminate();
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(m_GLFWWindow);
    }

    void Window::SwapBuffers() {
        glfwSwapBuffers(m_GLFWWindow);
    }

    void Window::PollEvents() {
        glfwPollEvents();
    }
}
```

### 3. `include/core/app.hpp`
```cpp
#pragma once
#include "core/window.hpp"
#include <memory>

namespace Physix {
    class App {
    public:
        App();
        ~App();

        void Run();

    private:
        std::unique_ptr<Window> m_Window;
        bool m_Running;
    };
}
```

### 4. `src/core/app.cpp`
```cpp
#include "core/app.hpp"

namespace Physix {
    App::App() : m_Running(true) {
        m_Window = std::make_unique<Window>(800, 600, "Physix Engine");
    }

    App::~App() {}

    void App::Run() {
        while (m_Running && !m_Window->ShouldClose()) {
            m_Window->PollEvents();

            // Clear Screen
            glClearColor(0.1f, 0.1f, 0.14f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // TODO: Update physics, render frame, draw UI

            m_Window->SwapBuffers();
        }
    }
}
```

### 5. `src/main.cpp`
```cpp
#include "core/app.hpp"

int main() {
    Physix::App app;
    app.Run();
    return 0;
}
```
