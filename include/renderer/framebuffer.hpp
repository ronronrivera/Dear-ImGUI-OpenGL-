#pragma once

namespace Physix {
    class Framebuffer {
        private:
            unsigned int m_FboID;
            unsigned int m_TextureID;
            unsigned int m_RboID;

            int m_Width, m_Height;

            void Create(int width, int height);
            void Destroy();

        public:
            Framebuffer(int width, int height);
            ~Framebuffer();

            void Resize(int width, int height);

            void Bind() const;
            void Unbind() const;

            unsigned int GetFboID() const {return m_FboID;}
            unsigned int GetTextureID() const {return m_TextureID;}
            unsigned int GetRboID() const {return m_RboID;}

            int GetWidth() const {return m_Width;}
            int GetHeight() const {return m_Height;}
    };
}
