#pragma once
#include "core/window.hpp"
#include <memory>

namespace Physix {
    class App {
        private:
            std::unique_ptr<Window> m_Window;
            bool m_Running;

        public:
            App();
            ~App();

            void Run();
    };
}
