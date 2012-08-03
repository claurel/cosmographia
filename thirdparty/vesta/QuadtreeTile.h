/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_QUADTREE_TILE_H_
#define _VESTA_QUADTREE_TILE_H_

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <deque>


// The QuadtreeTile class is used for level of detail when rendering
// planet geometry.

#include "IntegerTypes.h"


namespace vesta
{
class RenderContext;
class Material;
class TiledMap;
class MapLayer;
class QuadtreeTileAllocator;
class WorldLayer;
class WorldGeometry;

struct CullingPlaneSet
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Eigen::Hyperplane<float, 3> planes[6];
};


class QuadtreeTile
{
friend class QuadtreeTileAllocator;

public:
    enum Direction
    {
        East  = 0,
        North = 1,
        West  = 2,
        South = 3
    };

    enum Quadrant
    {
        Northeast = 0,
        Northwest = 1,
        Southwest = 2,
        Southeast = 3
    };

    enum
    {
        NormalMap = 0x1,
        Normals   = 0x2
    };

    QuadtreeTile();
    QuadtreeTile(QuadtreeTile* parent, unsigned int quadrant, const Eigen::Vector3f& semiAxes);

    void setNeighbor(Direction direction, QuadtreeTile* tile)
    {
        m_neighbors[(int) direction] = tile;
    }

    void tessellate(const Eigen::Vector3f& eyePosition,
                    const CullingPlaneSet& cullFrustum,
                    const Eigen::Vector3f& globeSemiAxes,
                    float splitThreshold,
                    float pixelSize);
    void split(const CullingPlaneSet& cullFrustum, const Eigen::Vector3f& semiAxes);
    bool cull(const CullingPlaneSet& cullFrustum) const;
    void render(RenderContext& rc, unsigned int features) const;
    void render(RenderContext& rc, Material& material, TiledMap* tiledMap, unsigned int features) const;
    void render(RenderContext& rc, Material& material, TiledMap* baseMap, TiledMap* normalMap) const;
    void render(RenderContext& rc, const MapLayer& layer, unsigned int features) const;
    void renderWorldLayer(RenderContext& rc, const WorldGeometry* world, const WorldLayer* layer) const;
    void drawPatch(RenderContext& rc, unsigned int features) const;
    void drawPatch(RenderContext& rc, const MapLayer& layer, unsigned int features) const;
    void drawPatch(RenderContext& rc, Material& material, TiledMap* baseMap, unsigned int features) const;
    void drawPatch(RenderContext& rc, Material& material, TiledMap* baseMap, TiledMap* normalMap) const;

    bool isRoot() const
    {
        return !m_parent;
    }

    bool hasChildren() const
    {
        // The following test works because a tile has either no children or a full set.
        return m_children[0] != NULL;
    }

    bool isCulled() const
    {
        return m_isCulled;
    }

    Eigen::Vector2f southwest() const
    {
        return m_southwest;
    }

    float extent() const
    {
        return m_extent;
    }

    void link(Direction direction, QuadtreeTile* neighbor)
    {
        m_neighbors[direction] = neighbor;
        if (neighbor)
        {
            Direction opposing = Direction(((unsigned int) direction + 2) & 0x3);
            neighbor->m_neighbors[opposing] = this;
        }
    }

    /** Number of tiles on each side of a tile.
      */
    static const unsigned int TileSubdivision = 16;

private:
    void computeCenterAndRadius(const Eigen::Vector3f& semiAxes);
    void drawTriangles(RenderContext& rc) const;

    static bool createTileMeshIndices();

private:
    QuadtreeTile* m_parent;
    QuadtreeTile* m_neighbors[4];
    QuadtreeTile* m_children[4];
    QuadtreeTileAllocator* m_allocator;
    unsigned int m_level;
    unsigned int m_row;
    unsigned int m_column;
    Eigen::Vector2f m_southwest;
    float m_extent;
    Eigen::Vector3f m_center;
    float m_boundingSphereRadius;
    float m_approxPixelSize;
    bool m_isCulled;

    static v_uint16* ms_tileMeshIndices[16];
    static unsigned int ms_tileMeshTriangleCounts[16];
    static bool ms_indicesInitialized;
};


class QuadtreeTileAllocator
{
public:
    QuadtreeTileAllocator()
    {
    }

    QuadtreeTile* newRootTile(unsigned int row, unsigned int column,
                              const Eigen::Vector2f& southwest, float extent,
                              const Eigen::Vector3f& semiAxes)
    {
        QuadtreeTile tile;
        tile.m_allocator = this;
        tile.m_row = row;
        tile.m_column = column;
        tile.m_southwest = southwest;
        tile.m_extent = extent;
        tile.computeCenterAndRadius(semiAxes);

        m_tilePool.push_back(tile);
        return &m_tilePool.back();
    }

    QuadtreeTile* newTile(QuadtreeTile* parent, unsigned int whichChild,
                          const Eigen::Vector3f& semiAxes)
    {
        QuadtreeTile tile(parent, whichChild, semiAxes);
        m_tilePool.push_back(tile);
        return &m_tilePool.back();
    }

    unsigned int tileCount() const
    {
        return m_tilePool.size();
    }

    void clear()
    {
        m_tilePool.clear();
    }

    typedef std::deque<QuadtreeTile> TileArray;

    const TileArray& tiles() const
    {
        return m_tilePool;
    }

private:
    TileArray m_tilePool;
};

} // namespace vesta

#endif // _VESTA_QUADTREE_TILE_H_
