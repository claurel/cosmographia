/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "QuadtreeTile.h"
#include "Units.h"
#include "MapLayer.h"
#include "RenderContext.h"
#include "TiledMap.h"
#include "WorldLayer.h"
#include "Debug.h"
#include <vector>
#include <algorithm>
#include <cassert>

using namespace vesta;
using namespace Eigen;
using namespace std;


// QuadtreeTile implements a 'restricted quadtree': adjacent
// tiles are not allowed to have more differ by more than one
// level of subdivision. This greatly simplifies the process of stitching
// together tiles to avoid cracks.
//
// The neighbor pointers of a tile will be null when no neighbors exist
// at the current level of detail. When a tile is split into four child
// tiles, missing neighbors must first be created in order to prevent violating
// the level of detail restriction.
//
// Each tile is a patch of the ellipsoid surface, subdivided to TileSubdivision
// squares per side. Batching is essential for performance on modern GPUs, since
// the cost to cull at per-quad granularity is much higher than the cost of
// rendering a few extra primitives.
//
// Some care is required when two adjacent tiles have different LODs. The
// diagram below shows the bottom two rows of a 4x4 tile. Vertices are indicated by
// plus signs. The size of the tile refers to the number of square cells it contains;
// an N x N tile will have (N + 1) x (N + 1) vertices.
//
//    +--+--+--+--+
//    |\ | /|\ | /|
//    | \|/ | \|/ |
//    +--+--+--+--+
//    | /|\ | /|\ |
//    |/ | \|/ | \|
//    +--+--+--+--+
//
// In order to avoid troublesome 'T' junctions, a different triangulation is used for
// any edge of a finely tessellated tile that abuts a more coarsely tessellated tile. This
// diagram shows the section of the above tile with a coarsely tessellated tile below
// it:
//
//    +--+--+--+--+
//    |\ | /|\ | /|    Fine tessellated tile, row N - 1
//    | \|/ | \|/ |
//    +--+--+--+--+
//    | / \ | / \ |    Finely tessellated tile, row N
//    |/   \|/   \|
//    +--+--+--+--+
//    |\    |    /|
//    | \   |   / |
//    |  \  |  /  |    Coarsely tesselated tile, row 1
//    |   \ | /   |
//    |    \|/    |
//    +-----+-----+
//
// In the code, a tile edge that abuts a more coarsely tessellated tile is called a
// transitional edge.
//
// The tile vertices are _not_ affected by the tessellation level of neighboring tiles;
// it is only the triangulation that changes. There are sixteen different triangulations
// possible: all combinations of transitional or normal edges in the north, south, east
// and west directions.


// Indices for all 16 possible tile triangulations. These are generated once and reused.
// TODO: These should eventually be stored in GPU index buffers.
v_uint16* QuadtreeTile::ms_tileMeshIndices[16];
unsigned int QuadtreeTile::ms_tileMeshTriangleCounts[16];
bool QuadtreeTile::ms_indicesInitialized = false;

static const float SquareSize = 1.0f / float(QuadtreeTile::TileSubdivision);


static VertexAttribute posNormTexTangentAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::Normal,       VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::TextureCoord, VertexAttribute::Float2),
    VertexAttribute(VertexAttribute::Tangent,      VertexAttribute::Float3),
};

static VertexSpec PositionNormalTexTangent(4, posNormTexTangentAttributes);


// TileTriangulationBuilder is a utility class used to construct tile meshes. Only 16
// unique tile meshes are used, and they are generated just once.
class TileTriangulationBuilder
{
public:
    TileTriangulationBuilder(unsigned int subdivision) :
        m_subdivision(subdivision)
    {
        assert(m_subdivision > 1);
        assert((m_subdivision & 1) == 0); // Require an even count of cells in each dimension
    }

    unsigned int triangleCount() const
    {
        return m_vertexIndices.size() / 3;
    }

    // Add a new triangle with the specified vertex indices
    void addTriangle(v_uint16 t0, v_uint16 t1, v_uint16 t2)
    {
        m_vertexIndices.push_back(t0);
        m_vertexIndices.push_back(t1);
        m_vertexIndices.push_back(t2);
    }

    // Generate vertices for a triangle in the specified cell of the tile mesh.
    // The 'half' parameter specifies the half of the cell that should be filled
    // by the triangle. The vertices are always ordered counterclockwise (as seen
    // from the exterior of the sphere.)
    void fillHalfCell(unsigned int x, unsigned int y, QuadtreeTile::Quadrant half)
    {
        v_uint16 i00 = v_uint16(y * (m_subdivision + 1) + x);
        v_uint16 i01 = i00 + 1;
        v_uint16 i10 = i00 + v_uint16(m_subdivision + 1);
        v_uint16 i11 = i10 + 1;

        switch (half)
        {
        case QuadtreeTile::Northeast:
            addTriangle(i01, i11, i10);
            break;
        case QuadtreeTile::Southwest:
            addTriangle(i00, i01, i10);
            break;
        case QuadtreeTile::Southeast:
            addTriangle(i00, i01, i11);
            break;
        case QuadtreeTile::Northwest:
            addTriangle(i00, i11, i10);
            break;
        }
    }

    // Fill a tile cell with two triangles. The split of the cell
    // (NW-to-SE or NE-to-SW) depends on its position in the mesh.
    void fillCell(unsigned int x, unsigned int y)
    {
        unsigned int diagonal = (x + y) & 1;

        v_uint16 i00 = v_uint16(y * (m_subdivision + 1) + x);
        v_uint16 i01 = i00 + 1;
        v_uint16 i10 = i00 + v_uint16(m_subdivision + 1);
        v_uint16 i11 = i10 + 1;

        if (diagonal == 0)
        {
            addTriangle(i00, i01, i11);
            addTriangle(i00, i11, i10);
        }
        else
        {
            addTriangle(i01, i11, i10);
            addTriangle(i01, i10, i00);
        }
    }

