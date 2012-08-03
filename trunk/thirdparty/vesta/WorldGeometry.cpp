/*
 * $Revision: 685 $ $Date: 2012-08-03 12:54:39 -0700 (Fri, 03 Aug 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "WorldGeometry.h"
#include "RenderContext.h"
#include "Units.h"
#include "TextureMap.h"
#include "TiledMap.h"
#include "OGLHeaders.h"
#include "ShaderBuilder.h"
#include "Atmosphere.h"
#include "PlanetaryRings.h"
//#include "VertexBuffer.h"
#include "QuadtreeTile.h"
#include "WorldLayer.h"
#include "Intersect.h"
#include "Debug.h"
#include <Eigen/LU>
#include <algorithm>
#include <deque>

using namespace vesta;
using namespace Eigen;
using namespace std;


#define DEBUG_QUADTREE 0


static VertexAttribute posNormTexTangentAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::Normal,       VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::TextureCoord, VertexAttribute::Float2),
    VertexAttribute(VertexAttribute::Tangent,      VertexAttribute::Float3),
};

static VertexSpec PositionNormalTexTangent(4, posNormTexTangentAttributes);

static const float MaxTileSquareSize = 256.0f;  // size in pixels

bool WorldGeometry::ms_atmospheresVisible = true;
bool WorldGeometry::ms_cloudLayersVisible = true;


WorldGeometry::WorldGeometry() :
    m_emissive(false),
    m_specularReflectance(Spectrum(0.0f, 0.0f, 0.0f)),
    m_specularPower(20.0f),
    m_cloudAltitude(0.0f),
    m_tileAllocator(NULL)
{
    setClippingPolicy(Geometry::PreventClipping);
    setShadowCaster(true);

    m_material = new Material();
    m_material->setDiffuse(Spectrum(1.0f, 1.0f, 1.0f));

    m_tileAllocator = new QuadtreeTileAllocator;
}


WorldGeometry::~WorldGeometry()
{
    delete m_tileAllocator;
}


// Calculate the horizon distance; we'll just approximate this for non-spherical
// bodies, using an estimate that will always be greater than or equal to the
// actual horizon distance (thus ensuring that we don't clip anything that should
// be visible.
static float
HorizonDistance(const Vector3f& eyePosition, const Vector3f& ellipsoidAxes)
{
    float approxAltitude = eyePosition.norm() - ellipsoidAxes.minCoeff() * 0.5f;
    float horizonDistance = 0.0f;
    if (approxAltitude > 0.0f)
    {
        float r = ellipsoidAxes.maxCoeff() * 0.5f;
        horizonDistance = sqrt((2 * r + approxAltitude) * approxAltitude);
    }

    return horizonDistance;
}


// Calculate the distance between an observer and the most distant point of the
// atmosphere geometry that will be visible (i.e. not below the horizon)
static float
CloudShellDistance(const Vector3f& eyePosition, const Vector3f& ellipsoidAxes, float cloudHeight)
{
    float planetRadius = ellipsoidAxes.maxCoeff() * 0.5f;
    float shellRadius = planetRadius + cloudHeight;
    float cloudScale = shellRadius / planetRadius;

    Vector3f shellEllipsoidAxes = ellipsoidAxes * cloudScale;

    // Altitude above the planet
    float approxAltitudePlanet = eyePosition.norm() - ellipsoidAxes.minCoeff() * 0.5f;
    float approxAltitudeCloud = eyePosition.norm() - shellEllipsoidAxes.minCoeff() * 0.5f;

    float maxCloudDistance = 0.0f;
    if (approxAltitudeCloud > 0.0f)
    {
        // Observer is above cloud layer
        float r = shellEllipsoidAxes.maxCoeff() * 0.5f;
        maxCloudDistance = sqrt((2 * r + approxAltitudeCloud) * approxAltitudeCloud);
    }
    else if (approxAltitudePlanet > 0.0f)
    {
        // Observer is in between planet surface and cloud layer
        float r = ellipsoidAxes.maxCoeff() * 0.5f;
        float horizonDistance = sqrt((2 * r + approxAltitudePlanet) * approxAltitudePlanet);
        maxCloudDistance = horizonDistance + sqrt(max(0.0f, shellRadius * shellRadius - planetRadius * planetRadius));
    }
    else
    {
        // Observer is inside the planet--hide the clouds
        maxCloudDistance = 0.0f;
    }

    return maxCloudDistance;
}


// Calculate the distance between an observer and the most distant point of the
// atmosphere geometry that will be visible (i.e. not below the horizon)
static void
AtmosphereShellDistance(const Vector3f& eyePosition, const Vector3f& ellipsoidAxes, float atmHeight,
                        float *minAtmDistance, float* maxAtmDistance)
{
    float planetRadius = ellipsoidAxes.maxCoeff() * 0.5f;
    float shellRadius = planetRadius + atmHeight;
    float atmScale = shellRadius / planetRadius;

    Vector3f shellEllipsoidAxes = ellipsoidAxes * atmScale;

    // Altitude above the planet
    float approxAltitudePlanet = eyePosition.norm() - ellipsoidAxes.minCoeff() * 0.5f;
    float approxAltitudeShell = eyePosition.norm() - shellEllipsoidAxes.minCoeff() * 0.5f;

    if (approxAltitudePlanet > 0.0f)
    {
        // Observer is above the planet
        float r = ellipsoidAxes.maxCoeff() * 0.5f;
        float horizonDistance = sqrt((2 * r + approxAltitudePlanet) * approxAltitudePlanet);
        *maxAtmDistance = horizonDistance + sqrt(max(0.0f, shellRadius * shellRadius - planetRadius * planetRadius));
    }
    else
    {
        // Observer is inside the planet--hide the atmosphere
        *maxAtmDistance = 0.0f;
    }

    // Since we're rendering just the back of the atmosphere shell, we can cull the front
    // patches. If the observer is outside the atmosphere, the near plane distance can be
    // set to the horizon distance for the atmosphere shell.
    if (approxAltitudeShell > 0.0f && 0)
    {
        float r = shellEllipsoidAxes.maxCoeff() * 0.5f;
        *minAtmDistance = sqrt((2 * r + approxAltitudeShell) * approxAltitudeShell);
    }
    else
    {
        *minAtmDistance = 0.0f;
    }
}


void
WorldGeometry::render(RenderContext& rc, double clock) const
{
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        if (m_ringSystem.isValid())
        {
            m_ringSystem->render(rc, clock);
        }
        return;
    }

    // Determine the level of detail
    float radius = maxRadius();

    // Get the position of the eye in model coordinates *before* scaling
    Transform3f invModelView = Transform3f(rc.modelview().inverse());
    Vector3f eyePosition = invModelView * Vector3f::Zero();

    // Calculate the horizon distance; we'll just approximate this for non-spherical
    // bodies, using an estimate that will always be greater than or equal to the
    // actual horizon distance (thus ensuring that we don't clip anything that should
    // be visible.
    float approxAltitude = eyePosition.norm() - m_ellipsoidAxes.minCoeff() * 0.5f;
    float horizonDistance = 0.0f;
    if (approxAltitude > 0.0f)
    {
        float r = m_ellipsoidAxes.maxCoeff() * 0.5f;
        horizonDistance = sqrt((2 * r + approxAltitude) * approxAltitude);
    }

    // Compute the culling planes. Use the horizon distance for the far plane in order
    // to cull as many surface patches as possible.
    Frustum viewFrustum = rc.frustum();
    float farDistance = max(viewFrustum.nearZ, min(horizonDistance, viewFrustum.farZ));
    Matrix4f modelviewTranspose = rc.modelview().matrix().transpose();
    CullingPlaneSet cullingPlanes;
    for (unsigned int i = 0; i < 4; ++i)
    {
        cullingPlanes.planes[i] = Hyperplane<float, 3>(viewFrustum.planeNormals[i].cast<float>(), 0.0f);
        cullingPlanes.planes[i].coeffs() = modelviewTranspose * cullingPlanes.planes[i].coeffs();
    }
    cullingPlanes.planes[4].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f, -1.0f, -viewFrustum.nearZ);
    cullingPlanes.planes[5].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f,  1.0f, farDistance);

    rc.pushModelView();
    rc.scaleModelView(m_ellipsoidAxes * 0.5f);

    // Enable normal maps when a normal texture has been set and the
    // render context supports shaders.
    bool useNormalTexture = false;
    if (!m_normalMap.isNull() && rc.shaderCapability() != RenderContext::FixedFunction)
    {
        useNormalTexture = true;
    }

    unsigned int tileFeatures = 0;

    // Set the vertex information directly; we should change this so that
    // we're using a vertex array instead of immediate mode rendering.
    if (m_emissive)
    {
        // Switch to unlit rendering by disabling surface normals
        // required for lighting.
        rc.setVertexInfo(VertexSpec::PositionTex);
    }
    else if (useNormalTexture)
    {
        rc.setVertexInfo(PositionNormalTexTangent);
        tileFeatures |= QuadtreeTile::NormalMap | QuadtreeTile::Normals;
    }
    else
    {
        rc.setVertexInfo(VertexSpec::PositionNormalTex);
        tileFeatures |= QuadtreeTile::Normals;
    }

    RenderContext::ScatteringParameters scatteringParams;

    float atmosphereHeight = 0.0f;

    if (!m_atmosphere.isNull() && ms_atmospheresVisible)
    {
        float r = maxRadius();

        atmosphereHeight = m_atmosphere->transparentHeight() / r;

        scatteringParams.planetRadius = 1.0f;
        scatteringParams.atmosphereRadius = 1.0f + atmosphereHeight;
        scatteringParams.rayleighScaleHeight = m_atmosphere->rayleighScaleHeight() / r;
        scatteringParams.rayleighCoeff = m_atmosphere->rayleighScatteringCoeff() * r * 1000.0f;
        scatteringParams.color = m_atmosphere->color(100000.0f);
        scatteringParams.mieAsymmetry = m_atmosphere->mieAsymmetry();
        scatteringParams.transmittanceTexture = m_atmosphere->transmittanceTexture();
        scatteringParams.scatterTexture = m_atmosphere->scatterTexture();
        rc.setScatteringParameters(scatteringParams);
        rc.setScattering(true);
    }

    rc.setSphericalGeometryHint(true);

    Material material = *m_material;
    material.setBaseTexture(m_baseMap.ptr());
    material.setSpecular(m_specularReflectance);
    material.setPhongExponent(m_specularPower);
    material.setSpecularModifier(Material::DiffuseTextureAlpha);

    if (useNormalTexture)
    {
        material.setNormalTexture(m_normalMap.ptr());
        rc.bindMaterial(&material);
    }
    else
    {
        rc.bindMaterial(&material);
    }

    // Create the root quadtree nodes. Presently, we always start with two root
    // tiles: one for the western hemisphere and one for the eastern hemisphere.
    // But, depending on what sort of tiles we have, a different set of root
    // tiles might be more appropriate.
    Vector3f semiAxes = m_ellipsoidAxes * 0.5f;

    QuadtreeTile* westHemi = NULL;
    QuadtreeTile* eastHemi = NULL;
    initQuadtree(semiAxes, &westHemi, &eastHemi);

    float splitThreshold = rc.pixelSize() * MaxTileSquareSize * QuadtreeTile::TileSubdivision;
    if (m_baseTiledMap.isValid())
    {
        // Adjust split threshold based on tile size
        //   - 0 is a special case indicating that the tile size shouldn't be used
        //     to determine tessellation
        //   - Prevent huge numbers of tiles from being generated if the tiled map
        //     reports a very small tile size.
        unsigned int tileSize = m_baseTiledMap->tileSize();
        if (tileSize != 0 && tileSize < 1000)
        {
            splitThreshold *= float(max(128u, tileSize)) / 1000.0f;
        }
    }

    westHemi->tessellate(eyePosition, cullingPlanes, semiAxes, splitThreshold, rc.pixelSize());
    eastHemi->tessellate(eyePosition, cullingPlanes, semiAxes, splitThreshold, rc.pixelSize());

    if (m_baseTiledMap.isNull())
    {
        westHemi->render(rc, tileFeatures);
        eastHemi->render(rc, tileFeatures);
    }
    else if (m_tiledNormalMap.isNull())
    {
        // Tiled base map, but no tiled normal map
        westHemi->render(rc, material, m_baseTiledMap.ptr(), QuadtreeTile::Normals);
        eastHemi->render(rc, material, m_baseTiledMap.ptr(), QuadtreeTile::Normals);
    }
    else
    {
        // We have tiled base and tiled normal maps
        westHemi->render(rc, material, m_baseTiledMap.ptr(), m_tiledNormalMap.ptr());
        eastHemi->render(rc, material, m_baseTiledMap.ptr(), m_tiledNormalMap.ptr());
    }

    // TODO: replace this with more general WorldLayers mechanism
    if (!m_mapLayers.empty())
    {
        // Enable polygon offset to ensure that layers are rendered on top of the
        // base planet geometry.
        // TODO: Investigate replacing this with a projection matrix adjustment;
        // glPolygonOffset can interfere with the performance of GPUs that have
        // hiearchical z-buffer optimizations.
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-3.0f, 0.0f);

        // Add a scale factor to prevent depth buffer artifacts. The scale factor
        // is dependent on the projected size of the planet sphere.
        rc.pushModelView();

        Material simpleMaterial;
        simpleMaterial.setDiffuse(Spectrum(1.0f, 1.0f, 1.0f));
        for (unsigned int layerIndex = 0; layerIndex < m_mapLayers.size(); ++layerIndex)
        {
            const MapLayer* layer = m_mapLayers[layerIndex].ptr();
            if (layer && layer->opacity() > 0.0f)
            {
                TextureMap* texture = layer->texture();
                if (texture)
                {
                    simpleMaterial.setOpacity(layer->opacity());
                    simpleMaterial.setBaseTexture(texture);
                    rc.bindMaterial(&simpleMaterial);

                    westHemi->render(rc, *layer, QuadtreeTile::Normals);
                    eastHemi->render(rc, *layer, QuadtreeTile::Normals);
                }
            }
        }
        rc.popModelView();

        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    if (hasLayers())
    {
        // Enable polygon offset to ensure that layers are rendered on top of the
        // base planet geometry.
        // TODO: Investigate replacing this with a projection matrix adjustment;
        // glPolygonOffset can interfere with the performance of GPUs that have
        // hiearchical z-buffer optimizations.
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-3.0f, 0.0f);

        for (WorldLayerTable::const_iterator iter = m_layers.begin(); iter != m_layers.end(); ++iter)
        {
            const WorldLayer* layer = iter->second.ptr();
            if (layer && layer->isVisible())
            {
                westHemi->renderWorldLayer(rc, this, layer);
                eastHemi->renderWorldLayer(rc, this, layer);
            }
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // Set vertex info for cloud layer rendering
    rc.setVertexInfo(VertexSpec::PositionNormalTex);

    if ((m_cloudMap.isValid() || m_tiledCloudMap.isValid()) && ms_cloudLayersVisible)
    {
        float scale = 1.0f + m_cloudAltitude / maxRadius();

        rc.pushModelView();
        rc.scaleModelView(Vector3f::Constant(scale));

        Material cloudMaterial;
        cloudMaterial.setOpacity(1.0f);
        cloudMaterial.setBlendMode(Material::AlphaBlend);
        cloudMaterial.setDiffuse(Spectrum(1.0f, 1.0f, 1.0f));

        // Draw the inside of the cloud layer if the viewer is below the clouds.
        // Instead of the actual viewer height above the surface, use the distance
        // along a line through the center of the planet ellipsoid. This is correct
        // since the cloud geometry is drawn as a scaled ellipsoid rather than at
        // constant height above the planet.
        float ellipDistance = (rc.modelTranslation().cast<float>().cwise() * (m_ellipsoidAxes * (0.5f / radius))).norm();
        if (ellipDistance < m_cloudAltitude + radius)
        {
            glCullFace(GL_FRONT);
        }

        Vector3f cloudSemiAxes = m_ellipsoidAxes * 0.5f * scale;
        
        QuadtreeTile* westHemi = NULL;
        QuadtreeTile* eastHemi = NULL;
        initQuadtree(cloudSemiAxes, &westHemi, &eastHemi);
        
        // Adjust the distance of the far plane.
        float maxCloudDistance = CloudShellDistance(eyePosition, m_ellipsoidAxes, m_cloudAltitude);
        farDistance = max(viewFrustum.nearZ, min(maxCloudDistance, viewFrustum.farZ));
        cullingPlanes.planes[5].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f,  1.0f, farDistance);
        
        float splitThreshold = rc.pixelSize() * MaxTileSquareSize * QuadtreeTile::TileSubdivision;
        westHemi->tessellate(eyePosition, cullingPlanes, cloudSemiAxes, splitThreshold, rc.pixelSize());
        eastHemi->tessellate(eyePosition, cullingPlanes, cloudSemiAxes, splitThreshold, rc.pixelSize());
        
        // Only draw the cloud layer if the cloud texture is resident; otherwise, the cloud
        // layer is drawn as an opaque shell until texture loading is complete.
        if (m_tiledCloudMap.isValid())
        {
            westHemi->render(rc, cloudMaterial, m_tiledCloudMap.ptr(), QuadtreeTile::Normals);
            eastHemi->render(rc, cloudMaterial, m_tiledCloudMap.ptr(), QuadtreeTile::Normals);
        }
        else if (m_cloudMap.isValid())
        {
            m_cloudMap->makeResident();
            if (m_cloudMap->isResident())
            {
                cloudMaterial.setBaseTexture(m_cloudMap.ptr());
                rc.bindMaterial(&cloudMaterial);

                westHemi->render(rc, QuadtreeTile::Normals);
                eastHemi->render(rc, QuadtreeTile::Normals);
            }
        }
        glCullFace(GL_BACK);

        rc.popModelView();
    }

    // Draw the atmosphere as a pixel shaded 'shell'
    if (!m_atmosphere.isNull() && ms_atmospheresVisible)
    {
        // Scale the scattering parameters as well as the geometry
        float scale = 1.0f + atmosphereHeight;
        scatteringParams.planetRadius /= scale;
        scatteringParams.atmosphereRadius /= scale;
        scatteringParams.rayleighScaleHeight /= scale;
        scatteringParams.rayleighCoeff *= scale;
        rc.setScatteringParameters(scatteringParams);

        glCullFace(GL_FRONT);
        rc.pushModelView();
        rc.scaleModelView(Vector3f::Constant(scale));

        // Atmosphere rendering benefits greatly from sRGB gamma correction; enable this
        // setting eventually:
        // if (GLEW_EXT_framebuffer_sRGB)
        //     glEnable(GL_FRAMEBUFFER_SRGB_EXT);
        Material atmosphereMaterial;
        atmosphereMaterial.setOpacity(0.0f);
        atmosphereMaterial.setBlendMode(Material::PremultipliedAlphaBlend);
        rc.bindMaterial(&atmosphereMaterial);

        Vector3f atmSemiAxes = m_ellipsoidAxes * 0.5f * scale;

        QuadtreeTile* westHemi = NULL;
        QuadtreeTile* eastHemi = NULL;
        initQuadtree(atmSemiAxes, &westHemi, &eastHemi);

        // Adjust the distance of the near and far planes so that as much of the atmosphere
        // shell geometry as possible is culled.
        float maxAtmosphereDistance = 0.0f;
        float minAtmosphereDistance = 0.0f;
        AtmosphereShellDistance(eyePosition, m_ellipsoidAxes, m_atmosphere->transparentHeight(),
                                &minAtmosphereDistance, &maxAtmosphereDistance);
        farDistance = max(viewFrustum.nearZ, min(maxAtmosphereDistance, viewFrustum.farZ));
        cullingPlanes.planes[5].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f,  1.0f, farDistance);
        float nearDistance = max(viewFrustum.nearZ, min(minAtmosphereDistance, viewFrustum.nearZ));
        cullingPlanes.planes[4].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f, -1.0f, -nearDistance);

        float splitThreshold = rc.pixelSize() * MaxTileSquareSize * QuadtreeTile::TileSubdivision * 2;
        westHemi->tessellate(eyePosition, cullingPlanes, atmSemiAxes, splitThreshold, rc.pixelSize());
        eastHemi->tessellate(eyePosition, cullingPlanes, atmSemiAxes, splitThreshold, rc.pixelSize());

        westHemi->render(rc, QuadtreeTile::Normals);
        eastHemi->render(rc, QuadtreeTile::Normals);

        rc.popModelView();
        glCullFace(GL_BACK);
        // if (GLEW_EXT_framebuffer_sRGB)
        //     glDisable(GL_FRAMEBUFFER_SRGB_EXT);

        rc.setScattering(false);
    }

    rc.setSphericalGeometryHint(false);

#if DEBUG_QUADTREE
    rc.setVertexInfo(VertexSpec::Position);
    rc.pushModelView();

    Material highlight;
    highlight.setDiffuse(Spectrum(0.0f, 1.0f, 0.0f));
    highlight.setOpacity(0.3f);
    rc.bindMaterial(&highlight);

    rc.setModelView(Matrix4f::Identity());
    rc.pushProjection();
    rc.orthographicProjection2D(-1.0f, 1.0f, -1.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);

    float minExtent = 1.0f;
    for (QuadtreeTileAllocator::TileArray::const_iterator iter = m_tileAllocator->tiles().begin();
         iter != m_tileAllocator->tiles().end(); ++iter)
    {
        if (!iter->hasChildren() && !iter->isCulled())
        {
            glBegin(GL_LINE_LOOP);

            Vector3f p0(iter->southwest().x(), iter->southwest().y(), 0.0f);
            Vector3f p1(iter->southwest().x() + iter->extent(), iter->southwest().y(), 0.0f);
            Vector3f p2(iter->southwest().x() + iter->extent(), iter->southwest().y() + iter->extent(), 0.0f);
            Vector3f p3(iter->southwest().x(), iter->southwest().y() + iter->extent(), 0.0f);

            glVertex3fv(p0.data());
            glVertex3fv(p1.data());
            glVertex3fv(p2.data());
            glVertex3fv(p3.data());

            glEnd();

            minExtent = min(minExtent, iter->extent());
        }
    }

    glEnable(GL_DEPTH_TEST);
    rc.popProjection();

    rc.popModelView();
#endif // DEBUG_QUADTREE

    rc.popModelView();
}


void
WorldGeometry::initQuadtree(const Vector3f& semiAxes, QuadtreeTile **westHemi, QuadtreeTile **eastHemi) const
{
    m_tileAllocator->clear();
    *westHemi = m_tileAllocator->newRootTile(0, 0, Vector2f(-1.0f, -0.5f), 1.0f, semiAxes);
    *eastHemi = m_tileAllocator->newRootTile(0, 1, Vector2f( 0.0f, -0.5f), 1.0f, semiAxes);

    // Set up the neighbor connections for the root nodes. Since the map wraps,
    // the eastern hemisphere is both the east and west neighbor of the western
    // hemisphere (and vice versa.) There are no north and south neighbors.
    (*westHemi)->setNeighbor(QuadtreeTile::West, *eastHemi);
    (*westHemi)->setNeighbor(QuadtreeTile::East, *eastHemi);
    (*eastHemi)->setNeighbor(QuadtreeTile::West, *westHemi);
    (*eastHemi)->setNeighbor(QuadtreeTile::East, *westHemi);
}


#if USE_VBO
class VertexWriter
{
public:
    VertexWriter(RenderContext& rc, VertexBuffer* vb, const VertexSpec& spec, PrimitiveBatch::PrimitiveType primType) :
        m_rc(rc),
        m_vb(vb),
        m_spec(spec),
        m_primType(primType),
        m_vbSize(vb->size() / spec.size()),
        m_vertexSizeInFloats(spec.size() / sizeof(float)),
        m_count(0),
        m_limit(m_vbSize - 4),
        m_buffer(NULL),
        m_isMapped(false)
    {
        m_buffer = reinterpret_cast<float*>(vb->mapWriteOnly());
        m_isMapped = true;
    }

    ~VertexWriter()
    {
        checkFlush();
        if (m_isMapped)
        {
            m_vb->unmap();
        }
    }

    inline void vertex(const Vector3f& position, const Vector3f& normal, const Vector2f& texCoord)
    {
        unsigned int offset = m_count * m_vertexSizeInFloats;
        m_buffer[offset + 0] = position.x();
        m_buffer[offset + 1] = position.y();
        m_buffer[offset + 2] = position.z();
        m_buffer[offset + 3] = normal.x();
        m_buffer[offset + 4] = normal.y();
        m_buffer[offset + 5] = normal.z();
        m_buffer[offset + 6] = texCoord.x();
        m_buffer[offset + 7] = texCoord.y();

        ++m_count;
        if (m_count >= m_limit)
        {
            checkFlush();
        }
    }

    void checkFlush()
    {
        bool done = false;
        unsigned int primCount = 0;

        switch (m_primType)
        {
        case PrimitiveBatch::TriangleFan:
        case PrimitiveBatch::TriangleStrip:
            done = m_count > 2;
            primCount = m_count - 2;
            break;
         case PrimitiveBatch::LineStrip:
            done = m_count > 1;
            primCount = m_count - 1;
            break;
         case PrimitiveBatch::Triangles:
            done = m_count % 3 == 0;
            primCount = m_count / 3;
            break;
         case PrimitiveBatch::Lines:
            done = m_count & 0x1 == 0;
            primCount = m_count / 2;
            break;
         default:
            primCount = m_count;
            done = true;
        }

        if (done)
        {
            flush(primCount);
            m_buffer = reinterpret_cast<float*>(m_vb->mapWriteOnly());
            m_isMapped = true;
        }
    }

    void flush(unsigned int primCount)
    {
        if (m_vb->unmap())
        {
            if (primCount != 0)
            {
                m_rc.bindVertexBuffer(m_spec, m_vb, m_spec.size());
                m_rc.drawPrimitives(PrimitiveBatch(m_primType, primCount));
            }
        }
        m_isMapped = false;
        m_count = 0;
    }

private:
    RenderContext& m_rc;
    VertexBuffer* m_vb;
    const VertexSpec& m_spec;
    PrimitiveBatch::PrimitiveType m_primType;

    unsigned int m_vbSize;
    unsigned int m_vertexSizeInFloats;
    unsigned int m_count;
    unsigned int m_limit;
    float* m_buffer;

    bool m_isMapped;
};
#endif


#if RENDER_SPHERE_AS_PATCHES
static v_uint16* PatchIndices = NULL;
static void renderPatch(RenderContext& rc, const Vector2f& southwest, float extent, unsigned int features)
{
    const unsigned int MaxVertexSize = 11;
    unsigned int vertexStride = 8;
    if ((features & 0x1) != 0)
    {
        vertexStride = 11;
    }

    const unsigned int vertexCount = (TileSubdivision + 1) * (TileSubdivision + 1);
    const unsigned int triangleCount = TileSubdivision * TileSubdivision * 2;

    float vertexData[vertexCount * MaxVertexSize];
    //VertexBuffer* vb = rc.vertexStreamBuffer();
    //float* vertexData = reinterpret_cast<float*>(vb->mapWriteOnly(true));

    unsigned int vertexIndex = 0;

    float tileArc = float(PI) * extent;
    float lonWest = float(PI) * southwest.x();
    float latSouth = float(PI) * southwest.y();
    float dlon = tileArc / float(TileSubdivision);
    float dlat = tileArc / float(TileSubdivision);
    float du = extent / float(TileSubdivision);
    float dv = extent / float(TileSubdivision);


    for (unsigned int i = 0; i <= TileSubdivision; ++i)
    {
        float v = southwest.y() + i * dv;
        float lat = latSouth + i * dlat;
        float cosLat = cos(lat);
        float sinLat = sin(lat);

        if ((features & 0x1) != 0)
        {
            for (unsigned int j = 0; j <= TileSubdivision; ++j)
            {
                unsigned int vertexStart = vertexStride * vertexIndex;

                float lon = lonWest + j * dlon;
                float u = southwest.x() + j * du;

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
        else
        {
            for (unsigned int j = 0; j <= TileSubdivision; ++j)
            {
                unsigned int vertexStart = vertexStride * vertexIndex;

                float lon = lonWest + j * dlon;
                float u = southwest.x() + j * du;

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
    }


    unsigned int triangleIndex = 0;

    if (PatchIndices == NULL)
    {
        PatchIndices = new v_uint16[triangleCount * 3];

        for (unsigned int i = 0; i < TileSubdivision; ++i)
        {
            for (unsigned int j = 0; j < TileSubdivision; ++j)
            {
                v_uint16 i00 = v_uint16(i * (TileSubdivision + 1) + j);
                v_uint16 i01 = i00 + 1;
                v_uint16 i10 = i00 + v_uint16(TileSubdivision + 1);
                v_uint16 i11 = i10 + 1;

                PatchIndices[triangleIndex * 3 + 0] = i00;
                PatchIndices[triangleIndex * 3 + 1] = i01;
                PatchIndices[triangleIndex * 3 + 2] = i11;
                ++triangleIndex;

                PatchIndices[triangleIndex * 3 + 0] = i00;
                PatchIndices[triangleIndex * 3 + 1] = i11;
                PatchIndices[triangleIndex * 3 + 2] = i10;
                ++triangleIndex;
            }
        }
    }

    //vb->unmap();

    if ((features & 0x1) != 0)
    {
        //rc.bindVertexBuffer(PositionNormalTexTangent, vb, vertexStride * 4);
        rc.bindVertexArray(PositionNormalTexTangent, vertexData, vertexStride * 4);
    }
    else
    {
        //rc.bindVertexBuffer(VertexSpec::PositionNormalTex, vb, vertexStride * 4);
        rc.bindVertexArray(VertexSpec::PositionNormalTex, vertexData, vertexStride * 4);
    }

    glDrawElements(GL_TRIANGLES, triangleCount * 3, GL_UNSIGNED_SHORT, PatchIndices);
}


void
WorldGeometry::renderSphere(RenderContext& rc, int subdivisions) const
{
    unsigned int latPatchCount = (subdivisions * 2 + TileSubdivision - 1) / TileSubdivision;
    unsigned int lonPatchCount = 2 * latPatchCount;

    float patchExtent = 1.0f / float(latPatchCount);
    for (unsigned int latPatch = 0; latPatch < latPatchCount; ++latPatch)
    {
        for (unsigned int lonPatch = 0; lonPatch < lonPatchCount; ++lonPatch)
        {
            ::renderPatch(rc, Vector2f(-1.0f + lonPatch * patchExtent, -0.5f + latPatch * patchExtent), patchExtent, 0);
        }
    }
}
#endif


void
WorldGeometry::renderSphere(RenderContext& /* rc */, int subdivisions) const
{
    // Not available without immediate mode 3D
#ifndef VESTA_NO_IMMEDIATE_MODE_3D
    //VertexBuffer* vb = rc.vertexStreamBuffer();

    float lastSinPhi = -1.0f;
    float lastCosPhi = 0.0f;
    float lastTexT = 1.0f;

#if 0
    VertexWriter writer(rc, vb, VertexSpec::PositionNormalTex, PrimitiveBatch::TriangleStrip);

    // Render latitudinal bands
    for (int band = -subdivisions + 1; band <= subdivisions; band++)
    {
        float t = (float) (band) / (float) subdivisions;
        float phi = t * (float) PI / 2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        float texT = (1.0f - t) * 0.5f;

        for (int slice = 0; slice < subdivisions * 4; slice++)
        {
            float u = (float) slice / (float) (subdivisions * 4);
            float theta = u * 2.0f * (float) PI;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            float texS = u;

            Vector3f v0(lastCosPhi * cosTheta, lastCosPhi * sinTheta, lastSinPhi);
            Vector3f v1(cosPhi * cosTheta,     cosPhi * sinTheta, sinPhi);

            writer.vertex(v1, v1, Vector2f(texS, texT));
            writer.vertex(v0, v0, Vector2f(texS, lastTexT));
        }

        Vector3f v0(lastCosPhi, 0.0f, lastSinPhi);
        Vector3f v1(cosPhi,     0.0f, sinPhi);

        writer.vertex(v1, v1, Vector2f(1.0f, texT));
        writer.vertex(v0, v0, Vector2f(1.0f, lastTexT));

        lastSinPhi = sinPhi;
        lastCosPhi = cosPhi;
        lastTexT = texT;
    }
#else
    for (int band = -subdivisions + 1; band <= subdivisions; band++)
    {
        float t = (float) (band) / (float) subdivisions;
        float phi = t * (float) PI / 2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        float texT = (1.0f - t) * 0.5f;

        glBegin(GL_QUAD_STRIP);

        for (int slice = 0; slice < subdivisions * 4; slice++)
        {
            float u = (float) slice / (float) (subdivisions * 4);
            float theta = u * 2.0f * (float) PI;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            float texS = u;

            Vector3f v0(lastCosPhi * cosTheta, lastCosPhi * sinTheta, lastSinPhi);
            Vector3f v1(cosPhi * cosTheta,     cosPhi * sinTheta, sinPhi);

            glTexCoord2f(texS, texT);
            glNormal3fv(v1.data());
            glVertex3fv(v1.data());
            glTexCoord2f(texS, lastTexT);
            glNormal3fv(v0.data());
            glVertex3fv(v0.data());
        }

        Vector3f v0(lastCosPhi, 0.0f, lastSinPhi);
        Vector3f v1(cosPhi,     0.0f, sinPhi);
        glTexCoord2f(1.0f, texT);
        glNormal3fv(v1.data());
        glVertex3fv(v1.data());
        glTexCoord2f(1.0f, lastTexT);
        glNormal3fv(v0.data());
        glVertex3fv(v0.data());

        glEnd();

        lastSinPhi = sinPhi;
        lastCosPhi = cosPhi;
        lastTexT = texT;
    }
#endif
#endif
}


