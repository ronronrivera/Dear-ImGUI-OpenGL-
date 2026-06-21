#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Physix{
    class Window {
        private:
            GLFWwindow *m_GLFWwindow;
            int m_Width;
            int m_Height;
            float m_Scale;
        public:
            Window(int width, int height, const char *title);
            ~Window();
            
            bool shouldClose() const;
            void swapBuffers();
            void PollEvents();

            GLFWwindow *GetGLFWwindow() const {return m_GLFWwindow;}
            int GetWidth() const {return m_Width;}
            int GetHeight() const {return m_Height;}
            int GetScale() const {return m_Scale;}
    };
}