    // Create the edge triangles of a tile mesh that will match with the
    // edge of an adjacent tile with the same or finer tessellation.
    void generateEdge(QuadtreeTile::Direction edge)
    {
        unsigned int x = 0;
        unsigned int y = 0;
        unsigned int xstep = 0;
        unsigned int ystep = 0;

        const unsigned int last = m_subdivision - 1;
        switch (edge)
        {
        case QuadtreeTile::North:
            fillHalfCell(0,    last, QuadtreeTile::Northeast);
            fillHalfCell(last, last, QuadtreeTile::Northwest);
            y = last;
            xstep = 1;
            break;
        case QuadtreeTile::East:
            fillHalfCell(last, 0,    QuadtreeTile::Northeast);
            fillHalfCell(last, last, QuadtreeTile::Southeast);
            x = last;
            ystep = 1;
            break;
        case QuadtreeTile::South:
            fillHalfCell(0,    0, QuadtreeTile::Southeast);
            fillHalfCell(last, 0, QuadtreeTile::Southwest);
            xstep = 1;
            break;
        case QuadtreeTile::West:
            fillHalfCell(0, 0,    QuadtreeTile::Northwest);
            fillHalfCell(0, last, QuadtreeTile::Southwest);
            ystep = 1;
            break;
        }

        x += xstep;
        y += ystep;

        for (unsigned int i = 1; i < m_subdivision - 1; ++i)
        {
            fillCell(x, y);
            x += xstep;
            y += ystep;
        }
    }


    // Create the edge triangles of a tile mesh that will match with the
    // edge of an adjacent tile with coarser tessellation.
    void generateTransitionEdge(QuadtreeTile::Direction edge)
    {
        const unsigned int last = m_subdivision - 1;

        for (unsigned int i = 0; i < m_subdivision; i += 2)
        {
            unsigned int i0 = 0;
            unsigned int i1 = 0;
            unsigned int i2 = 0;

            switch (edge)
            {
            case QuadtreeTile::North:
                if (i > 0)
                {
                    fillHalfCell(i,     last, QuadtreeTile::Southwest);
                }
                if (i < last - 1)
                {
                    fillHalfCell(i + 1, last, QuadtreeTile::Southeast);
                }
                i0 = m_subdivision * (m_subdivision + 1) + i;
                i1 = i0 - (m_subdivision + 1) + 1;
                i2 = i0 + 2;
                break;
            case QuadtreeTile::East:
                if (i > 0)
                {
                    fillHalfCell(last, i,     QuadtreeTile::Southwest);
                }
                if (i < last - 1)
                {
                    fillHalfCell(last, i + 1, QuadtreeTile::Northwest);
                }
                i0 = i * (m_subdivision + 1) + m_subdivision;
                i1 = i0 + 2 * (m_subdivision + 1);
                i2 = i0 + (m_subdivision + 1) - 1;
                break;
            case QuadtreeTile::South:
                if (i > 0)
                {
                    fillHalfCell(i,     0, QuadtreeTile::Northwest);
                }
                if (i < last - 1)
                {
                    fillHalfCell(i + 1, 0, QuadtreeTile::Northeast);
                }
                i0 = i;
                i1 = i0 + 2;
                i2 = i0 + (m_subdivision + 1) + 1;
                break;
            case QuadtreeTile::West:
                if (i > 0)
                {
                    fillHalfCell(0, i,     QuadtreeTile::Southeast);
                }
                if (i < last - 1)
                {
                    fillHalfCell(0, i + 1, QuadtreeTile::Northeast);
                }
                i0 = i * (m_subdivision + 1);
                i1 = i0 + (m_subdivision + 1) + 1;
                i2 = i0 + 2 * (m_subdivision + 1);
                break;
            }

            addTriangle(i0, i1, i2);
        }
    }

    // Create a tile mesh. The transitionEdges parameter specifies which adjacent tiles are
    // more coarsely tesselated (and thus which edges are transition edges.)
    void generateTile(unsigned int transitionEdges)
    {
        for (unsigned int y = 1; y < m_subdivision - 1; ++y)
        {
            for (unsigned int x = 1; x < m_subdivision - 1; ++x)
            {
                fillCell(x, y);
            }
        }

        for (unsigned int i = 0; i < 4; ++i)
        {
            QuadtreeTile::Direction dir = (QuadtreeTile::Direction) i;
            if ((transitionEdges & (1 << i)) != 0)
            {
                generateTransitionEdge(dir);
            }
            else
            {
                generateEdge(dir);
            }
        }
    }

    // Allocate a block of memory and fill it with all the vertex indices for the tile mesh
    v_uint16* allocateIndexList()
    {
        v_uint16* indices = NULL;
        if (!m_vertexIndices.empty())
        {
            indices = new v_uint16[m_vertexIndices.size()];
            if (indices)
            {
                std::copy(m_vertexIndices.begin(), m_vertexIndices.end(), indices);
            }
        }

        return indices;
    }

private:
    unsigned int m_subdivision;
    vector<v_uint16> m_vertexIndices;
};



QuadtreeTile::QuadtreeTile() :
    m_parent(NULL),
    m_level(NULL),
    m_approxPixelSize(0.0f),
    m_isCulled(false)
{
    for (unsigned int i = 0; i < 4; ++i)
    {
        m_neighbors[i] = NULL;
        m_children[i] = NULL;
    }
}