void
WorldGeometry::renderNormalMappedSphere(RenderContext& /* rc */, int subdivisions) const
{
    // Not available without immediate mode
#ifndef VESTA_NO_IMMEDIATE_MODE_3D
    float lastSinPhi = -1.0f;
    float lastCosPhi = 0.0f;
    float lastTexT = 1.0f;
    int tangentLocation = ShaderBuilder::TangentAttributeLocation;

    // Render latitudinal bands
    for (int band = -subdivisions + 1; band <= subdivisions; band++)
    {
        float t = (float) (band) / (float) subdivisions;
        float phi = t * (float) PI / 2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        float texT = (1.0f - t) * 0.5f;

        glBegin(GL_QUAD_STRIP);

        for (int slice = 0; slice < subdivisions * 4; slice++)
        {
            float u = (float) slice / (float) (subdivisions * 4);
            float theta = u * 2.0f * (float) PI;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            float texS = u;

            Vector3f v0(lastCosPhi * cosTheta, lastCosPhi * sinTheta, lastSinPhi);
            Vector3f v1(cosPhi * cosTheta,     cosPhi * sinTheta, sinPhi);

            glTexCoord2f(texS, texT);
            glNormal3fv(v1.data());
            glVertexAttrib3f(tangentLocation, -sinTheta, cosTheta, 0.0f);
            glVertex3fv(v1.data());
            glTexCoord2f(texS, lastTexT);
            glNormal3fv(v0.data());
            glVertexAttrib3f(tangentLocation, -sinTheta, cosTheta, 0.0f);
            glVertex3fv(v0.data());
        }

        Vector3f v0(lastCosPhi, 0.0f, lastSinPhi);
        Vector3f v1(cosPhi,     0.0f, sinPhi);
        glTexCoord2f(1.0f, texT);
        glNormal3fv(v1.data());
        glVertexAttrib3f(tangentLocation, 0.0f, 1.0f, 0.0f);
        glVertex3fv(v1.data());
        glTexCoord2f(1.0f, lastTexT);
        glNormal3fv(v0.data());
        glVertexAttrib3f(tangentLocation, 0.0f, 1.0f, 0.0f);
        glVertex3fv(v0.data());

        glEnd();

        lastSinPhi = sinPhi;
        lastCosPhi = cosPhi;
        lastTexT = texT;
    }
#endif
}


