/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "CubeMapFramebuffer.h"
#include "Debug.h"
#include "glhelp/GLFramebuffer.h"

using namespace vesta;


// Private constructor; framebuffers may only be created through
// factory methods.
CubeMapFramebuffer::CubeMapFramebuffer(unsigned int /* size */,
                                       TextureMap::ImageFormat format) :
    m_format(format)
{
}


CubeMapFramebuffer::~CubeMapFramebuffer()
{
}


/** Get the size in texels of a side of the cube map.
  */
unsigned int
CubeMapFramebuffer::size() const
{
    return m_faces[0]->width();
}


/** Create a cube map framebuffer with a shared depth buffer for the six
  * color faces.
  */
CubeMapFramebuffer*
CubeMapFramebuffer::CreateCubicReflectionMap(unsigned int size, TextureMap::ImageFormat format)
{
    counted_ptr<TextureMap> depthTex(TextureMap::CreateDepthTexture(size, size, TextureMap::Depth24));
    if (!depthTex.isValid())
    {
        return NULL;
    }

    counted_ptr<TextureMap> cubeMap(TextureMap::CreateCubeMap(size, format));
    if (!cubeMap.isValid())
    {
        return NULL;
    }

    CubeMapFramebuffer* cubeMapFb(new CubeMapFramebuffer(size, format));

    // Allocate the framebuffer objects
    for (int i = 0; i < 6; ++i)
    {
        Framebuffer* fb = new Framebuffer(size, size,
                                          Framebuffer::ColorAttachment | Framebuffer::DepthAttachment,
                                          format);
        if (!fb)
        {
            return NULL;
        }

        fb->m_fb->attachTarget(GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMap->id());
        GLenum status = fb->m_fb->attachTarget(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex->id());
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            delete fb;
            delete cubeMapFb;
            return NULL;
        }

        cubeMapFb->m_faces[i] = fb;
    }

    cubeMapFb->m_colorTexture = cubeMap.ptr();
    cubeMapFb->m_depthTexture = depthTex.ptr();

    return cubeMapFb;
}


/** Create a cube map framebuffer with six depth-only faces for use as an
  * omnidirectional shadow map.
  */
CubeMapFramebuffer*
CubeMapFramebuffer::CreateCubicShadowMap(unsigned int size)
{
    counted_ptr<TextureMap> cubeMap(TextureMap::CreateCubeMap(size, TextureMap::R32F));
    if (!cubeMap.isValid())
    {
        return NULL;
    }

    CubeMapFramebuffer* cubeMapFb(new CubeMapFramebuffer(size, TextureMap::R32F));

    // Allocate the depth buffer objects
    for (int i = 0; i < 6; ++i)
    {
        Framebuffer* fb = new Framebuffer(size, size,
                                          Framebuffer::ColorAttachment,
                                          TextureMap::R32F);
        if (!fb)
        {
            return NULL;
        }

        GLenum status = fb->m_fb->attachTarget(GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMap->id());
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            delete fb;
            delete cubeMapFb;
            return NULL;
        }

        cubeMapFb->m_faces[i] = fb;
    }

    cubeMapFb->m_colorTexture = cubeMap.ptr();
    cubeMapFb->m_depthTexture = NULL;

    return cubeMapFb;

#if 0
    counted_ptr<TextureMap> cubeMap(TextureMap::CreateCubeMap(size, TextureMap::Depth24));
    if (!cubeMap.isValid())
    {
        return NULL;
    }

    CubeMapFramebuffer* cubeMapFb(new CubeMapFramebuffer(size, TextureMap::R8G8B8A8));

    // Allocate the depth buffer objects
    for (int i = 0; i < 6; ++i)
    {
        Framebuffer* fb = new Framebuffer(size, size,
                                          Framebuffer::DepthAttachment,
                                          TextureMap::R8G8B8A8);
        if (!fb)
        {
            return NULL;
        }

        GLenum status = fb->m_fb->attachTarget(GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, cubeMap->id());
        if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
        {
            delete fb;
            delete cubeMapFb;
            return NULL;
        }

        cubeMapFb->m_faces[i] = fb;
    }

    cubeMapFb->m_colorTexture = NULL;
    cubeMapFb->m_depthTexture = cubeMap.ptr();

    return cubeMapFb;
#endif
}


/** Returns true if the graphics hardware and drivers supports rendering
  * to the faces of a cube map texture.
  */
bool
CubeMapFramebuffer::supported()
{
#if RENDERER == OGLES2
    return true;
#else
    return GLFramebuffer::supported() && GLEW_ARB_texture_cube_map;
#endif
}