QuadtreeTile::QuadtreeTile(QuadtreeTile* parent, unsigned int quadrant, const Vector3f& semiAxes) :
    m_parent(parent),
    m_allocator(parent->m_allocator),
    m_level(parent->m_level + 1),
    m_extent(parent->m_extent * 0.5f),
    m_approxPixelSize(parent->m_approxPixelSize),
    m_isCulled(parent->m_isCulled)
{
    for (unsigned int i = 0; i < 4; ++i)
    {
        m_neighbors[i] = NULL;
        m_children[i] = NULL;
    }

    switch (quadrant)
    {
    case Northeast:
        m_column = parent->m_column * 2 + 1;
        m_row = parent->m_row * 2 + 1;
        m_southwest = parent->m_southwest + Vector2f(m_extent, m_extent);
        break;
    case Northwest:
        m_column = parent->m_column * 2;
        m_row = parent->m_row * 2 + 1;
        m_southwest = parent->m_southwest + Vector2f(0.0f, m_extent);
        break;
    case Southwest:
        m_column = parent->m_column * 2;
        m_row = parent->m_row * 2;
        m_southwest = parent->m_southwest + Vector2f(0.0f, 0.0f);
        break;
    case Southeast:
        m_column = parent->m_column * 2 + 1;
        m_row = parent->m_row * 2;
        m_southwest = parent->m_southwest + Vector2f(m_extent, 0.0f);
        break;
    }

    computeCenterAndRadius(semiAxes);
}


void
QuadtreeTile::tessellate(const Vector3f& eyePosition,
                         const CullingPlaneSet& cullPlanes,
                         const Vector3f& globeSemiAxes,
                         float splitThreshold,
                         float pixelSize)
{
    float tileArc = float(PI) * m_extent;

    // Compute the approximate altitude of the eye point. This is the exact altitude when the
    // world is a sphere, but larger than the actual altitude for other ellipsoids.
    //float approxAltitude = abs(eyePosition.norm() - (eyePosition.normalized().cwise() * globeSemiAxes).norm());
    float distToCenter = eyePosition.norm();
    float approxAltitude = abs(distToCenter - ((eyePosition.cwise() * globeSemiAxes).norm()) / max(1.0e-6f, distToCenter));

    // Compute the approximate projected size of the tile.
    float distanceToTile = max(approxAltitude, (eyePosition - m_center).norm() - m_boundingSphereRadius);
    distanceToTile = max(1.0e-6f, distanceToTile);
    float apparentTileSize = m_boundingSphereRadius / distanceToTile;

    // Compute the approximate projected size of the tile, in pixel
    m_approxPixelSize = apparentTileSize / pixelSize;

    // We may also need to split a tile when the error from approximating a
    // curve as a straight line gets too large. In practice, this is mostly
    // an issue at larger distances.
    //
    // The error expression is derived from formula for the maximum distance
    // from a unit circle to the chord of angle theta: 1 - cos(angle / 2)
    float curveApproxError = globeSemiAxes.maxCoeff() * (1.0f - cos(tileArc * SquareSize * 0.5f));
    float curveErrorPixels = curveApproxError / (distanceToTile * pixelSize);

    // Tessellate when the tile is too large or the curve approximation error is too great
    if (apparentTileSize > splitThreshold || curveErrorPixels > 0.5f)
    {
        // Only split tiles that lie inside the view frustum.
        if (!m_isCulled)
        {
            split(cullPlanes, globeSemiAxes);
            for (unsigned int i = 0; i < 4; ++i)
            {
                m_children[i]->tessellate(eyePosition, cullPlanes, globeSemiAxes, splitThreshold, pixelSize);
            }
        }
    }
}


// Split a tile into four child tiles. If
void
QuadtreeTile::split(const CullingPlaneSet& cullPlanes, const Vector3f& semiAxes)
{
    if (hasChildren())
    {
        return;
    }

    // Split any neighbor tiles are shallower (coarser) than this one. This
    // prevents neighboring tiles from ending up with tessellations that
    // differ by more than one level.
    if (!isRoot())
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            if (!m_neighbors[i])
            {
                // Parent may not have a neighbor if (and only if!) we're at the edge of the map
                if (m_parent->m_neighbors[i])
                {
                    m_parent->m_neighbors[i]->split(cullPlanes, semiAxes);
                }
            }
        }
    }

    // Create children; mark any that lie outside the view frustum as culled
    for (unsigned int i = 0; i < 4; ++i)
    {
        m_children[i] = m_allocator->newTile(this, Quadrant(i), semiAxes);
        if (!m_isCulled)
        {
            m_children[i]->m_isCulled = m_children[i]->cull(cullPlanes);
        }
    }

    // Link neighbor tiles
    m_children[Northeast]->link(South, m_children[Southeast]);
    m_children[Northwest]->link(South, m_children[Southwest]);
    m_children[Northeast]->link(West, m_children[Northwest]);
    m_children[Southeast]->link(West, m_children[Southwest]);

    // We need to check for the existence of neighbors, as they might be
    // missing for root nodes.
    if (m_neighbors[North])
    {
        m_children[Northeast]->link(North, m_neighbors[North]->m_children[Southeast]);
        m_children[Northwest]->link(North, m_neighbors[North]->m_children[Southwest]);
    }

    if (m_neighbors[West])
    {
        m_children[Northwest]->link(West, m_neighbors[West]->m_children[Northeast]);
        m_children[Southwest]->link(West, m_neighbors[West]->m_children[Southeast]);
    }

    if (m_neighbors[South])
    {
        m_children[Southeast]->link(South, m_neighbors[South]->m_children[Northeast]);
        m_children[Southwest]->link(South, m_neighbors[South]->m_children[Northwest]);
    }

    if (m_neighbors[East])
    {
        m_children[Southeast]->link(East, m_neighbors[East]->m_children[Southwest]);
        m_children[Northeast]->link(East, m_neighbors[East]->m_children[Northwest]);
    }
}


