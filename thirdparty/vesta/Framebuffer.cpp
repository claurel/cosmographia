/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Framebuffer.h"
#include "Debug.h"
#include "glhelp/GLFramebuffer.h"

using namespace vesta;


// Private constructor; framebuffers may only be created through
// factory methods.
Framebuffer::Framebuffer(unsigned int width,
                         unsigned int height,
                         unsigned int attachments,
                         TextureMap::ImageFormat format) :
    m_format(format),
    m_attachments(attachments)
{
    m_fb = new GLFramebuffer(width, height);
}


Framebuffer::~Framebuffer()
{
}


/** Get the width in pixels of the framebuffer.
  */
unsigned int
Framebuffer::width() const
{
    return m_fb->width();
}


/** Get the height in pixels of the framebuffer.
  */
unsigned int
Framebuffer::height() const
{
    return m_fb->height();
}


/** Check whether this framebuffer is ready to be used for rendering.
  */
bool
Framebuffer::isValid() const
{
    return m_fb->isValid();
}


/** Make this frame buffer the active render target.
  */
void
Framebuffer::bind() const
{
    return m_fb->bind();
}


/** Revert to using the default framebuffer for rendering.
  */
void
Framebuffer::unbind()
{
    GLFramebuffer::unbind();
}


/** Change the size of this framebuffer.
  *
  * NOT YET IMPLEMENTED.
  *
  * \return true if the resize was successful, false if not (probably an out of memory situation)
  */
bool
Framebuffer::resize(unsigned int /* width */, unsigned int /* height */)
{
    return false;
}


/** Get the color texture for this framebuffer. Returns NULL if the
  * framebuffer is depth-only.
  */
TextureMap*
Framebuffer::colorTexture() const
{
    return m_colorTexture.ptr();
}


/** Get the depth texture for this framebuffer. Returns NULL if the
  * framebuffer is color-only.
  */
TextureMap*
Framebuffer::depthTexture() const
{
    return m_depthTexture.ptr();
}


// Test formats for validity and hardware support
static bool CheckFormats(TextureMap::ImageFormat colorFormat, TextureMap::ImageFormat depthFormat)
{
    if (!TextureMap::IsDepthFormat(depthFormat))
    {
        VESTA_WARNING("Error creating framebuffer. %s is not a depth buffer format",
                      TextureMap::FormatName(depthFormat).c_str());
        return false;
    }

    if (!TextureMap::IsFormatSupported(depthFormat))
    {
        VESTA_WARNING("Error creating framebuffer. %s is not a format supported by the graphics hardware.",
                      TextureMap::FormatName(depthFormat).c_str());
        return false;
    }

    if (!TextureMap::IsFormatSupported(colorFormat))
    {
        VESTA_WARNING("Error creating framebuffer. %s is not a format supported by the graphics hardware.",
                      TextureMap::FormatName(colorFormat).c_str());
        return false;
    }

    return true;
}


/** Create a new framebuffer object with both a color buffer and depth buffer.
  * This factory method will return either a valid and fully constructed framebuffer
  * or null if there was a problem creating the framebuffer.
  */
Framebuffer*
Framebuffer::CreateFramebuffer(unsigned int width,
                               unsigned int height,
                               TextureMap::ImageFormat format,
                               TextureMap::ImageFormat depthFormat)
{
    if (!CheckFormats(format, depthFormat))
    {
        return NULL;
    }

    Framebuffer* fb = new Framebuffer(width, height,
                                      ColorAttachment | DepthAttachment,
                                      format);
    if (!fb)
    {
        return NULL;
    }

    TextureProperties texProps;
    texProps.addressS = TextureProperties::Clamp;
    texProps.addressT = TextureProperties::Clamp;
    texProps.useMipmaps = false;
    texProps.usage = TextureProperties::ColorTexture;

    fb->m_colorTexture = new TextureMap("color", NULL, texProps);
    if (fb->m_colorTexture->generate(width, height, format) == 0)
    {
        VESTA_WARNING("Error creating color texture for framebuffer.");
        delete fb;
        return NULL;
    }
    else
    {
        fb->m_fb->attachColorTarget2D(fb->m_colorTexture->id());
    }

    fb->m_depthTexture = TextureMap::CreateDepthTexture(width, height, depthFormat);
    if (fb->m_depthTexture.isNull())
    {
        VESTA_WARNING("Error creating depth texture for framebuffer.");
        delete fb;
        return NULL;
    }

    fb->m_fb->attachDepthTarget(fb->m_depthTexture->id());

    if (!fb->isValid())
    {
        delete fb;
        return NULL;
    }

    return fb;
}


/** Create a new framebuffer object with a color buffer but no depth buffer.
  * This factory method will return either a valid and fully constructed framebuffer
  * or null if there was a problem creating the framebuffer.
  */
Framebuffer*
Framebuffer::CreateColorOnlyFramebuffer(unsigned int width,
                                        unsigned int height,
                                        TextureMap::ImageFormat format)
{
    if (!TextureMap::IsFormatSupported(format))
    {
        VESTA_WARNING("Error creating framebuffer. %s is not a format supported by the graphics hardware.",
                      TextureMap::FormatName(format).c_str());
        return false;
    }

    Framebuffer* fb = new Framebuffer(width, height,
                                      ColorAttachment,
                                      format);
    if (!fb)
    {
        return NULL;
    }

    TextureProperties texProps;
    texProps.addressS = TextureProperties::Clamp;
    texProps.addressT = TextureProperties::Clamp;
    texProps.useMipmaps = false;
    texProps.usage = TextureProperties::ColorTexture;

    fb->m_colorTexture = new TextureMap("color", NULL, texProps);
    if (fb->m_colorTexture->generate(width, height, format) == 0)
    {
        VESTA_WARNING("Error creating color texture.");
        delete fb;
        return NULL;
    }
    else
    {
        fb->m_fb->attachColorTarget2D(fb->m_colorTexture->id());
    }

    if (!fb->isValid())
    {
        delete fb;
        return NULL;
    }

    return fb;
}


/** Create a new framebuffer object with just a depth buffer. Such a framebuffer
  * is appropriate for drawing shadow maps.
  *
  * This factory method will return either a valid and fully constructed framebuffer
  * or null if there was a problem creating the framebuffer.
  */
Framebuffer*
Framebuffer::CreateDepthOnlyFramebuffer(unsigned int width,
                                        unsigned int height,
                                        TextureMap::ImageFormat depthFormat)
{
    if (!CheckFormats(TextureMap::R8G8B8A8, depthFormat))
    {
        return NULL;
    }

    Framebuffer* fb = new Framebuffer(width, height,
                                      DepthAttachment,
                                      TextureMap::R8G8B8A8);
    if (!fb)
    {
        return NULL;
    }

    fb->m_depthTexture = TextureMap::CreateDepthTexture(width, height, TextureMap::Depth24);
    if (fb->m_depthTexture.isNull())
    {
        VESTA_WARNING("Error creating framebuffer for shadow map.");
        delete fb;
        return NULL;
    }

    fb->m_fb->attachDepthTarget(fb->m_depthTexture->id());

    if (!fb->isValid())
    {
        delete fb;
        return NULL;
    }

    return fb;
}


/** Return true if the graphics hardware and driver supports rendering to offscreen
  * frame buffers.
  */
bool
Framebuffer::supported()
{
    return GLFramebuffer::supported();
}
