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

private:
    TextureMapLoader* m_loader;

    // TODO: a hash table would be a better fit here
    typedef std::map<v_uint64, counted_ptr<TextureMap> > TileCache;
    TileCache m_tiles;
    unsigned int m_tileSize;
};

}

#endif // _VESTA_HIERARCHICAL_TILED_MAP_H_
