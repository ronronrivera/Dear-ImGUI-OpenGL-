#include "core/app.hpp"
#include <memory>

namespace Physix {
    App::App(): m_Running(true){
        m_Window = std::make_unique<Window>(800, 600, "Physix Engine");
    }

    App::~App(){}
    
    void App::Run(){
        while(m_Running && !m_Window->shouldClose()){
            m_Window->PollEvents();

            //clear screen
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //TODO:: Update physics, render frame, draw UI 

            m_Window->swapBuffers();
        }
    }

}
