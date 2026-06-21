#include "ui/viewport_panel.hpp"
#include <cstdint>
#include <imgui.h>

namespace Physix {
    ViewportPanel::ViewportPanel() {}
    ViewportPanel::~ViewportPanel() {}

    void ViewportPanel::Render(Framebuffer &framebuffer){
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Physics Viewport");

        ImVec2 currentViewportSize = ImGui::GetContentRegionAvail();
        m_PanelSize = currentViewportSize;

        if(currentViewportSize.x > 1 && currentViewportSize.y > 1){
            int newW = (int)currentViewportSize.x;
            int newH = (int)currentViewportSize.y;

            // Only resize when the integer pixel size actually changes
            if(newW != framebuffer.GetWidth() || newH != framebuffer.GetHeight()){
                framebuffer.Resize(newW, newH);
            }

            ImGui::Image((void*)(intptr_t)framebuffer.GetTextureID(),
                    currentViewportSize, ImVec2(0, 1), ImVec2(1, 0));
        }

        m_IsFocused = ImGui::IsWindowFocused();
        m_IsHovered = ImGui::IsWindowHovered();
        ImGui::End();
        ImGui::PopStyleVar();
    }
}