// Return true if this tile lies outside the convex volume given by
// the intersection of half-spaces.
bool
QuadtreeTile::cull(const CullingPlaneSet& cullPlanes) const
{
    // Test the sphere against all of the planes in the cull frustum
    for (unsigned int i = 0; i < 6; ++i)
    {
        if (cullPlanes.planes[i].signedDistance(m_center) < -m_boundingSphereRadius)
        {
            return true;
        }
    }

    return false;
}


void
QuadtreeTile::render(RenderContext& rc, unsigned int features) const
{
    if (!m_isCulled)
    {
        if (hasChildren())
        {
            for (unsigned int i = 0; i < 4; ++i)
            {
                m_children[i]->render(rc, features);
            }
        }
        else
        {
            drawPatch(rc, features);
        }
    }
}


void
QuadtreeTile::render(RenderContext& rc, Material& material, TiledMap* baseTexture, unsigned int features) const
{
    if (!m_isCulled)
    {
        if (hasChildren())
        {
            for (unsigned int i = 0; i < 4; ++i)
            {
                m_children[i]->render(rc, material, baseTexture, features);
            }
        }
        else
        {
            drawPatch(rc, material, baseTexture, features);
        }
    }
}


void
QuadtreeTile::render(RenderContext& rc, Material& material, TiledMap* baseTexture, TiledMap* normalTexture) const
{
    if (!m_isCulled)
    {
        if (hasChildren())
        {
            for (unsigned int i = 0; i < 4; ++i)
            {
                m_children[i]->render(rc, material, baseTexture, normalTexture);
            }
        }
        else
        {
            drawPatch(rc, material, baseTexture, normalTexture);
        }
    }
}


void
QuadtreeTile::render(RenderContext& rc, const MapLayer& layer, unsigned int features) const
{
    if (!m_isCulled)
    {
        float tileArc = float(PI) * m_extent;
        float lonWest = float(PI) * m_southwest.x();
        float lonEast = lonWest + tileArc;
        float latSouth = float(PI) * m_southwest.y();
        float latNorth = latSouth + tileArc;

        // Check whether map layer lies completely outside this tile
        if (layer.box().west() > lonEast ||
            layer.box().east() < lonWest ||
            layer.box().south() > latNorth ||
            layer.box().north() < latSouth)
        {
            return;
        }

        if (hasChildren())
        {
            for (unsigned int i = 0; i < 4; ++i)
            {
                m_children[i]->render(rc, layer, features);
            }
        }
        else
        {
            if (layer.box().west() <= lonWest &&
                layer.box().east() >= lonEast &&
                layer.box().south() <= latSouth &&
                layer.box().north() >= latNorth)
            {
                // Draw a complete patch
                drawPatch(rc, layer, features);
            }
            else
            {
                // Patch isn't completely covered by the map layer; draw
                // only a portion of the patch.
                drawPatch(rc, layer, features);
            }
        }
    }
}


void
QuadtreeTile::renderWorldLayer(RenderContext& rc, const WorldGeometry* world, const WorldLayer* layer) const
{
    if (!m_isCulled)
    {
        if (hasChildren())
        {
            for (unsigned int i = 0; i < 4; ++i)
            {
                m_children[i]->renderWorldLayer(rc, world, layer);
            }
        }
        else
        {
            layer->renderTile(rc, world, this);
        }
    }
}


