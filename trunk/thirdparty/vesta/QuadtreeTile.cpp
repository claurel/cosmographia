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

using namespace vesta;
using namespace Eigen;
using namespace std;


static const float SquareSize = 1.0f / float(QuadtreeTile::TileSubdivision);


static VertexAttribute posNormTexTangentAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::Normal,       VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::TextureCoord, VertexAttribute::Float2),
    VertexAttribute(VertexAttribute::Tangent,      VertexAttribute::Float3),
};

static VertexSpec PositionNormalTexTangent(4, posNormTexTangentAttributes);



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
    const unsigned int triangleCount = TileSubdivision * TileSubdivision * 2;

    float vertexData[vertexCount * MaxVertexSize];

    v_uint16 indexData[triangleCount * 3];

    unsigned int vertexIndex = 0;

    float tileArc = float(PI) * m_extent;
    float lonWest = float(PI) * m_southwest.x();
    float latSouth = float(PI) * m_southwest.y();
    float dlon = tileArc / float(TileSubdivision);
    float dlat = tileArc / float(TileSubdivision);
    float du = m_extent / float(TileSubdivision);
    float dv = m_extent / float(TileSubdivision);

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

                float lon = lonWest + j * dlon;
                float u = m_southwest.x() + j * du;

                float cosLon = cos(lon);
                float sinLon = sin(lon);

                Vector3f p(cosLat * cos(lon), cosLat * sin(lon), sinLat);

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

                float lon = lonWest + j * dlon;
                float u = m_southwest.x() + j * du;

                Vector3f p(cosLat * cos(lon), cosLat * sin(lon), sinLat);
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

                float lon = lonWest + j * dlon;
                float u = m_southwest.x() + j * du;

                Vector3f p(cosLat * cos(lon), cosLat * sin(lon), sinLat);
                vertexData[vertexStart + 0] = p.x();
                vertexData[vertexStart + 1] = p.y();
                vertexData[vertexStart + 2] = p.z();
                vertexData[vertexStart + 3] = u * 0.5f + 0.5f;
                vertexData[vertexStart + 4] = 0.5f - v;

                ++vertexIndex;
            }
        }
    }

    unsigned int triangleIndex = 0;
    for (unsigned int i = 0; i < TileSubdivision; ++i)
    {
        for (unsigned int j = 0; j < TileSubdivision; ++j)
        {
            v_uint16 i00 = v_uint16(i * (TileSubdivision + 1) + j);
            v_uint16 i01 = i00 + 1;
            v_uint16 i10 = i00 + v_uint16(TileSubdivision + 1);
            v_uint16 i11 = i10 + 1;

            indexData[triangleIndex * 3 + 0] = i00;
            indexData[triangleIndex * 3 + 1] = i01;
            indexData[triangleIndex * 3 + 2] = i11;
            ++triangleIndex;

            indexData[triangleIndex * 3 + 0] = i00;
            indexData[triangleIndex * 3 + 1] = i11;
            indexData[triangleIndex * 3 + 2] = i10;
            ++triangleIndex;
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
    rc.drawPrimitives(PrimitiveBatch::Triangles, triangleCount * 3, PrimitiveBatch::Index16, reinterpret_cast<char*>(indexData));
}


void
QuadtreeTile::drawPatch(RenderContext& rc, Material& material, TiledMap* baseMap, unsigned int features) const
{
    const unsigned int MaxVertexSize = 11;
    unsigned int vertexStride = 8;

    const unsigned int vertexCount = (TileSubdivision + 1) * (TileSubdivision + 1);
    const unsigned int triangleCount = TileSubdivision * TileSubdivision * 2;

    float vertexData[vertexCount * MaxVertexSize];
    v_uint16 indexData[triangleCount * 3];

    unsigned int vertexIndex = 0;

    float tileArc = float(PI) * m_extent;
    float lonWest = float(PI) * m_southwest.x();
    float latSouth = float(PI) * m_southwest.y();
    float dlon = tileArc / float(TileSubdivision);
    float dlat = tileArc / float(TileSubdivision);

    unsigned int tileSize = baseMap->tileSize();

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

    for (unsigned int i = 0; i <= TileSubdivision; ++i)
    {
        float v = v0 + i * dv;
        float lat = latSouth + i * dlat;
        float cosLat = cos(lat);
        float sinLat = sin(lat);

        for (unsigned int j = 0; j <= TileSubdivision; ++j)
        {
            unsigned int vertexStart = vertexStride * vertexIndex;

            float lon = lonWest + j * dlon;
            float u = u0 + j * du;

            Vector3f p(cosLat * cos(lon), cosLat * sin(lon), sinLat);
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

    unsigned int triangleIndex = 0;
    for (unsigned int i = 0; i < TileSubdivision; ++i)
    {
        for (unsigned int j = 0; j < TileSubdivision; ++j)
        {
            v_uint16 i00 = v_uint16(i * (TileSubdivision + 1) + j);
            v_uint16 i01 = i00 + 1;
            v_uint16 i10 = i00 + v_uint16(TileSubdivision + 1);
            v_uint16 i11 = i10 + 1;

            indexData[triangleIndex * 3 + 0] = i00;
            indexData[triangleIndex * 3 + 1] = i01;
            indexData[triangleIndex * 3 + 2] = i11;
            ++triangleIndex;

            indexData[triangleIndex * 3 + 0] = i00;
            indexData[triangleIndex * 3 + 1] = i11;
            indexData[triangleIndex * 3 + 2] = i10;
            ++triangleIndex;
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
    rc.drawPrimitives(PrimitiveBatch::Triangles, triangleCount * 3, PrimitiveBatch::Index16, reinterpret_cast<char*>(indexData));
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
        float lon = layer.box().west();
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
        float lon = layer.box().east();
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

    float layerLonExtent = layer.box().east() - layer.box().west();
    float layerLatExtent = layer.box().north() - layer.box().south();

    // Compute texture coordinate starting points and steps
    float u0 = (lonWest - layer.box().west()) / layerLonExtent;
    float v0 = (latSouth - layer.box().south()) / layerLatExtent;
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
        startColumn = ceil((layer.box().west() - lonWest) / tileArc * TileSubdivision);
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
            v_uint16 i00 = v_uint16(i * (columnCount + 1) + j);
            v_uint16 i01 = i00 + 1;
            v_uint16 i10 = i00 + v_uint16(columnCount + 1);
            v_uint16 i11 = i10 + 1;

            indexData[triangleIndex * 3 + 0] = i00;
            indexData[triangleIndex * 3 + 1] = i01;
            indexData[triangleIndex * 3 + 2] = i11;
            ++triangleIndex;

            indexData[triangleIndex * 3 + 0] = i00;
            indexData[triangleIndex * 3 + 1] = i11;
            indexData[triangleIndex * 3 + 2] = i10;
            ++triangleIndex;
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