void
WorldGeometry::renderBand(int subdivisions,
                          double latStart,
                          double latEnd,
                          double lonStart,
                          double lonEnd,
                          float tStart,
                          float tEnd) const
{
#ifndef VESTA_NO_IMMEDIATE_MODE_3D
    double lonStep = PI / (subdivisions * 2);
    float invLonRange = (float) (1.0 / (lonEnd - lonStart));
    int startLonStep = (int) floor(lonStart / lonStep) + 1;
    int endLonStep = (int) floor(lonEnd / lonStep);
    float sinLatStart = (float) std::sin(latStart);
    float cosLatStart = (float) std::cos(latStart);
    float sinLatEnd = (float) std::sin(latEnd);
    float cosLatEnd = (float) std::cos(latEnd);

    glBegin(GL_QUAD_STRIP);

    float sinLon = float(std::sin(lonStart));
    float cosLon = float(std::cos(lonStart));
    Vector3f v0(cosLatStart * cosLon, cosLatStart * sinLon, sinLatStart);
    Vector3f v1(cosLatEnd * cosLon,   cosLatEnd * sinLon,   sinLatEnd);
    glTexCoord2f(0.0f, 1.0f - tEnd);
    glNormal3fv(v1.data());
    glVertex3fv(v1.data());
    glTexCoord2f(0.0f, 1.0f - tStart);
    glNormal3fv(v0.data());
    glVertex3fv(v0.data());

    for (int i = startLonStep; i < endLonStep; ++i)
    {
        float lon = (float) (i * lonStep);

        sinLon = std::sin(lon);
        cosLon = std::cos(lon);
        float texS = (lon - (float) lonStart) * invLonRange;

        v0 = Vector3f(cosLatStart * cosLon, cosLatStart * sinLon, sinLatStart);
        v1 = Vector3f(cosLatEnd * cosLon,   cosLatEnd * sinLon,   sinLatEnd);

        glTexCoord2f(texS, 1.0f - tEnd);
        glNormal3fv(v1.data());
        glVertex3fv(v1.data());
        glTexCoord2f(texS, 1.0f - tStart);
        glNormal3fv(v0.data());
        glVertex3fv(v0.data());
    }

    sinLon = float(std::sin(lonEnd));
    cosLon = float(std::cos(lonEnd));
    v0 = Vector3f(cosLatStart * cosLon, cosLatStart * sinLon, sinLatStart);
    v1 = Vector3f(cosLatEnd * cosLon,   cosLatEnd * sinLon,   sinLatEnd);
    glTexCoord2f(1.0f, 1.0f - tEnd);
    glNormal3fv(v1.data());
    glVertex3fv(v1.data());
    glTexCoord2f(1.0f, 1.0f - tStart);
    glNormal3fv(v0.data());
    glVertex3fv(v0.data());

    glEnd();
#endif
}


