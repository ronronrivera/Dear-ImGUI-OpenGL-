# Phase 04: Camera 🎥

This phase implements a 3D perspective camera positioned relative to the 2D XY physics simulation plane (Z = 0), allowing orbit, zoom, pan, and mouse picking (converting screen coordinates to world coordinates).

## 📁 Filenames & Directory Structure

* **Camera Engine**:
  * `include/renderer/camera.hpp` & `src/renderer/camera.cpp` — Stores views, orientation, and mouse projection matrices.
* **Input Hooking**:
  * `include/core/input_manager.hpp` & `src/core/input_manager.cpp` — Filters raw mouse/key events, hookable by window callback.

## ⚙️ Core Variables

### `Camera` State
* `glm::vec3 m_Position` — World position.
* `glm::vec3 m_Target` — Look-at target (anchor point on XY plane).
* `glm::vec3 m_Up`, `m_Right`, `m_Front` — Direction vectors.
* `float m_Yaw` — Orbit angle around the vertical axis.
* `float m_Pitch` — Elevation angle.
* `float m_Distance` — Radius of the orbit sphere around the target.
* `float m_FOV` — Field of view (default: `45.0f` degrees).
* `float m_AspectRatio` — Width / Height ratio matching the ImGui Viewport panel.

## 🏛️ Engine Component Role

* **Perspective Projector**: Generates view and projection matrices to represent 2D physics simulations with a 3D sense of depth.
* **Raycaster**: Translates 2D screen positions from ImGui panels into 3D rays that intersect the physics plane.

## 📝 Class Structures & Flow of Execution

### 1. `include/renderer/camera.hpp`
```cpp
#pragma once
#include <glm/glm.hpp>

namespace Physix {
    class Camera {
    public:
        Camera(glm::vec3 position, glm::vec3 target);
        ~Camera();

        void UpdateVectors();
        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;

        void ProcessMouseScroll(float yOffset);
        void ProcessPan(float deltaX, float deltaY);
        void ProcessRotate(float deltaX, float deltaY);

        glm::vec2 ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& viewportSize);

        glm::vec3 GetPosition() const { return m_Position; }
        void SetAspectRatio(float aspect) { m_AspectRatio = aspect; }

    private:
        glm::vec3 m_Position;
        glm::vec3 m_Target;
        glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };
        glm::vec3 m_Right = { 1.0f, 0.0f, 0.0f };
        glm::vec3 m_Front = { 0.0f, 0.0f, -1.0f };
        float m_Yaw = -90.0f;
        float m_Pitch = 0.0f;
        float m_Distance = 10.0f;
        float m_FOV = 45.0f;
        float m_AspectRatio = 1.33f;
    };
}
```