// Draw a patch with an ordinary texture
void
QuadtreeTile::drawPatch(RenderContext& rc, unsigned int features) const
{
    const unsigned int MaxVertexSize = 11;
    unsigned int vertexStride = 5;

    if ((features & NormalMap) != 0)
    {
        vertexStride = 11;
    }
    else if ((features & Normals) != 0)
    {
        vertexStride = 8;
    }

    const unsigned int vertexCount = (TileSubdivision + 1) * (TileSubdivision + 1);

    float vertexData[vertexCount * MaxVertexSize];
    unsigned int vertexIndex = 0;

    float tileArc = float(PI) * m_extent;
    float lonWest = float(PI) * m_southwest.x();
    float latSouth = float(PI) * m_southwest.y();
    float dlon = tileArc / float(TileSubdivision);
    float dlat = tileArc / float(TileSubdivision);
    float du = m_extent / float(TileSubdivision);
    float dv = m_extent / float(TileSubdivision);

    // Precompute a trig table for this patch
    float sines[TileSubdivision + 1];
    float cosines[TileSubdivision + 1];
    for (unsigned int i = 0; i <= TileSubdivision; ++i)
    {
        float lon = lonWest + i * dlon;
        sines[i] = sin(lon);
        cosines[i] = cos(lon);
    }

    for (unsigned int i = 0; i <= TileSubdivision; ++i)
    {
        float v = m_southwest.y() + i * dv;
        float lat = latSouth + i * dlat;
        float cosLat = cos(lat);
        float sinLat = sin(lat);

        if ((features & NormalMap) != 0)
        {
            for (unsigned int j = 0; j <= TileSubdivision; ++j)
            {
                unsigned int vertexStart = vertexStride * vertexIndex;

                float u = m_southwest.x() + j * du;

                float cosLon = cosines[j];
                float sinLon = sines[j];

                Vector3f p(cosLat * cosLon, cosLat * sinLon, sinLat);

                // Position
                vertexData[vertexStart + 0]  = p.x();
                vertexData[vertexStart + 1]  = p.y();
                vertexData[vertexStart + 2]  = p.z();

                // Vertex normal
                vertexData[vertexStart + 3]  = p.x();
                vertexData[vertexStart + 4]  = p.y();
                vertexData[vertexStart + 5]  = p.z();

                // Texture coordinate
                vertexData[vertexStart + 6]  = u * 0.5f + 0.5f;
                vertexData[vertexStart + 7]  = 0.5f - v;

                // Tangent (we use dP/du), where P(u,v) is the sphere parametrization
                vertexData[vertexStart + 8]  = -sinLon;
                vertexData[vertexStart + 9]  = cosLon;
                vertexData[vertexStart + 10] = 0.0f;

                ++vertexIndex;
            }
        }
        else if ((features & Normals) != 0)
        {
            for (unsigned int j = 0; j <= TileSubdivision; ++j)
            {
                unsigned int vertexStart = vertexStride * vertexIndex;

                float u = m_southwest.x() + j * du;

                Vector3f p(cosLat * cosines[j], cosLat * sines[j], sinLat);
                vertexData[vertexStart + 0] = p.x();
                vertexData[vertexStart + 1] = p.y();
                vertexData[vertexStart + 2] = p.z();
                vertexData[vertexStart + 3] = p.x();
                vertexData[vertexStart + 4] = p.y();
                vertexData[vertexStart + 5] = p.z();
                vertexData[vertexStart + 6] = u * 0.5f + 0.5f;
                vertexData[vertexStart + 7] = 0.5f - v;

                ++vertexIndex;
            }
        }
        else
        {
            for (unsigned int j = 0; j <= TileSubdivision; ++j)
            {
                unsigned int vertexStart = vertexStride * vertexIndex;

                float u = m_southwest.x() + j * du;

                Vector3f p(cosLat * cosines[j], cosLat * sines[j], sinLat);
                vertexData[vertexStart + 0] = p.x();
                vertexData[vertexStart + 1] = p.y();
                vertexData[vertexStart + 2] = p.z();
                vertexData[vertexStart + 3] = u * 0.5f + 0.5f;
                vertexData[vertexStart + 4] = 0.5f - v;

                ++vertexIndex;
            }
        }
    }

    if ((features & NormalMap) != 0)
    {
        rc.bindVertexArray(PositionNormalTexTangent, vertexData, vertexStride * 4);
    }
    else if ((features & Normals) != 0)
    {
        rc.bindVertexArray(VertexSpec::PositionNormalTex, vertexData, vertexStride * 4);
    }
    else
    {
        rc.bindVertexArray(VertexSpec::PositionTex, vertexData, vertexStride * 4);
    }

    drawTriangles(rc);
}


// Draw a patch with a tiled texture map
void
QuadtreeTile::drawPatch(RenderContext& rc, Material& material, TiledMap* baseMap, unsigned int features) const
{
    const unsigned int MaxVertexSize = 11;
    unsigned int vertexStride = 8;

    const unsigned int vertexCount = (TileSubdivision + 1) * (TileSubdivision + 1);

    float vertexData[vertexCount * MaxVertexSize];
    unsigned int vertexIndex = 0;

    float tileArc = float(PI) * m_extent;
    float lonWest = float(PI) * m_southwest.x();
    float latSouth = float(PI) * m_southwest.y();
    float dlon = tileArc / float(TileSubdivision);
    float dlat = tileArc / float(TileSubdivision);

    float tileSize = static_cast<float>(baseMap->tileSize());

    unsigned int mapLevel = m_level;
    unsigned int mapColumn = m_column;
    unsigned int mapRow = m_row;

    // If the projected size in pixels of the tile is less than the number texels
    // in the tile, then we can use a lower resolution version of the tile.
    if (m_approxPixelSize < tileSize)
    {
        unsigned int n = tileSize / m_approxPixelSize;
        while (n > 0 && mapLevel > 0)
        {
            n >>= 1;
            mapColumn >>= 1;
            mapRow >>= 1;
            mapLevel--;
        }
    }

    float u0;
    float v0;
    float du;
    float dv;

    TiledMap::TextureSubrect r = baseMap->tile(mapLevel, mapColumn, mapRow);
    if (mapLevel >= m_level)
    {
        u0 = r.u0;
        v0 = r.v0;
        du = (r.u1 - r.u0) / float(TileSubdivision);
        dv = (r.v1 - r.v0) / float(TileSubdivision);
    }
    else
    {
        unsigned int log2Scale = m_level - mapLevel;
        float scale = 1.0f / float(1 << log2Scale);
        unsigned int mask = (1 << log2Scale) - 1;

        float uExt = (r.u1 - r.u0) * scale;
        float vExt = (r.v1 - r.v0) * scale;

        u0 = r.u0 + uExt * (m_column & mask);
        v0 = r.v0 + vExt * (m_row & mask);
        du = uExt / float(TileSubdivision);
        dv = vExt / float(TileSubdivision);
    }

    // Precompute a trig table for this patch
    float sines[TileSubdivision + 1];
    float cosines[TileSubdivision + 1];
    for (unsigned int i = 0; i <= TileSubdivision; ++i)
    {
        float lon = lonWest + i * dlon;
        sines[i] = sin(lon);
        cosines[i] = cos(lon);
    }

    for (unsigned int i = 0; i <= TileSubdivision; ++i)
    {
        float v = v0 + i * dv;
        float lat = latSouth + i * dlat;
        float cosLat = cos(lat);
        float sinLat = sin(lat);

        for (unsigned int j = 0; j <= TileSubdivision; ++j)
        {
            unsigned int vertexStart = vertexStride * vertexIndex;

            float u = u0 + j * du;

            Vector3f p(cosLat * cosines[j], cosLat * sines[j], sinLat);
            vertexData[vertexStart + 0] = p.x();
            vertexData[vertexStart + 1] = p.y();
            vertexData[vertexStart + 2] = p.z();
            vertexData[vertexStart + 3] = p.x();
            vertexData[vertexStart + 4] = p.y();
            vertexData[vertexStart + 5] = p.z();
            vertexData[vertexStart + 6] = u;
            vertexData[vertexStart + 7] = 1.0f - v;

            ++vertexIndex;
        }
    }

    material.setBaseTexture(r.texture);
    rc.bindMaterial(&material);

    if ((features & NormalMap) != 0)
    {
        rc.bindVertexArray(PositionNormalTexTangent, vertexData, vertexStride * 4);
    }
    else if ((features & Normals) != 0)
    {
        rc.bindVertexArray(VertexSpec::PositionNormalTex, vertexData, vertexStride * 4);
    }
    else
    {
        rc.bindVertexArray(VertexSpec::PositionTex, vertexData, vertexStride * 4);
    }

    drawTriangles(rc);
}