void
WorldGeometry::renderPatch(int subdivisions,
                           const MapLayerBounds& box) const
{
    double latStep = PI / (subdivisions * 2);
    float invLatRange = (float) (1.0 / (box.north() - box.south()));
    int startLatStep = (int) floor(box.south() / latStep) + 1;
    int endLatStep   = (int) floor(box.north() / latStep);
    float west = (float) (box.west() + PI);
    float east = (float) (box.east() + PI);

    // Render latitudinal bands
    if (startLatStep > endLatStep)
    {
    }
    else
    {
        renderBand(subdivisions,
                   (float) box.south(), (float) (startLatStep * latStep), west, east,
                   0.0f, float(startLatStep * latStep - (float) box.south()) * invLatRange);
        for (int band = startLatStep; band < endLatStep; ++band)
        {
            float latStart = (float) (band * latStep);
            float latEnd = (float) ((band + 1) * latStep);
            renderBand(subdivisions,
                       latStart, latEnd, west, east,
                       (latStart - (float) box.south()) * invLatRange, (latEnd - (float) box.south()) * invLatRange);
        }
        renderBand(subdivisions,
                   (float) (endLatStep * latStep), (float) box.north(), west, east,
                   float(endLatStep * latStep - (float) box.south()) * invLatRange, 1.0f);
    }

}


