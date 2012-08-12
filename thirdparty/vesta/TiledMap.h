/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_TILED_MAP_H_
#define _VESTA_TILED_MAP_H_

#include "Object.h"
#include "TextureMap.h"


namespace vesta
{

class TiledMap : public Object
{
public:
    /** Structure returned by the tile() method of TiledMap. (u0, v0) and
      * (u1, v1) are texture coordinates that give some subrectangle of a
      * texture.
      */
    struct TextureSubrect
    {
        TextureMap* texture;
        float u0;
        float v0;
        float u1;
        float v1;
    };

    TiledMap() :
        m_textureUsage(TextureProperties::ColorTexture)
    {
    }

    virtual ~TiledMap()
    {
    }

    /** Get the tile at the specified level, column, and row.
      *
      * \param level zero-based level index
      * \param x column index; level n has 2^(n+1) columns
      * \param y row index; level n has 2^n rows
      */
    virtual TextureSubrect tile(unsigned int level, unsigned int x, unsigned int y) = 0;

    /** Get the size in pixels of one side of a tile. Maps map
      * contain texture tiles of different resolutions, but determining
      * which tiles to load is based on assuming that all tiles are
      * tileSize x tileSize.
      *
      * A tileSize of zero indicates that no assumptions about tessellating
      * globe geometry should be made based on texture tile resolution. This is
      * appropriate when the tiled map isn't a pyramid with higher resolution
      * at lower levels (e.g. SingleTextureTiledMap)
      */
    virtual unsigned int tileSize() const
    {
        return 0;
    }

    TextureProperties::TextureUsage textureUsage() const
    {
        return m_textureUsage;
    }

    void setTextureUsage(TextureProperties::TextureUsage usage)
    {
        m_textureUsage = usage;
    }

private:
    TextureProperties::TextureUsage m_textureUsage;
};

}

#endif // _VESTA_TILED_MAP_H_