// Draw a patch with a tiled base texture and normal map
void
QuadtreeTile::drawPatch(RenderContext& rc, Material& material, TiledMap* baseMap, TiledMap* normalMap) const
{
    const unsigned int MaxVertexSize = 11;
    unsigned int vertexStride = 11;

    const unsigned int vertexCount = (TileSubdivision + 1) * (TileSubdivision + 1);

    float vertexData[vertexCount * MaxVertexSize];
    unsigned int vertexIndex = 0;

    float tileArc = float(PI) * m_extent;
    float lonWest = float(PI) * m_southwest.x();
    float latSouth = float(PI) * m_southwest.y();
    float dlon = tileArc / float(TileSubdivision);
    float dlat = tileArc / float(TileSubdivision);

    float tileSize = static_cast<float>(baseMap->tileSize());

    unsigned int mapLevel = m_level;
    unsigned int mapColumn = m_column;
    unsigned int mapRow = m_row;

    // If the projected size in pixels of the tile is less than the number texels
    // in the tile, then we can use a lower resolution version of the tile.
    if (m_approxPixelSize < tileSize)
    {
        unsigned int n = tileSize / m_approxPixelSize;
        while (n > 0 && mapLevel > 0)
        {
            n >>= 1;
            mapColumn >>= 1;
            mapRow >>= 1;
            mapLevel--;
        }
    }

    float u0;
    float v0;
    float du;
    float dv;

    TiledMap::TextureSubrect baseRect = baseMap->tile(mapLevel, mapColumn, mapRow);
    TiledMap::TextureSubrect normalMapRect = normalMap->tile(mapLevel, mapColumn, mapRow);

    // We need to have the same texture coordinates for all textures. If we have
    // a more detailed tile for one of the textures,
    float baseUExt = baseRect.u1 - baseRect.u0;
    float normalMapUExt = normalMapRect.u1 - normalMapRect.u0;
    if (baseUExt > normalMapUExt)
    {
        while (mapLevel > 0 && baseUExt > normalMapUExt)
        {
            mapColumn >>= 1;
            mapRow >>= 1;
            mapLevel--;
            baseRect = baseMap->tile(mapLevel, mapColumn, mapRow);
            baseUExt = baseRect.u1 - baseRect.u0;
        }
    }
    else if (normalMapUExt > baseUExt)
    {
        while (mapLevel > 0 && normalMapUExt > baseUExt)
        {
            mapColumn >>= 1;
            mapRow >>= 1;
            mapLevel--;
            normalMapRect = normalMap->tile(mapLevel, mapColumn, mapRow);
            normalMapUExt = normalMapRect.u1 - normalMapRect.u0;
        }
    }

    if (mapLevel >= m_level)
    {
        u0 = baseRect.u0;
        v0 = baseRect.v0;
        du = (baseRect.u1 - baseRect.u0) / float(TileSubdivision);
        dv = (baseRect.v1 - baseRect.v0) / float(TileSubdivision);
    }
    else
    {
        unsigned int log2Scale = m_level - mapLevel;
        float scale = 1.0f / float(1 << log2Scale);
        unsigned int mask = (1 << log2Scale) - 1;

        float uExt = (baseRect.u1 - baseRect.u0) * scale;
        float vExt = (baseRect.v1 - baseRect.v0) * scale;

        u0 = baseRect.u0 + uExt * (m_column & mask);
        v0 = baseRect.v0 + vExt * (m_row & mask);
        du = uExt / float(TileSubdivision);
        dv = vExt / float(TileSubdivision);
    }

    // Precompute a trig table for this patch
    float sines[TileSubdivision + 1];
    float cosines[TileSubdivision + 1];
    for (unsigned int i = 0; i <= TileSubdivision; ++i)
    {
        float lon = lonWest + i * dlon;
        sines[i] = sin(lon);
        cosines[i] = cos(lon);
    }

    for (unsigned int i = 0; i <= TileSubdivision; ++i)
    {
        float v = v0 + i * dv;
        float lat = latSouth + i * dlat;
        float cosLat = cos(lat);
        float sinLat = sin(lat);

        for (unsigned int j = 0; j <= TileSubdivision; ++j)
        {
            unsigned int vertexStart = vertexStride * vertexIndex;

            float u = u0 + j * du;

            Vector3f p(cosLat * cosines[j], cosLat * sines[j], sinLat);
            vertexData[vertexStart + 0] = p.x();
            vertexData[vertexStart + 1] = p.y();
            vertexData[vertexStart + 2] = p.z();
            vertexData[vertexStart + 3] = p.x();
            vertexData[vertexStart + 4] = p.y();
            vertexData[vertexStart + 5] = p.z();
            vertexData[vertexStart + 6] = u;
            vertexData[vertexStart + 7] = 1.0f - v;
            vertexData[vertexStart + 8]  = -sines[j];
            vertexData[vertexStart + 9]  = cosines[j];
            vertexData[vertexStart + 10] = 0.0f;

            ++vertexIndex;
        }
    }

    material.setBaseTexture(baseRect.texture);
    material.setNormalTexture(normalMapRect.texture);
    rc.bindMaterial(&material);

    rc.bindVertexArray(PositionNormalTexTangent, vertexData, vertexStride * 4);

    drawTriangles(rc);
}


