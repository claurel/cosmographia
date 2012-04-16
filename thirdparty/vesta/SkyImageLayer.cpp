/*
 * $Revision: 418 $ $Date: 2010-08-10 09:07:36 -0700 (Tue, 10 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "SkyImageLayer.h"
#include "RenderContext.h"
#include "Material.h"
#include "QuadtreeTile.h"
#include "Debug.h"
#include <Eigen/LU>
#include <cmath>

using namespace vesta;
using namespace Eigen;

static const float MaxSkyImageTileSquareSize = 256.0f;  // size in pixels


SkyImageLayer::SkyImageLayer() :
    m_orientation(Quaterniond::Identity()),
    m_opacity(1.0f),
    m_tintColor(1.0f, 1.0f, 1.0f),
    m_tileAllocator(NULL)
{
    m_tileAllocator = new QuadtreeTileAllocator;
}


SkyImageLayer::~SkyImageLayer()
{
    delete m_tileAllocator;
}


void
SkyImageLayer::setTexture(TextureMap* texture)
{
    m_texture = texture;
}


void
SkyImageLayer::render(RenderContext& rc)
{
    // Don't render anything if the sky texture isn't resident
    if (m_texture.isNull() || m_texture->makeResident() == false)
    {
        return;
    }

    rc.pushModelView();
    rc.rotateModelView(m_orientation.cast<float>());

    // Get the position of the eye in model coordinates *before* scaling
    Transform3f invModelView = Transform3f(rc.modelview().inverse());
    Vector3f eyePosition = invModelView * Vector3f::Zero();

    // Compute the culling planes. Use the horizon distance for the far plane in order
    // to cull as many surface patches as possible.
    Frustum viewFrustum = rc.frustum();
    float farDistance = 2.0e6f;
    Matrix4f modelviewTranspose = rc.modelview().matrix().transpose();
    CullingPlaneSet cullingPlanes;
    for (unsigned int i = 0; i < 4; ++i)
    {
        cullingPlanes.planes[i] = Hyperplane<float, 3>(viewFrustum.planeNormals[i].cast<float>(), 0.0f);
        cullingPlanes.planes[i].coeffs() = modelviewTranspose * cullingPlanes.planes[i].coeffs();
    }
    cullingPlanes.planes[4].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f, -1.0f, -viewFrustum.nearZ);
    cullingPlanes.planes[5].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f,  1.0f, farDistance);

    // Set the vertex information directly; we should change this so that
    // we're using a vertex array instead of immediate mode rendering.
    // Switch to unlit rendering by disabling surface normals
    // required for lighting.
    unsigned int tileFeatures = 0;

    Material material;
    material.setDiffuse(m_tintColor);
    material.setEmission(Spectrum::Black());
    material.setOpacity(m_opacity);
    material.setBaseTexture(m_texture.ptr());
    rc.bindMaterial(&material);

    // Create the root quadtree nodes. Presently, we always start with two root
    // tiles: one for the western hemisphere and one for the eastern hemisphere.
    // But, depending on what sort of tiles we have, a different set of root
    // tiles might be more appropriate.
    Vector3f semiAxes = Vector3f::Constant(1.0f);

    m_tileAllocator->clear();
    QuadtreeTile* westHemi = m_tileAllocator->newRootTile(0, 0, Vector2f(-1.0f, -0.5f), 1.0f, semiAxes);
    QuadtreeTile* eastHemi = m_tileAllocator->newRootTile(0, 1, Vector2f( 0.0f, -0.5f), 1.0f, semiAxes);

    // Set up the neighbor connections for the root nodes. Since the map wraps,
    // the eastern hemisphere is both the east and west neighbor of the western
    // hemisphere (and vice versa.) There are no north and south neighbors.
    westHemi->setNeighbor(QuadtreeTile::West, eastHemi);
    westHemi->setNeighbor(QuadtreeTile::East, eastHemi);
    eastHemi->setNeighbor(QuadtreeTile::West, westHemi);
    eastHemi->setNeighbor(QuadtreeTile::East, westHemi);

    // TODO: Consider map tile resolution when setting the split threshold
    float splitThreshold = rc.pixelSize() * MaxSkyImageTileSquareSize * QuadtreeTile::TileSubdivision;
    westHemi->tessellate(eyePosition, cullingPlanes, semiAxes, splitThreshold, rc.pixelSize());
    eastHemi->tessellate(eyePosition, cullingPlanes, semiAxes, splitThreshold, rc.pixelSize());

    glCullFace(GL_FRONT);
    westHemi->render(rc, tileFeatures);
    eastHemi->render(rc, tileFeatures);
    glCullFace(GL_BACK);

    rc.popModelView();
}