float
WorldGeometry::boundingSphereRadius() const
{
    float r = maxRadius();

    float atmosphereHeight = 0.0f;
    if (m_atmosphere.isValid())
    {
        atmosphereHeight = m_atmosphere->transparentHeight();
    }

    if ((m_cloudMap.isValid() || m_tiledCloudMap.isValid()) && ms_cloudLayersVisible)
    {
        atmosphereHeight = max(atmosphereHeight, m_cloudAltitude);
    }

    float boundingRadius = r + atmosphereHeight;
    if (m_ringSystem.isValid())
    {
        boundingRadius = max(boundingRadius, m_ringSystem->outerRadius());
    }

    return boundingRadius;
}


float
WorldGeometry::nearPlaneDistance(const Eigen::Vector3f& cameraPosition) const
{
    // Use a custom calculation for the near plane distance. We're concerned about
    // clipping the main planet geometry and not as worried about the rings,
    // atmosphere, and cloud layer.
    // TODO: We should compute the distance to the planet ellipsoid (and eventually
    // the terrain model), not just the bounding sphere.
    float nearDistance = cameraPosition.norm() - maxRadius();
    if (m_ringSystem.isValid())
    {
        // Avoid near clipping of the rings; calculate the distance from the viewer
        // to the ring geometry. CameraPosition is in local coordinates, so
        // so |cameraPosition.z| is the distance to the ring plane.
        float ringPlaneDistance = abs(cameraPosition.z());

        // Calculate the distance between the rings and the projection of the camera
        // position onto the ring plane.
        Vector2f ringPlanePos(cameraPosition.x(), cameraPosition.y());
        float r = ringPlanePos.norm();
        float inPlaneDistance = 0.0f;
        if (r > m_ringSystem->outerRadius())
        {
            inPlaneDistance = r - m_ringSystem->outerRadius();
        }
        else
        {
            inPlaneDistance = m_ringSystem->innerRadius() - r;
        }

        float distanceToRings = max(1.0f, max(ringPlaneDistance, inPlaneDistance));
        nearDistance = min(nearDistance, distanceToRings);
    }

    return nearDistance;
}