inline void setVertex(float* vertexData, const Vector3f& p, float u, float v)
{
    vertexData[0] = p.x();
    vertexData[1] = p.y();
    vertexData[2] = p.z();
    vertexData[3] = p.x();
    vertexData[4] = p.y();
    vertexData[5] = p.z();
    vertexData[6] = u;
    vertexData[7] = 1.0f - v;
}


// Helper function to generate one row of vertices for a map
// layer patch.
static unsigned int
mapLayerRow(const MapLayer& layer,
            float* vertexData,
            unsigned int vertexIndex,
            unsigned int vertexStride,
            float lat, float v,
            float lonWest, float dlon,
            float u0, float du,
            unsigned int startColumn, unsigned int endColumn,
            bool westEdge, bool eastEdge)
{
    float cosLat = cos(lat);
    float sinLat = sin(lat);

    if (westEdge)
    {
        float lon = static_cast<float>(layer.box().west());
        Vector3f p(cosLat * cos(lon), cosLat * sin(lon), sinLat);
        setVertex(vertexData + vertexStride * vertexIndex, p, 0.0f, v);

        ++vertexIndex;
    }

    for (unsigned int j = startColumn; j <= endColumn; ++j)
    {
        float lon = lonWest + j * dlon;
        float u = u0 + j * du;
        Vector3f p(cosLat * cos(lon), cosLat * sin(lon), sinLat);

        setVertex(vertexData + vertexStride * vertexIndex, p, u, v);

        ++vertexIndex;
    }

    if (eastEdge)
    {
        float lon = static_cast<float>(layer.box().east());
        Vector3f p(cosLat * cos(lon), cosLat * sin(lon), sinLat);
        setVertex(vertexData + vertexStride * vertexIndex, p, 1.0f, v);

        ++vertexIndex;
    }

    return vertexIndex;
}


// Draw a patch for a map layer. Layers are rendered with exactly the same
// vertices as the base planet surface when possible in order to eliminate
// depth buffer artifacts. Using identical vertex coordinates may not be
// possible at the very edges of map layers.
void
QuadtreeTile::drawPatch(RenderContext& rc, const MapLayer& layer, unsigned int features) const
{
    const unsigned int vertexStride = 8;

    const unsigned int maxVertexCount = (TileSubdivision + 1) * (TileSubdivision + 1);
    const unsigned int maxTriangleCount = TileSubdivision * TileSubdivision * 2;
    float vertexData[maxVertexCount * vertexStride];
    v_uint16 indexData[maxTriangleCount * 3];

    float tileArc = float(PI) * m_extent;
    float lonWest = float(PI) * m_southwest.x();
    float latSouth = float(PI) * m_southwest.y();
    float dlon = tileArc / float(TileSubdivision);
    float dlat = tileArc / float(TileSubdivision);

    float layerLonExtent = static_cast<float>(layer.box().east() - layer.box().west());
    float layerLatExtent = static_cast<float>(layer.box().north() - layer.box().south());

    // Compute texture coordinate starting points and steps
    float u0 = static_cast<float>(lonWest - layer.box().west()) / layerLonExtent;
    float v0 = static_cast<float>(latSouth - layer.box().south()) / layerLatExtent;
    float du = tileArc / layerLonExtent / TileSubdivision;
    float dv = tileArc / layerLatExtent / TileSubdivision;

    // Handle map layers that only partially cover the current world patch. westEdge, etc.
    // flags will be set for any edge that crosses the world patch.
    bool westEdge = false;
    bool eastEdge = false;
    bool southEdge = false;
    bool northEdge = false;

    unsigned int startColumn = 0;
    if (lonWest < layer.box().west())
    {
        startColumn = static_cast<unsigned int>(ceil((layer.box().west() - lonWest) / tileArc * TileSubdivision));
        westEdge = true;
    }

    unsigned int endColumn = TileSubdivision;
    if (lonWest + tileArc > layer.box().east())
    {
        endColumn = floor((layer.box().east() - lonWest) / tileArc * TileSubdivision);
        eastEdge = true;
    }

    unsigned int startRow = 0;
    if (latSouth < layer.box().south())
    {
        startRow = ceil((layer.box().south() - latSouth) / tileArc * TileSubdivision);
        southEdge = true;
    }

    unsigned int endRow = TileSubdivision;
    if (latSouth + tileArc > layer.box().north())
    {
        endRow = floor((layer.box().north() - latSouth) / tileArc * TileSubdivision);
        northEdge = true;
    }

    // Generate vertices for the patch
    unsigned int vertexIndex = 0;
    if (southEdge)
    {
        vertexIndex = mapLayerRow(layer,
                                  vertexData, vertexIndex, vertexStride,
                                  layer.box().south(),
                                  0.0f,
                                  lonWest, dlon,
                                  u0, du,
                                  startColumn, endColumn, westEdge, eastEdge);
    }

    for (unsigned int i = startRow; i <= endRow; ++i)
    {
        vertexIndex = mapLayerRow(layer,
                                  vertexData, vertexIndex, vertexStride,
                                  latSouth + i * dlat,
                                  v0 + i * dv,
                                  lonWest, dlon,
                                  u0, du,
                                  startColumn, endColumn, westEdge, eastEdge);
    }

    if (northEdge)
    {
        vertexIndex = mapLayerRow(layer,
                                  vertexData, vertexIndex, vertexStride,
                                  layer.box().north(),
                                  1.0f,
                                  lonWest, dlon,
                                  u0, du,
                                  startColumn, endColumn, westEdge, eastEdge);
    }

    // Compute the total number of vertices and triangles in this patch.
    unsigned int columnCount = endColumn - startColumn;
    unsigned int rowCount = endRow - startRow;
    if (northEdge)
    {
        rowCount++;
    }
    if (southEdge)
    {
        rowCount++;
    }
    if (westEdge)
    {
        columnCount++;
    }
    if (eastEdge)
    {
        columnCount++;
    }

    // Generate indices for the patch
    unsigned int triangleIndex = 0;
    for (unsigned int i = 0; i < rowCount; ++i)
    {
        for (unsigned int j = 0; j < columnCount; ++j)
        {
            unsigned int diagonal = (i + j) & 1;

            v_uint16 i00 = v_uint16(i * (columnCount + 1) + j);
            v_uint16 i01 = i00 + 1;
            v_uint16 i10 = i00 + v_uint16(columnCount + 1);
            v_uint16 i11 = i10 + 1;

            if (diagonal == 0)
            {
                indexData[triangleIndex * 3 + 0] = i00;
                indexData[triangleIndex * 3 + 1] = i01;
                indexData[triangleIndex * 3 + 2] = i11;
                ++triangleIndex;

                indexData[triangleIndex * 3 + 0] = i00;
                indexData[triangleIndex * 3 + 1] = i11;
                indexData[triangleIndex * 3 + 2] = i10;
                ++triangleIndex;
            }
            else
            {
                indexData[triangleIndex * 3 + 0] = i01;
                indexData[triangleIndex * 3 + 1] = i11;
                indexData[triangleIndex * 3 + 2] = i10;
                ++triangleIndex;

                indexData[triangleIndex * 3 + 0] = i01;
                indexData[triangleIndex * 3 + 1] = i10;
                indexData[triangleIndex * 3 + 2] = i00;
                ++triangleIndex;
            }
        }
    }

    // Draw the triangles
    unsigned int triangleCount = rowCount * columnCount * 2;
    if (triangleCount > 0)
    {
        if ((features & Normals) != 0)
        {
            rc.bindVertexArray(VertexSpec::PositionNormalTex, vertexData, vertexStride * 4);
        }
        else
        {
            rc.bindVertexArray(VertexSpec::PositionTex, vertexData, vertexStride * 4);
        }

        rc.drawPrimitives(PrimitiveBatch::Triangles, triangleCount * 3, PrimitiveBatch::Index16, reinterpret_cast<char*>(indexData));
    }
}


