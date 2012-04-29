/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_HIERARCHICAL_TILED_MAP_H_
#define _VESTA_HIERARCHICAL_TILED_MAP_H_

#include "TiledMap.h"
#include "IntegerTypes.h"
#include <string>
#include <map>


namespace vesta
{

/** HierarchicalTiledMap contains a set of tiles of (ideally) the same resolution, with
  * each lower level of the map having four times as many tiles as the one above it.
  * (i.e. if level N contains m x n tiles, N+1 will have 2m x 2n tiles.
  *
  * HierarchicalTiledMap is an abstract class. Subclasses are responsible for implementing
  * the tileResourceIdentifier method which maps a tile address (level, column, row) to a
  * a string. The interpretation of the string is up to the TextureMapLoader. Depending on
  * the loader, the string could be a filename, an URL, or something else.
  */
class HierarchicalTiledMap : public TiledMap
{
public:
    HierarchicalTiledMap(TextureMapLoader* loader, unsigned int tileSize);
    virtual ~HierarchicalTiledMap();

    virtual TextureSubrect tile(unsigned int level, unsigned int x, unsigned int y);

    /** Subclasses must implement this method to generate a resource identifier string
      * from the level, column, and row of the tile.
      */
    virtual std::string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row) = 0;

    /** Return true if the tile address is valid.
      */
    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row) = 0;

    /** Return true if the tile resource exists. Subclasses can implement this to optimize
      * tile loading in cases when it's simple to verify that a tile resource is available
      * (i.e. loading files from a local disk.)
      */
    virtual bool tileResourceExists(const std::string& /* resourceId */)
    {
        return true;
    }

    TextureMapLoader* loader() const
    {
        return m_loader;
    }

    virtual unsigned int tileSize() const
    {
        return m_tileSize;
    }

    /** Return the border thickness as a fraction of the overall tile size.
      *
      * \see setTileInset
      */
    float tileBorderFraction() const
    {
        return m_tileBorderFraction;
    }

    /** Set the tile border thickness as a fraction of overall tile size. By default,
      * it is zero, and the tile has no border pixels. It can be set to a non-zero
      * value in order to reduce or eliminate visible discontinuities across tiles.
      * The border pixels should duplicate pixels from adjacent tiles.
      *
      * Example: 256x256 pixel tiles with a 1 pixel border will have a tile border
      * fraction of 1/512 = 0.00390625.
      */
    void setTileBorderFraction(float fraction)
    {
        m_tileBorderFraction = fraction;
    }

private:
    TextureMapLoader* m_loader;

    // TODO: a hash table would be a better fit here
    typedef std::map<v_uint64, counted_ptr<TextureMap> > TileCache;
    TileCache m_tiles;
    unsigned int m_tileSize;
    float m_tileBorderFraction;
};

}

#endif // _VESTA_HIERARCHICAL_TILED_MAP_H_
