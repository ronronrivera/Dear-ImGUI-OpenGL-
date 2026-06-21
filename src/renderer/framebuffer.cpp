#include "renderer/framebuffer.hpp"
#include <glad/glad.h>
#include <iostream>

namespace Physix {
    Framebuffer::Framebuffer(int width, int height){
        Create(width, height);
    }
    Framebuffer::~Framebuffer(){
        Destroy();
    }

    void Framebuffer::Create(int width, int height){
        m_Width = width;
        m_Height = height;
    

        //gen framebuffer
        glGenFramebuffers(1, &m_FboID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
        
        //create color attachment texture
        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0 ,GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

        //create depth/stencil render buffer (for 3d camera depth testing);
        glGenRenderbuffers(1, &m_RboID);
        glBindRenderbuffer(GL_RENDERBUFFER, m_RboID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RboID);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
            std::cerr << "Framebuffer is not complete\n";
        }

        // Clear the newly created framebuffer to avoid displaying garbage data
        GLint restoreViewport[4];
        glGetIntegerv(GL_VIEWPORT, restoreViewport);

        glViewport(0, 0, m_Width, m_Height);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(restoreViewport[0], restoreViewport[1], restoreViewport[2], restoreViewport[3]);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::Destroy(){
        if(m_FboID) glDeleteFramebuffers(1, &m_FboID);
        if(m_TextureID) glDeleteTextures(1, &m_TextureID);
        if(m_RboID) glDeleteRenderbuffers(1, &m_RboID);
    
        m_RboID = 0;
        m_TextureID = 0;
        m_FboID = 0;
    }
    
    
    void Framebuffer::Resize(int width, int height){
        Destroy();
        Create(width, height);
    }


    void Framebuffer::Bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
    }

    void Framebuffer::Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


}
