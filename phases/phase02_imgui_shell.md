# Phase 02: Dear ImGui Shell 🖼️

This phase introduces a multi-panel Editor layout mimicking professional tools like Blender, utilizing a central OpenGL **Framebuffer Object (FBO)** texture to display the rendered physics scene within an ImGui window.

## 📁 Filenames & Directory Structure

* **UI Controllers**:
  * `include/ui/imgui_layer.hpp` & `src/ui/imgui_layer.cpp` — Main layout setup, starts/ends ImGui frames.
  * `include/ui/viewport_panel.hpp` & `src/ui/viewport_panel.cpp` — ImGui wrapper displaying the FBO viewport.
* **OpenGL Render Target**:
  * `include/renderer/framebuffer.hpp` & `src/renderer/framebuffer.cpp` — Allocates, resizes, and binds the FBO.

## ⚙️ Core Variables

### `Framebuffer` Class Members
* `unsigned int m_FboID` — OpenGL Framebuffer Object ID.
* `unsigned int m_TextureID` — Color attachment texture ID.
* `unsigned int m_RboID` — Depth/stencil renderbuffer ID.
* `int m_Width, m_Height` — Dimensions of the off-screen texture.

### `ViewportPanel` Class Members
* `ImVec2 m_PanelSize` — Size of the ImGui viewport window in screen coordinates.
* `bool m_IsFocused` — Tracks whether mouse inputs should target camera/picking or the UI controls.

## 🏛️ Engine Component Role

* **UI Panel Engine**: Draws docking nodes, layouts, inspector fields, and menus.
* **FBO Renderer**: Directs the OpenGL draw calls away from the default monitor window backbuffer and into a texture. This keeps the rendering system separate from user interface buttons.

## 📝 Class Structures & Flow of Execution

### 1. `include/renderer/framebuffer.hpp`
```cpp
#pragma once

namespace Physix {
    class Framebuffer {
    public:
        Framebuffer(int width, int height);
        ~Framebuffer();

        void Create(int width, int height);
        void Destroy();
        void Resize(int width, int height);

        void Bind() const;
        void Unbind() const;

        unsigned int GetFboID() const { return m_FboID; }
        unsigned int GetTextureID() const { return m_TextureID; }
        unsigned int GetRboID() const { return m_RboID; }
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

    private:
        unsigned int m_FboID = 0;
        unsigned int m_TextureID = 0;
        unsigned int m_RboID = 0;
        int m_Width = 0;
        int m_Height = 0;
    };
}
```

### 2. `src/renderer/framebuffer.cpp`
```cpp
#include "renderer/framebuffer.hpp"
#include <glad/glad.h>
#include <iostream>

namespace Physix {
    Framebuffer::Framebuffer(int width, int height) {
        Create(width, height);
    }

    Framebuffer::~Framebuffer() {
        Destroy();
    }

    void Framebuffer::Create(int width, int height) {
        m_Width = width;
        m_Height = height;

        // Generate FBO
        glGenFramebuffers(1, &m_FboID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);

        // Create Color Attachment Texture
        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

        // Create Depth/Stencil Renderbuffer (for 3D Camera depth testing)
        glGenRenderbuffers(1, &m_RboID);
        glBindRenderbuffer(GL_RENDERBUFFER, m_RboID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RboID);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::Destroy() {
        if (m_FboID) glDeleteFramebuffers(1, &m_FboID);
        if (m_TextureID) glDeleteTextures(1, &m_TextureID);
        if (m_RboID) glDeleteRenderbuffers(1, &m_RboID);
        m_FboID = 0;
        m_TextureID = 0;
        m_RboID = 0;
    }

    void Framebuffer::Resize(int width, int height) {
        Destroy();
        Create(width, height);
    }

    void Framebuffer::Bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
    }

    void Framebuffer::Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
```

### 3. `include/ui/imgui_layer.hpp`
```cpp
#pragma once
#include <GLFW/glfw3.h>

namespace Physix {
    class ImGuiLayer {
    public:
        ImGuiLayer(GLFWwindow* window);
        ~ImGuiLayer();

        void Begin();
        void End();
    };
}
```

### 4. `src/ui/imgui_layer.cpp`
```cpp
#include "ui/imgui_layer.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Physix {
    ImGuiLayer::ImGuiLayer(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    ImGuiLayer::~ImGuiLayer() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End() {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
}
```

### 5. `include/ui/viewport_panel.hpp`
```cpp
#pragma once
#include "renderer/framebuffer.hpp"
#include <imgui/imgui.h>

namespace Physix {
    class ViewportPanel {
    public:
        ViewportPanel();
        ~ViewportPanel();

        void Render(Framebuffer& framebuffer);

        ImVec2 GetSize() const { return m_PanelSize; }
        bool IsFocused() const { return m_IsFocused; }
        bool IsHovered() const { return m_IsHovered; }

    private:
        ImVec2 m_PanelSize = { 0.0f, 0.0f };
        bool m_IsFocused = false;
        bool m_IsHovered = false;
    };
}
```

### 6. `src/ui/viewport_panel.cpp`
```cpp
#include "ui/viewport_panel.hpp"
#include <imgui/imgui.h>

namespace Physix {
    ViewportPanel::ViewportPanel() {}
    ViewportPanel::~ViewportPanel() {}

    void ViewportPanel::Render(Framebuffer& framebuffer) {
        ImGui::Begin("Physics Viewport");

        ImVec2 currentViewportSize = ImGui::GetContentRegionAvail();
        m_PanelSize = currentViewportSize;

        // Handle resizing if the panel size changed
        if (currentViewportSize.x != framebuffer.GetWidth() || currentViewportSize.y != framebuffer.GetHeight()) {
            framebuffer.Resize((int)currentViewportSize.x, (int)currentViewportSize.y);
        }

        // Display texture (OpenGL texture coordinates are Y-inverted)
        ImGui::Image((void*)(intptr_t)framebuffer.GetTextureID(), currentViewportSize, ImVec2(0, 1), ImVec2(1, 0));

        m_IsFocused = ImGui::IsWindowFocused();
        m_IsHovered = ImGui::IsWindowHovered();

        ImGui::End();
    }
}
```

### 7. Main Render Loop Integration
```cpp
// Within App::Update or Main Loop:

// Step A: Draw physics scene into Framebuffer
m_Framebuffer.Bind();
glViewport(0, 0, m_Framebuffer.GetWidth(), m_Framebuffer.GetHeight());
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

m_Renderer.RenderScene(m_World, m_Camera);

m_Framebuffer.Unbind(); // Directs rendering back to screen backbuffer

// Step B: Setup ImGui layout Dockspace
m_ImGuiLayer.Begin();

// DockSpace setup (creating a full-screen background dock space)
ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

// Step C: Left Panel - Hierarchy/Presets
ImGui::Begin("Presets & Problems");
if (ImGui::Button("Spawn Projectile Scenario")) {
    LoadScenario(PROJECTILE);
}
ImGui::End();

// Step D: Right Panel - Inspector
ImGui::Begin("Inspector");
ImGui::DragFloat2("Gravity Offset", &m_World.Gravity.x, 0.1f);
ImGui::End();

// Step E: Viewport Panel (displays the FBO texture)
m_ViewportPanel.Render(m_Framebuffer);

// Step F: Draw final UI
m_ImGuiLayer.End();
```
