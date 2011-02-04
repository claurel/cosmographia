/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_CUBE_MAP_FRAMEBUFFER_H_
#define _VESTA_CUBE_MAP_FRAMEBUFFER_H_

#include "Framebuffer.h"


namespace vesta
{

/** CubeMapFramebuffer is a bundle of six Framebuffer objects, one for each
 *  side of a cube map. CubeMapFramebuffers are used for environment maps and
 *  omnidirectional shadow buffers.
 */
class CubeMapFramebuffer : public Object
{
public:
    enum Face
    {
        PositiveX = 0,
        NegativeX = 1,
        PositiveY = 2,
        NegativeY = 3,
        PositiveZ = 4,
        NegativeZ = 5,
    };

private:
    CubeMapFramebuffer(unsigned int size, TextureMap::ImageFormat colorFormat);

public:
    ~CubeMapFramebuffer();

    Framebuffer* face(Face faceIndex) const
    {
        int i = (int) faceIndex;
        if (i < 6)
        {
            return m_faces[i].ptr();
        }
        else
        {
            return 0;
        }
    }

    unsigned int size() const;

    bool isValid() const;

    TextureMap* colorTexture() const
    {
        return m_colorTexture.ptr();
    }

    TextureMap* depthTexture() const
    {
        return m_depthTexture.ptr();
    }

    static bool supported();

    static CubeMapFramebuffer* CreateCubicShadowMap(unsigned int size);
    static CubeMapFramebuffer* CreateCubicReflectionMap(unsigned int size,
                                                        TextureMap::ImageFormat format);

private:
    counted_ptr<Framebuffer> m_faces[6];
    counted_ptr<TextureMap> m_colorTexture;
    counted_ptr<TextureMap> m_depthTexture;
    TextureMap::ImageFormat m_format;
    bool m_valid;
};

}

#endif // _VESTA_CUBE_MAP_FRAMEBUFFER_H_