bool
WorldGeometry::isOpaque() const
{
    // Rings are the only translucent part of a world (we'll ignore the
    // atmosphere for now.)
    if (m_ringSystem.isValid())
    {
        return false;
    }
    else
    {
        return true;
    }
}


/** Set the shape of the world to be a perfect sphere with the specified
  * radius.
  */
void
WorldGeometry::setSphere(float radius)
{
    m_ellipsoidAxes.fill(radius * 2.0f);
}


/** Set the shape to be an spheroid with distinct polar and equatorial
  * radii. The equatorial radius is given by radius, and the polar radius
  * is radius * (1 - oblateness). When oblateness is zero, the body will
  * be perfectly spherical; for oblateness > 0, it will be an oblate spheroid.
  */
void
WorldGeometry::setSpheroid(float radius, float oblateness)
{
    m_ellipsoidAxes = Vector3f(radius * 2.0f, radius * 2.0f, radius * (1.0f - oblateness) * 2.0f);
}


/** Set the shape to be triaxial ellipsoid with the specified
  * axis lengths.
  */
void
WorldGeometry::setEllipsoid(const Vector3f& axes)
{
    m_ellipsoidAxes = axes;
}


/** Set the global base texture.
  */
void
WorldGeometry::setBaseMap(TextureMap* baseMap)
{
    m_baseMap = baseMap;
}


