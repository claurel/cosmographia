// GLFramebuffer.cpp
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// VESTA is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// VESTA is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// VESTA. If not, see <http://www.gnu.org/licenses/>.

#include "GLFramebuffer.h"
#include "../Debug.h"

using namespace vesta;

#ifdef VESTA_OGLES2
#define glGenFramebuffersEXT glGenFramebuffers
#define glBindFramebufferEXT glBindFramebuffer
#define glDeleteFramebuffersEXT glDeleteFramebuffers
#define glFramebufferTexture2DEXT glFramebufferTexture2D
#define glCheckFramebufferStatusEXT glCheckFramebufferStatus
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0
#define GL_DEPTH_ATTACHMENT_EXT GL_DEPTH_ATTACHMENT
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT GL_FRAMEBUFFER_UNSUPPORTED
#define GL_FRAMEBUFFER_COMPLETE_EXT GL_FRAMEBUFFER_COMPLETE
#endif


#ifdef VESTA_OGLES2
static void glDrawBuffer(GLenum /* mode */ ) {}
#endif


GLFramebuffer::GLFramebuffer(unsigned int width, unsigned int height) :
    m_fboHandle(0),
    m_width(width),
    m_height(height),
    m_valid(false)
{
    if (supported())
    {
        GLint maxTexSize = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        if (int(width) > maxTexSize || int(height) > maxTexSize)
        {
            VESTA_WARNING("Requested framebuffer size of %dx%d exceeds maximum OpenGL texture dimension of %d", width, height, maxTexSize);
        }
        else
        {
            glGenFramebuffersEXT(1, &m_fboHandle);
            if (m_fboHandle != 0)
            {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboHandle);
#ifndef VESTA_OGLES2
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);
#endif
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            }
        }
    }
    else
    {
        VESTA_WARNING("Attempted to create GLFramebuffer object, but framebuffer_object extension isn't supported.");
    }
}


GLFramebuffer::~GLFramebuffer()
{
    if (m_fboHandle != 0)
    {
        glDeleteFramebuffersEXT(1, &m_fboHandle);
    }
}


GLenum
GLFramebuffer::attachTarget(GLenum attachment, GLenum target, GLuint texId)
{
    GLenum status = GL_FRAMEBUFFER_UNSUPPORTED_EXT;

    if (m_fboHandle != 0)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboHandle);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, target, texId, 0);
        if (attachment == GL_COLOR_ATTACHMENT0_EXT)
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        m_valid = true;
    }
    else
    {
        VESTA_WARNING("Attempted to attach texture to invalid FBO.");
    }

    return status;
}


GLenum
GLFramebuffer::detachTarget(GLenum attachment, GLenum target)
{
    GLenum status = GL_FRAMEBUFFER_UNSUPPORTED_EXT;

    if (m_fboHandle != 0)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboHandle);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, target, 0, 0);
        if (attachment == GL_COLOR_ATTACHMENT0_EXT)
        {
            glDrawBuffer(GL_NONE);
        }
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
    else
    {
        VESTA_WARNING("Attempted to detach texture from invalid FBO.");
    }

    return status;
}



void
GLFramebuffer::detachColorTarget()
{
    detachTarget(GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D);
}


bool
GLFramebuffer::attachColorTarget2D(GLuint texId)
{
    GLenum status = attachTarget(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        VESTA_WARNING("OpenGL error occurred while attaching color texture to FBO (status 0x%08x)", status);
        return false;
    }
    else
    {
        m_colorTexHandle = texId;
        return true;
    }
}


bool
GLFramebuffer::attachColorTargetCubeFace(GLenum target, GLuint texId)
{
    GLenum status = attachTarget(GL_COLOR_ATTACHMENT0, target, texId);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        VESTA_WARNING("OpenGL error occurred while attaching cube map face to FBO (status 0x%08x)", status);
        return false;
    }
    else
    {
        m_colorTexHandle = texId;
        return true;
    }
}


void
GLFramebuffer::detachDepthTarget()
{
    if (m_fboHandle != 0)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboHandle);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
}


/** Create a new depth texture and attach it.
  *
  * \return true if the framebuffer object is complete and valid
  */
bool
GLFramebuffer::attachDepthTarget()
{
    m_depthTexHandle = createDepthTexture();
    if (m_depthTexHandle == 0)
    {
        return false;
    }

    return attachDepthTarget(m_depthTexHandle);
}


/** Attach an existing depth texture to the framebuffer.
  *
  * \return true if the framebuffer object is complete and valid
  */
bool
GLFramebuffer::attachDepthTarget(GLuint depthTexHandle)
{
    GLenum status = attachTarget(GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depthTexHandle);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        VESTA_WARNING("OpenGL error occurred while attaching depth texture to FBO (status 0x%08x)", status);
        return false;
    }
    else
    {
        m_depthTexHandle = depthTexHandle;
        m_valid = true;
        return true;
    }
}


GLuint
GLFramebuffer::createDepthTexture()
{
    GLuint depthTexId = 0;

#ifdef VESTA_OGLES2
    glGenRenderbuffers(1, &depthTexId);
    if (depthTexId == 0)
    {
        VESTA_WARNING("Failed to create depth render buffer handle.");
        return 0;
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, depthTexId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, m_width, m_height);
#else
    glGenTextures(1, &depthTexId);
    if (depthTexId == 0)
    {
        VESTA_WARNING("Failed to create depth texture handle.");
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, depthTexId);

    // GL_NEAREST is usually the appropriate filtering for depth textures. However,
    // NVIDIA GPUs (and possibly others) perform 'free' 4x percentage close filtering
    // when the filter is set to GL_LINEAR.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Settings for shadow mapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Allocate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

    // Unbind it
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR)
    {
#ifdef VESTA_OGLES2
        const char* errorMessage = "Framebuffer error";
#else
        const GLubyte* errorMessage = gluErrorString(errorCode);
#endif
        if (errorMessage)
        {
            VESTA_WARNING("OpenGL error occurred when creating depth texture: %s", errorMessage);
            glDeleteTextures(1, &depthTexId);
            return 0;
        }
    }

    return depthTexId;
}


void
GLFramebuffer::bind() const
{
    if (m_valid)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboHandle);
    }
}


void
GLFramebuffer::unbind()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}


/** Return true if framebuffer objects are supported by the current
  * OpenGL context.
  */
bool
GLFramebuffer::supported()
{
#ifdef VESTA_OGLES2
    return true;
#else
    return GLEW_EXT_framebuffer_object;
#endif
}
