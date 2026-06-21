#pragma once
#include "core/window.hpp"

#include "renderer/framebuffer.hpp"
#include "ui/imgui_layer.hpp"
#include "ui/viewport_panel.hpp"

#include <memory>


namespace Physix {
    class App {
        private:
            std::unique_ptr<Window> m_Window;
            bool m_Running;
            
            std::unique_ptr<Framebuffer> m_Framebuffer; 
            std::unique_ptr<ImGuiLayer>  m_ImGuiLayer;
            ViewportPanel m_ViewportPanel;

        public:
            App();
            ~App();

            void Run();
    };
}