// Draw the tile mesh. This method assumes that the vertex arrays have already been set up.
void
QuadtreeTile::drawTriangles(RenderContext& rc) const
{
    if (!ms_indicesInitialized)
    {
        if (!createTileMeshIndices())
        {
            return;
        }
    }

    // Choose the right mesh for this tile based on the tessellation of neighboring tiles
    unsigned int stitchFlags = 0;
    for (unsigned int i = 0; i < 4; ++i)
    {
        if (!m_neighbors[i])
        {
            stitchFlags |= 1 << i;
        }
    }

    rc.drawPrimitives(PrimitiveBatch::Triangles,
                      ms_tileMeshTriangleCounts[stitchFlags] * 3,
                      PrimitiveBatch::Index16,
                      reinterpret_cast<const char*>(ms_tileMeshIndices[stitchFlags]));

}


void
QuadtreeTile::computeCenterAndRadius(const Vector3f& semiAxes)
{
    float tileArc = float(PI) * m_extent;

    float lonWest = float(PI) * m_southwest.x();
    float latSouth = float(PI) * m_southwest.y();
    float lonCenter = lonWest + tileArc * 0.5f;
    float latCenter = latSouth + tileArc * 0.5f;

    // Compute the center of the tile.
    float cosCenterLat = cos(latCenter);
    Vector3f unitCenter(cosCenterLat * cos(lonCenter), cosCenterLat * sin(lonCenter), sin(latCenter));
    m_center = unitCenter.cwise() * semiAxes;

    // Compute the radius of a sphere large enough to contain this tile
    float cornerLat = latSouth >= 0.0f ? latSouth : latSouth + tileArc;
    float cosCornerLat = cos(cornerLat);
    Vector3f corner(cosCornerLat * cos(lonWest), cosCornerLat * sin(lonWest), sin(cornerLat));
    corner = corner.cwise() * semiAxes;
    m_boundingSphereRadius = (corner - m_center).norm();
}


// Initialize vertex indices for all 16 possible tile meshes
bool
QuadtreeTile::createTileMeshIndices()
{
    // Don't reallocate
    if (ms_indicesInitialized)
    {
        return true;
    }

    bool ok = true;

    // Allocate indices for all 16 possible tile meshes
    for (unsigned int i = 0; i < 16; ++i)
    {
        TileTriangulationBuilder builder(TileSubdivision);

        builder.generateTile(i);
        ms_tileMeshTriangleCounts[i] = builder.triangleCount();
        ms_tileMeshIndices[i] = builder.allocateIndexList();
    }

    if (ok)
    {
        ms_indicesInitialized = true;
    }

    return ok;
}
