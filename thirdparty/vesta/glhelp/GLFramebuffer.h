// GLFramebuffer.h
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

#ifndef _VESTA_GL_FRAMEBUFFER_H_
#define _VESTA_GL_FRAMEBUFFER_H_

#include "../OGLHeaders.h"
#include "../Object.h"


namespace vesta
{

/** GLFramebuffer is a C++ wrapper for OpenGL framebuffer objects.
 */
class GLFramebuffer : public Object
{
public:
    GLFramebuffer(unsigned int width, unsigned int height);
    ~GLFramebuffer();

    GLuint fboHandle() const
    {
        return m_fboHandle;
    }

    GLuint depthTexHandle() const
    {
        return m_depthTexHandle;
    }

    GLuint colorTexHandle() const
    {
        return m_colorTexHandle;
    }

    unsigned int width() const
    {
        return m_width;
    }

    unsigned int height() const
    {
        return m_height;
    }

    GLenum attachTarget(GLenum attachment, GLenum target, GLuint texId);
    GLenum detachTarget(GLenum attachment, GLenum target);

    void detachDepthTarget();
    bool attachDepthTarget();
    bool attachDepthTarget(GLuint texId);
    void detachColorTarget();
    bool attachColorTarget2D(GLuint texId);
    bool attachColorTargetCubeFace(GLenum target, GLuint texId);

    bool isValid() const
    {
        return m_valid;
    }

    bool hasColorTarget() const
    {
        return m_colorTexHandle != 0;
    }

    bool hasDepthTarget() const
    {
        return m_depthTexHandle != 0;
    }

    void bind() const;
    static void unbind();

    static bool supported();

private:
    GLuint createDepthTexture();

private:
    unsigned int m_attachments;
    GLuint m_fboHandle;
    GLuint m_colorTexHandle;
    GLuint m_depthTexHandle;
    unsigned int m_width;
    unsigned int m_height;
    bool m_valid;
};

}

#endif // _VESTA_GL_FRAMEBUFFER_H_