/** Set a tiled map as the global base texture.
  */
void
WorldGeometry::setBaseMap(TiledMap* baseMap)
{
    m_baseTiledMap = baseMap;
}


/** Set the global normal map for this world.
  */
void
WorldGeometry::setNormalMap(TextureMap* normalMap)
{
    m_normalMap = normalMap;
}


/** Set a tiled map as the global normal map. The tiled normal
  * map will only be used if the base texture is also tiled.
  */
void
WorldGeometry::setNormalMap(TiledMap* normalMap)
{
    m_tiledNormalMap = normalMap;
}


/** Set whether this globe is self-luminous. If true, it
  * will not have any shading applied. Emissive true is the
  * appropriate setting for the Sun. Note that setting emissive
  * to true will *not* make the object a light source.
  */
void
WorldGeometry::setEmissive(bool emissive)
{
    m_emissive = emissive;
    setShadowCaster(!emissive);
}


/** Add a new map layer. The new layer is appended to the list layers and will
  * be drawn on top of previously added layers.
  */
void
WorldGeometry::addLayer(MapLayer* layer)
{
    m_mapLayers.push_back(counted_ptr<MapLayer>(layer));
}


/** Remove the topmost map layer. The method has no effect if there are no
  * map layers.
  */
void
WorldGeometry::removeLayer()
{
    if (!m_mapLayers.empty())
    {
        m_mapLayers.pop_back();
    }
}


