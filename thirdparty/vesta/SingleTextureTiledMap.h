/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_SINGLE_TEXTURE_TILED_MAP_H_
#define _VESTA_SINGLE_TEXTURE_TILED_MAP_H_

#include "TiledMap.h"


namespace vesta
{

class SingleTextureTiledMap : public TiledMap
{
public:
    SingleTextureTiledMap(TextureMap* texture) :
        m_texture(texture)
    {
    }

    virtual ~SingleTextureTiledMap()
    {
    }

    TextureMap* texture() const
    {
        return m_texture.ptr();
    }

    void setTexture(TextureMap* texture)
    {
        m_texture = texture;
    }

    /** Get the tile at the specified level, column, and row.
      */
    virtual TextureSubrect tile(unsigned int level, unsigned int x, unsigned int y)
    {
        float dy = 1.0f / float(1 << level);
        float dx = dy * 0.5f;

        TextureSubrect r;
        r.texture = m_texture.ptr();
        r.u0 = float(x) * dx;
        r.v0 = float(y) * dy;
        r.u1 = r.u0 + dx;
        r.v1 = r.v0 + dy;

        return r;
    }

private:
    counted_ptr<TextureMap> m_texture;
};

}

#endif // _VESTA_SINGLE_TEXTURE_TILED_MAP_H_
