# Phase 03: 2D Render Abstraction 🖌️

This phase builds the low-level rendering abstractions to draw 2D primitives (circles, boxes, lines) efficiently, decoupling them from direct OpenGL calls in the main logic.

## 📁 Filenames & Directory Structure

* **OpenGL Wrappers**:
  * `include/renderer/shader.hpp` & `src/renderer/shader.cpp` — Compiles and manages GLSL uniforms.
  * `include/renderer/vertex_array.hpp` & `src/renderer/vertex_array.cpp` — Vertex array layouts.
  * `include/renderer/vertex_buffer.hpp` & `src/renderer/vertex_buffer.cpp` — Raw vertex buffer memory.
  * `include/renderer/index_buffer.hpp` & `src/renderer/index_buffer.cpp` — Index (element) buffers.
* **Primitive Rendering Engine**:
  * `include/renderer/renderer2d.hpp` & `src/renderer/renderer2d.cpp` — Manages batching / instanced draw calls.
* **GLSL Shader Sources**:
  * `shaders/flat.vs` — Vertex shader handling translations, rotations, and projections.
  * `shaders/flat.fs` — Fragment shader applying color/wireframe values.

## ⚙️ Core Variables

### `Vertex` Struct
* `glm::vec2 Position` — Object coordinates.
* `glm::vec4 Color` — RGBA tint.

### `Renderer2D` Members
* `std::shared_ptr<Shader> m_FlatShader` — Standard solid-color primitive shader.
* `unsigned int m_QuadVAO, m_QuadVBO, m_QuadEBO` — Shared quad geometry used for instanced drawing of boxes and fluid particles.
* `unsigned int m_LineVAO, m_LineVBO` — Dynamic buffers for debug lines (re-allocated per frame).
* `std::vector<Vertex> m_LineVertexBuffer` — Local cache containing queued line primitives.

## 🏛️ Engine Component Role

* **Batch/Instance Shader Engine**: Groups shapes by material or geometry configuration to minimize draw calls.
* **Uniform Manager**: Updates transformation matrices (View, Projection) inside GPU memory.

## 📝 Class Structures & Flow of Execution

### 1. `include/renderer/shader.hpp`
```cpp
#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Physix {
    class Shader {
    public:
        Shader(const char* vertexPath, const char* fragmentPath);
        ~Shader();

        void Bind() const;
        void Unbind() const;

        void SetMat4(const std::string& name, const glm::mat4& matrix) const;
        void SetVec4(const std::string& name, const glm::vec4& value) const;

    private:
        unsigned int m_RendererID;
    };
}
```

### 2. `src/renderer/shader.cpp`
```cpp
#include "renderer/shader.hpp"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Physix {
    Shader::Shader(const char* vertexPath, const char* fragmentPath) {
        // Simple file reader, compile vertex + fragment shader and link program.
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        
        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        } catch (std::ifstream::failure& e) {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        }
        
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        
        unsigned int vertex, fragment;
        int success;
        char infoLog[512];
        
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        };
        
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        };
        
        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertex);
        glAttachShader(m_RendererID, fragment);
        glLinkProgram(m_RendererID);
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_RendererID, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    Shader::~Shader() {
        glDeleteProgram(m_RendererID);
    }

    void Shader::Bind() const {
        glUseProgram(m_RendererID);
    }

    void Shader::Unbind() const {
        glUseProgram(0);
    }

    void Shader::SetMat4(const std::string& name, const glm::mat4& matrix) const {
        int location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
    }

    void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
        int location = glGetUniformLocation(m_RendererID, name.c_str());
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}
```

### 3. `include/renderer/vertex_buffer.hpp`
```cpp
#pragma once

namespace Physix {
    class VertexBuffer {
    public:
        VertexBuffer(const void* data, unsigned int size, bool dynamic = false);
        ~VertexBuffer();

        void Bind() const;
        void Unbind() const;
        void SetData(const void* data, unsigned int size);

    private:
        unsigned int m_RendererID;
    };
}
```

### 4. `src/renderer/vertex_buffer.cpp`
```cpp
#include "renderer/vertex_buffer.hpp"
#include <glad/glad.h>

namespace Physix {
    VertexBuffer::VertexBuffer(const void* data, unsigned int size, bool dynamic) {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void VertexBuffer::Bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void VertexBuffer::Unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::SetData(const void* data, unsigned int size) {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }
}
```

### 5. `include/renderer/index_buffer.hpp`
```cpp
#pragma once

namespace Physix {
    class IndexBuffer {
    public:
        IndexBuffer(const unsigned int* indices, unsigned int count);
        ~IndexBuffer();

        void Bind() const;
        void Unbind() const;

        unsigned int GetCount() const { return m_Count; }

    private:
        unsigned int m_RendererID;
        unsigned int m_Count;
    };
}
```

### 6. `src/renderer/index_buffer.cpp`
```cpp
#include "renderer/index_buffer.hpp"
#include <glad/glad.h>

namespace Physix {
    IndexBuffer::IndexBuffer(const unsigned int* indices, unsigned int count) : m_Count(count) {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void IndexBuffer::Bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void IndexBuffer::Unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
```

### 7. `include/renderer/vertex_array.hpp`
```cpp
#pragma once
#include <memory>
#include "renderer/vertex_buffer.hpp"
#include "renderer/index_buffer.hpp"

namespace Physix {
    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        void Bind() const;
        void Unbind() const;

        void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer);
        void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

    private:
        unsigned int m_RendererID;
    };
}
```