### 2. `src/renderer/camera.cpp`
```cpp
#include "renderer/camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

namespace Physix {
    Camera::Camera(glm::vec3 position, glm::vec3 target) : m_Position(position), m_Target(target) {
        UpdateVectors();
    }

    Camera::~Camera() {}

    void Camera::UpdateVectors() {
        // Spherical to Cartesian coordinate transformation for Orbit camera
        m_Position.x = m_Target.x + m_Distance * cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));
        m_Position.y = m_Target.y + m_Distance * sin(glm::radians(m_Pitch));
        m_Position.z = m_Target.z + m_Distance * cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw));

        m_Front = glm::normalize(m_Target - m_Position);
        m_Right = glm::normalize(glm::cross(m_Front, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
    }

    glm::mat4 Camera::GetViewMatrix() const {
        return glm::lookAt(m_Position, m_Target, m_Up);
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return glm::perspective(glm::radians(m_FOV), m_AspectRatio, 0.1f, 1000.0f);
    }

    void Camera::ProcessMouseScroll(float yOffset) {
        m_Distance -= yOffset * 2.0f;
        if (m_Distance < 1.0f) m_Distance = 1.0f;
        UpdateVectors();
    }

    void Camera::ProcessPan(float deltaX, float deltaY) {
        // Calculate translate factor based on camera distance to make movement smooth
        float speed = m_Distance * 0.001f;
        m_Target -= m_Right * deltaX * speed;
        m_Target += m_Up * deltaY * speed;
        UpdateVectors();
    }

    void Camera::ProcessRotate(float deltaX, float deltaY) {
        m_Yaw   += deltaX * 0.25f;
        m_Pitch += deltaY * 0.25f;
        m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
        UpdateVectors();
    }

    glm::vec2 Camera::ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& viewportSize) {
        // 1. Normalized Device Coordinates (NDC) [-1 to 1]
        float x = (2.0f * screenPos.x) / viewportSize.x - 1.0f;
        float y = 1.0f - (2.0f * screenPos.y) / viewportSize.y;
        
        glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
        
        // 2. Homogeneous Eye Coordinates
        glm::mat4 invProj = glm::inverse(GetProjectionMatrix());
        glm::vec4 rayEye = invProj * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
        
        // 3. World Coordinates
        glm::mat4 invView = glm::inverse(GetViewMatrix());
        glm::vec3 rayDir = glm::normalize(glm::vec3(invView * rayEye));
        glm::vec3 rayOrigin = m_Position;
        
        // 4. Ray-Plane intersection: Ray = origin + t * dir, Plane: Z = 0
        if (std::abs(rayDir.z) > 0.0001f) {
            float t = -rayOrigin.z / rayDir.z;
            glm::vec3 intersection = rayOrigin + t * rayDir;
            return glm::vec2(intersection.x, intersection.y);
        }
        
        return glm::vec2(0.0f);
    }
}
```

### 3. `include/core/input_manager.hpp`
```cpp
#pragma once
#include <GLFW/glfw3.h>

namespace Physix {
    class InputManager {
    public:
        InputManager();
        ~InputManager();

        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        static bool IsKeyPressed(int key);
        static bool IsMouseButtonPressed(int button);
        static void GetMousePosition(double& x, double& y);

    private:
        static bool s_Keys[GLFW_KEY_LAST];
        static bool s_MouseButtons[GLFW_MOUSE_BUTTON_LAST];
        static double s_MouseX;
        static double s_MouseY;
    };
}
```

### 4. `src/core/input_manager.cpp`
```cpp
#include "core/input_manager.hpp"

namespace Physix {
    bool InputManager::s_Keys[GLFW_KEY_LAST] = { false };
    bool InputManager::s_MouseButtons[GLFW_MOUSE_BUTTON_LAST] = { false };
    double InputManager::s_MouseX = 0.0;
    double InputManager::s_MouseY = 0.0;

    InputManager::InputManager() {}
    InputManager::~InputManager() {}

    void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key >= 0 && key < GLFW_KEY_LAST) {
            if (action == GLFW_PRESS) s_Keys[key] = true;
            else if (action == GLFW_RELEASE) s_Keys[key] = false;
        }
    }

    void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (button >= 0 && button < GLFW_MOUSE_BUTTON_LAST) {
            if (action == GLFW_PRESS) s_MouseButtons[button] = true;
            else if (action == GLFW_RELEASE) s_MouseButtons[button] = false;
        }
    }

    void InputManager::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
        s_MouseX = xpos;
        s_MouseY = ypos;
    }

    void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        // Scroll event capture logic here if needed
    }

    bool InputManager::IsKeyPressed(int key) {
        return key >= 0 && key < GLFW_KEY_LAST ? s_Keys[key] : false;
    }

    bool InputManager::IsMouseButtonPressed(int button) {
        return button >= 0 && button < GLFW_MOUSE_BUTTON_LAST ? s_MouseButtons[button] : false;
    }

    void InputManager::GetMousePosition(double& x, double& y) {
        x = s_MouseX;
        y = s_MouseY;
    }
}
```
