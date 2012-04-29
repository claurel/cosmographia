/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "HierarchicalTiledMap.h"
#include "TextureMapLoader.h"

using namespace vesta;
using namespace std;


/** Construct a HierarchicalTiledMap.
  */
HierarchicalTiledMap::HierarchicalTiledMap(TextureMapLoader* loader, unsigned int tileSize) :
    m_loader(loader),
    m_tileSize(tileSize),
    m_tileBorderFraction(0.0f)
{
}


HierarchicalTiledMap::~HierarchicalTiledMap()
{
}


static inline v_uint64
computeTileId(unsigned int level, unsigned int x, unsigned int y)
{
    return (v_uint64(level) << 48) | v_uint64(x) << 24 | v_uint64(y);
}


/** Get the tile at the specified level, column, and row.
  *
  * \param level zero-based level index
  * \param x column index; level n has 2^(n+1) columns
  * \param y row index; level n has 2^n rows
  */
TiledMap::TextureSubrect
HierarchicalTiledMap::tile(unsigned int level, unsigned int x, unsigned int y)
{
    TextureSubrect r;
    r.texture = NULL;
    r.u0 = m_tileBorderFraction;
    r.v0 = m_tileBorderFraction;
    r.u1 = 1.0f - m_tileBorderFraction;
    r.v1 = 1.0f - m_tileBorderFraction;

    int testLevel = int(level);
    unsigned int testX = x;
    unsigned int testY = y;
    while (testLevel >= 0 && r.texture == NULL)
    {
        v_uint64 tileId = computeTileId((unsigned int) testLevel, testX, testY);

        TextureMap* tileTexture = NULL;

        TileCache::iterator iter = m_tiles.find(tileId);
        if (iter != m_tiles.end())
        {
            // An entry for the tile exists in the hash
            tileTexture = iter->second.ptr();
        }
        else
        {
            if (isValidTileAddress((unsigned int) testLevel, testX, testY))
            {
                // Tile not present, try and load it
                string resourceId = tileResourceIdentifier((unsigned int) testLevel, testX, testY);
                if (tileResourceExists(resourceId))
                {
                    TextureProperties props(TextureProperties::Clamp);
                    props.maxAnisotropy = 16;

                    tileTexture = m_loader->loadTexture(resourceId, props);
                    m_tiles[tileId] = counted_ptr<TextureMap>(tileTexture);
                }
                else
                {
                    // Insert a null in the table so that we don't attempt to load the
                    // tile again.
                    m_tiles[tileId] = counted_ptr<TextureMap>();
                }
            }
        }

        if (tileTexture && tileTexture->makeResident())
        {
            // The tile is loaded and ready to use
            r.texture = tileTexture;
        }
        else
        {
            testX /= 2;
            testY /= 2;
            testLevel--;

            // The requested tile doesn't exist or hasn't been loaded yet. Try using a subrectangle
            // of a lower resolution level.
            float uExtent = (r.u1 - r.u0) * 0.5f;
            float vExtent = (r.v1 - r.v0) * 0.5f;

            unsigned int mask = (1 << (level - (unsigned int) testLevel)) - 1;
            r.u0 = m_tileBorderFraction + uExtent * (x & mask);
            r.v0 = m_tileBorderFraction + vExtent * (y & mask);

            r.u1 = r.u0 + uExtent;
            r.v1 = r.v0 + vExtent;
        }
    }

    return r;
}