### 8. `src/renderer/vertex_array.cpp`
```cpp
#include "renderer/vertex_array.hpp"
#include <glad/glad.h>

namespace Physix {
    VertexArray::VertexArray() {
        glGenVertexArrays(1, &m_RendererID);
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    void VertexArray::Bind() const {
        glBindVertexArray(m_RendererID);
    }

    void VertexArray::Unbind() const {
        glBindVertexArray(0);
    }

    void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
        Bind();
        vertexBuffer->Bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    }

    void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
        Bind();
        indexBuffer->Bind();
    }
}
```

### 9. `include/renderer/renderer2d.hpp`
```cpp
#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "renderer/shader.hpp"

namespace Physix {
    struct Vertex {
        glm::vec2 Position;
        glm::vec4 Color;
    };

    class Renderer2D {
    public:
        Renderer2D();
        ~Renderer2D();

        void Init();
        void Shutdown();

        void BeginScene(const glm::mat4& viewProjMatrix);
        void EndScene();

        void DrawBox(const glm::vec2& position, const glm::vec2& size, float angle, const glm::vec4& color);
        void DrawLine(const glm::vec2& p0, const glm::vec2& p1, const glm::vec4& color);

    private:
        static const size_t MAX_LINE_VERTICES = 2000;

        std::shared_ptr<Shader> m_FlatShader;
        std::shared_ptr<Shader> m_LineShader;

        unsigned int m_QuadVAO = 0;
        unsigned int m_QuadVBO = 0;
        unsigned int m_QuadEBO = 0;

        unsigned int m_LineVAO = 0;
        unsigned int m_LineVBO = 0;

        std::vector<Vertex> m_LineVertexBuffer;
        glm::mat4 m_ViewProjCache;
    };
}
```

### 10. `src/renderer/renderer2d.cpp`
```cpp
#include "renderer/renderer2d.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Physix {
    Renderer2D::Renderer2D() {}
    Renderer2D::~Renderer2D() {
        Shutdown();
    }

    void Renderer2D::Init() {
        m_FlatShader = std::make_shared<Shader>("shaders/flat.vs", "shaders/flat.fs");
        m_LineShader = std::make_shared<Shader>("shaders/line.vs", "shaders/line.fs"); // Assuming a simple vertex color shader

        // 1. Set up static quad VAO/VBO/EBO for boxes
        float vertices[] = {
            -0.5f, -0.5f,  // 0: Bottom-left
             0.5f, -0.5f,  // 1: Bottom-right
             0.5f,  0.5f,  // 2: Top-right
            -0.5f,  0.5f   // 3: Top-left
        };
        unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        glGenVertexArrays(1, &m_QuadVAO);
        glGenBuffers(1, &m_QuadVBO);
        glGenBuffers(1, &m_QuadEBO);

        glBindVertexArray(m_QuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        
        // 2. Set up dynamic Line VAO/VBO
        glGenVertexArrays(1, &m_LineVAO);
        glGenBuffers(1, &m_LineVBO);
        glBindVertexArray(m_LineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
        glBufferData(GL_ARRAY_BUFFER, MAX_LINE_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0); // Position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
        glEnableVertexAttribArray(1); // Color
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));
    }

    void Renderer2D::Shutdown() {
        if (m_QuadVAO) glDeleteVertexArrays(1, &m_QuadVAO);
        if (m_QuadVBO) glDeleteBuffers(1, &m_QuadVBO);
        if (m_QuadEBO) glDeleteBuffers(1, &m_QuadEBO);

        if (m_LineVAO) glDeleteVertexArrays(1, &m_LineVAO);
        if (m_LineVBO) glDeleteBuffers(1, &m_LineVBO);
    }

    void Renderer2D::BeginScene(const glm::mat4& viewProjMatrix) {
        m_ViewProjCache = viewProjMatrix;
        m_FlatShader->Bind();
        m_FlatShader->SetMat4("u_ViewProjection", viewProjMatrix);
        m_LineVertexBuffer.clear();
    }

    void Renderer2D::DrawBox(const glm::vec2& position, const glm::vec2& size, float angle, const glm::vec4& color) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));

        m_FlatShader->SetMat4("u_Model", model);
        m_FlatShader->SetVec4("u_Color", color);

        glBindVertexArray(m_QuadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void Renderer2D::DrawLine(const glm::vec2& p0, const glm::vec2& p1, const glm::vec4& color) {
        m_LineVertexBuffer.push_back({ p0, color });
        m_LineVertexBuffer.push_back({ p1, color });
    }

    void Renderer2D::EndScene() {
        if (!m_LineVertexBuffer.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_LineVertexBuffer.size() * sizeof(Vertex), m_LineVertexBuffer.data());
            
            m_LineShader->Bind();
            m_LineShader->SetMat4("u_ViewProjection", m_ViewProjCache);
            
            glBindVertexArray(m_LineVAO);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_LineVertexBuffer.size()));
        }
    }
}
```

### 11. GLSL Shaders

**shaders/flat.vs**:
```glsl
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main() {
    gl_Position = u_ViewProjection * u_Model * vec4(aPos, 0.0, 1.0);
}
```

**shaders/flat.fs**:
```glsl
#version 330 core
out vec4 FragColor;
uniform vec4 u_Color;

void main() {
    FragColor = u_Color;
}
```
