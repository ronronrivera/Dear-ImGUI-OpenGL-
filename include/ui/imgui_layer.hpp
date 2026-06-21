#pragma once

#include <GLFW/glfw3.h>

namespace Physix {
    class ImGuiLayer {
        public:
            ImGuiLayer(GLFWwindow *window);
            ~ImGuiLayer();

            void Begin();
            void End();
    };
}
