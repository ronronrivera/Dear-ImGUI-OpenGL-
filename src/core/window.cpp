#include "core/window.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>

namespace Physix {
    Window::Window(int width, int height, const char* title): m_Width(width), m_Height(height), m_Scale(1.0f) {
        

        if(!glfwInit()){
            std::cerr << "Failed to initlize GLFW\n";
            exit(-1);
        }

        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        
        m_GLFWwindow = glfwCreateWindow(m_Width, m_Height, title, nullptr, nullptr);
        if(!m_GLFWwindow){
            std::cerr << "Failed to create window\n";
            glfwTerminate();
            exit(-1);
        }

        glfwMakeContextCurrent(m_GLFWwindow);

        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
            std::cerr << "Failed to initialize GLAD\n";
            exit(-1);
        }
        
        //configure viewport
        glViewport(0, 0, m_Width, m_Height);
        glfwSetWindowUserPointer(m_GLFWwindow, this);

        //Handle Resizing
        glfwSetFramebufferSizeCallback(m_GLFWwindow, [](GLFWwindow *w, int width, int height){
            auto *window = static_cast<Window*>(glfwGetWindowUserPointer(w));
            window->m_Width = width;
            window->m_Height = height;
            glViewport(0, 0, width, height);
        });

        GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
        if(primaryMonitor){
            float xScale, yScale;
            glfwGetMonitorContentScale(primaryMonitor, &xScale, &yScale);
            m_Scale = xScale;
        }
    }

    Window::~Window(){
        glfwDestroyWindow(m_GLFWwindow);
        glfwTerminate();
    }
    
    bool Window::shouldClose() const {
        return glfwWindowShouldClose(m_GLFWwindow);
    }
    
    void Window::swapBuffers(){
        glfwSwapBuffers(m_GLFWwindow);
    }

    void Window::PollEvents(){
        glfwPollEvents();
    }
}


