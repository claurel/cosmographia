/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_FRAMEBUFFER_H_
#define _VESTA_FRAMEBUFFER_H_

#include "TextureMap.h"
#include "OGLHeaders.h"


namespace vesta
{

class GLFramebuffer;
class CubeMapFramebuffer;

/** Framebuffer is a C++ wrapper for OpenGL framebuffer objects.
 */
class Framebuffer : public Object
{
friend class CubeMapFramebuffer;

public:
    enum
    {
        ColorAttachment = 0x1,
        DepthAttachment = 0x2,
    };

private:
    Framebuffer(unsigned int width, unsigned int height, unsigned int attachments, TextureMap::ImageFormat colorFormat);

public:
    ~Framebuffer();

    GLuint fboHandle() const;
    GLuint depthTexHandle() const;
    GLuint colorTexHandle() const;

    unsigned int width() const;
    unsigned int height() const;

    bool isValid() const;

    bool hasColor() const;
    bool hasDepthTarget() const;

    TextureMap* colorTexture() const;
    TextureMap* depthTexture() const;

    GLFramebuffer* glFramebuffer() const
    {
        return m_fb.ptr();
    }

    void bind() const;
    static void unbind();

    bool resize(unsigned int width, unsigned int height);

    static bool supported();

    static Framebuffer* CreateFramebuffer(unsigned int width,
                                          unsigned int height,
                                          TextureMap::ImageFormat format,
                                          TextureMap::ImageFormat depthFormat = TextureMap::Depth24);
    static Framebuffer* CreateDepthOnlyFramebuffer(unsigned int width,
                                                   unsigned int height,
                                                   TextureMap::ImageFormat depthFormat = TextureMap::Depth24);
    static Framebuffer* CreateColorOnlyFramebuffer(unsigned int width,
                                                   unsigned int height,
                                                   TextureMap::ImageFormat format);

private:
    counted_ptr<GLFramebuffer> m_fb;
    counted_ptr<TextureMap> m_colorTexture;
    counted_ptr<TextureMap> m_depthTexture;
    TextureMap::ImageFormat m_format;
    unsigned int m_attachments;
};

}

#endif // _VESTA_FRAMEBUFFER_H_
