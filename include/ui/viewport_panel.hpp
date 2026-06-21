#pragma  once

#include "renderer/framebuffer.hpp"
#include <imgui.h>

namespace Physix {
    class ViewportPanel {
        private:
            ImVec2 m_PanelSize = {0.0f, 0.0f};
            bool m_IsFocused = false;
            bool m_IsHovered = false;

        public:
            ViewportPanel();
            ~ViewportPanel();

            void Render(Framebuffer &framebuffer);

            ImVec2 GetSize() const {return m_PanelSize;}
            bool isFocused() const {return m_IsFocused;}
            bool isHovered() const {return m_IsHovered;}
    };
}
