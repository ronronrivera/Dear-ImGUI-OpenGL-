#include "core/app.hpp"
#include "imgui.h"
#include <memory>

namespace Physix {
    App::App(): m_Running(true){
        m_Window = std::make_unique<Window>(800, 600, "Physix Engine");

        m_Framebuffer = std::make_unique<Framebuffer>(800, 600);
        m_ImGuiLayer = std::make_unique<ImGuiLayer>(m_Window->GetGLFWwindow());
    }

    App::~App(){}
    
    void App::Run(){
        while(m_Running && !m_Window->shouldClose()){
            m_Window->PollEvents();

            // Clear the default main window backbuffer to prevent trailing/glitch lines when resizing panels
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Clear the physics viewport framebuffer
            m_Framebuffer->Bind();
            glViewport(0, 0, m_Framebuffer->GetWidth(), m_Framebuffer->GetHeight());
            glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //TODO:: Update physics, render frame, draw UI 
             
            m_Framebuffer->Unbind();
            
            m_ImGuiLayer->Begin();
            

            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::Begin("Presets and Problems");

            if (ImGui::Button("Spawn Projectile Scenario")) {

            }
            ImGui::End();

            // Step D: Right Panel - Inspector
            ImGui::Begin("Inspector");
            ImGui::End();

            // Step E: Viewport Panel (displays the FBO texture)
            m_ViewportPanel.Render(*m_Framebuffer);

            // Step F: Draw final UI
            m_ImGuiLayer->End();

            m_Window->swapBuffers();
        }
    }

}