/** Remove the map layer at the specified index. The method has no effect if
  * the index is invalid.
  */
void
WorldGeometry::removeLayer(unsigned int index)
{
    if (index < m_mapLayers.size())
    {
        m_mapLayers.erase(m_mapLayers.begin() + index);
    }
}


/** Get the map layer at the specified index. If the index is out
  * of range, the method returns null.
  */
MapLayer*
WorldGeometry::layer(unsigned int index) const
{
    if (index >= m_mapLayers.size())
    {
        return 0;
    }
    else
    {
        return m_mapLayers[index].ptr();
    }
}


/** Get the number of map layers (not including the base.)
  */
unsigned int
WorldGeometry::layerCount() const
{
    return m_mapLayers.size();
}


Atmosphere*
WorldGeometry::atmosphere() const
{
    return m_atmosphere.ptr();
}


/** Set the atmosphere for this world. The atmosphere may be null
  * for worlds without an atmosphere. Note that older graphics hardware
  * may not be capable of rendering atmospheres.
  */
void
WorldGeometry::setAtmosphere(Atmosphere* atmosphere)
{
    m_atmosphere = atmosphere;
}


/** Set the texture map used for the cloud layer. Clouds are only drawn
  * when a cloud map has been assigned.
  */
void
WorldGeometry::setCloudMap(TextureMap* cloudMap)
{
    m_cloudMap = cloudMap;
}


/** Set the tiled texture map used for the cloud layer. Clouds are only drawn
 *  when a cloud map has been assigned.
 */
void
WorldGeometry::setCloudMap(TiledMap* cloudMap)
{
    m_tiledCloudMap = cloudMap;
}


/** Set the ring system. Setting it to null indicates that the planet has no
  * ring system (the default state.)
  *
  * This method is retained for compatibility only. It is recommended instead
  * to create a separate entity for rings rather setting them as a property
  * of WorldGeometry. Rings will only cast shadows correctly when they are
  * separate entities.
  */
void
WorldGeometry::setRingSystem(PlanetaryRings* rings)
{
    m_ringSystem = rings;
}


bool
WorldGeometry::handleRayPick(const Eigen::Vector3d& pickOrigin,
                             const Eigen::Vector3d& pickDirection,
                             double /* clock */,
                             double* distance) const
{
    Vector3d semiAxes = (m_ellipsoidAxes * 0.5f).cast<double>();
    return TestRayEllipsoidIntersection(pickOrigin, pickDirection, semiAxes, distance);
}


/** Add a new layer with a specified tag. If a layer with the
  * same tag already exists, it will be replaced.
  */
void
WorldGeometry::setLayer(const std::string& tag, WorldLayer* layer)
{
    m_layers[tag] = counted_ptr<WorldLayer>(layer);
}


/** Remove the layer with the specified tag. The method has no
  * effect if the tag is not found.
  */
void
WorldGeometry::removeLayer(const std::string& tag)
{
    m_layers.erase(tag);
}


/** Get the layer with the specified tag. If no layer with
  * the requested tag exists, the method returns null.
  */
WorldLayer*
WorldGeometry::layer(const std::string& tag) const
{
    WorldLayerTable::const_iterator iter = m_layers.find(tag);
    if (iter == m_layers.end())
    {
        return NULL;
    }
    else
    {
        return iter->second.ptr();
    }
}


/** Return true if there are any layers.
  */
bool
WorldGeometry::hasLayers() const
{
    return !m_layers.empty();
}


/** Remove all layers from this world.
  */
void
WorldGeometry::clearLayers()
{
    m_layers.clear();
}

