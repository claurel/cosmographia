/*
 * $Revision: 597 $ $Date: 2011-03-31 09:25:53 -0700 (Thu, 31 Mar 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_UNIVERSE_RENDERER_H_
#define _VESTA_UNIVERSE_RENDERER_H_

#include "Universe.h"
#include "SkyLayer.h"
#include "Spectrum.h"
#include "Viewport.h"
#include "Frustum.h"
#include "LightingEnvironment.h"
#include "PlanarProjection.h"
#include <Eigen/StdVector>
#include <vector>

namespace vesta
{

class Observer;
class RenderContext;
class Framebuffer;
class CubeMapFramebuffer;
class EclipseShadowVolumeSet;
class TextureFont;

/** UniverseRenderer draws views of a VESTA Universe using a 3D rendering
  * library. Views are drawn as sets at a particular time. A typical usage
  * of UniverseRenderer looks like this:
  *
  * \code
  * UniverseRenderer* renderer = new UniverseRenderer();
  *
  * IntializeOpenGL();
  *
  * renderer->initializeGraphics();
  *
  * while (!done)
  * {
  *     glClear();
  *     renderer->beginViewSet(universe, simulationTime);
  *     renderer->renderView(observer1, fov1, viewWidth1, viewHeight2);
  *     renderer->renderView(observer2, fov2, viewHeight2, viewHeight2);
  *     simulationTime += deltaT;
  * }
  * \endcode
  *
  * The above code renders two different views at each step. This might happen
  * when rendering a stereo pair, or when drawing a secondary view inset.
  */
class UniverseRenderer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    UniverseRenderer();
    ~UniverseRenderer();

    bool initializeGraphics();

    enum RenderStatus
    {
        RenderOk,
        RenderNoViewSet,
        RenderViewSetAlreadyStarted,
        RendererUninitialized,
        RendererBadParameter,
    };

    static const unsigned int MaxShadowMaps     = 3;
    static const unsigned int MaxOmniShadowMaps = 3;

    RenderStatus beginViewSet(const Universe* universe, double t);
    RenderStatus endViewSet();

    RenderStatus renderView(const Observer* observer,
                            double fieldOfView,
                            int viewportWidth,
                            int viewportHeight);
    RenderStatus renderView(const LightingEnvironment* lighting,
                            const Observer* observer,
                            double fieldOfView,
                            const Viewport& viewport,
                            Framebuffer* renderSurface = 0);
    RenderStatus renderView(const LightingEnvironment* lighting,
                            const Eigen::Vector3d& cameraPosition,
                            const Eigen::Quaterniond& cameraOrientation,
                            const PlanarProjection& projection,
                            const Viewport& viewport,
                            Framebuffer* renderSurface = 0);
    RenderStatus renderCubeMap(const LightingEnvironment* lighting,
                               const Eigen::Vector3d& cameraPosition,
                               CubeMapFramebuffer* cubeMap,
                               double nearDistance = MinimumNearDistance,
                               double farDistance = MaximumFarDistance,
                               const Eigen::Quaterniond& rotation = Eigen::Quaterniond::Identity());
    RenderStatus renderShadowCubeMap(const LightingEnvironment* lighting,
                                     const Eigen::Vector3d& cameraPosition,
                                     CubeMapFramebuffer* cubeMap);

    Spectrum ambientLight() const
    {
        return m_ambientLight;
    }

    void setAmbientLight(const Spectrum& spectrum);

    bool initializeShadowMaps(unsigned int shadowMapSize = 1024,
                              unsigned int shadowMapCount = 1);
    bool initializeOmniShadowMaps(unsigned int shadowMapSize = 1024,
                                  unsigned int shadowMapCount = 1);


    /** Return true if this renderer has shadows enabled.
      */
    bool shadowsEnabled() const
    {
        return m_shadowsEnabled;
    }

    /** Return true if this renderer has eclipse shadows enabled
      */
    bool eclipseShadowsEnabled() const
    {
        return m_eclipseShadowsEnabled;
    }

    void setShadowsEnabled(bool enable);
    void setEclipseShadowsEnabled(bool enable);

    bool shadowsSupported() const;
    bool omniShadowsSupported() const;

    /** Return true if visualizers will be drawn. Visualizers are on by default.
     */
    bool visualizersEnabled() const
    {
        return m_visualizersEnabled;
    }
    void setVisualizersEnabled(bool enable);

    /** Return true if sky layers will be drawn. Sky layers are on by default.
      */
    bool skyLayersEnabled() const
    {
        return m_skyLayersEnabled;
    }
    void setSkyLayersEnabled(bool enable);

    TextureFont* defaultFont() const;
    void setDefaultFont(TextureFont* font);

public:
    struct VisibleItem
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        const Entity* entity;
        const Geometry* geometry;
        Eigen::Vector3d position;
        Eigen::Vector3d cameraRelativePosition;
        Eigen::Quaternionf orientation;
        float nearDistance;    // signed distance to the camera plane
        float farDistance;     // signed distance to the camera plane
        float boundingRadius;
        bool outsideFrustum;
    };

    struct LightSourceItem
    {
        const LightSource* lightSource;
        Eigen::Vector3d position;
        double radius;
    };

    struct VisibleLightSourceItem
    {
        const LightSource* lightSource;
        Eigen::Vector3d position;
        Eigen::Vector3d cameraRelativePosition;
    };

    typedef std::vector<VisibleItem, Eigen::aligned_allocator<VisibleItem> > VisibleItemVector;

    struct DepthBufferSpan
    {
        float nearDistance;
        float farDistance;
        unsigned int backItemIndex;
        unsigned int itemCount;
    };

    /** Minimum distance to the near clipping plane; objects nearer to the observer than this distance
      * will always be culled.
      */
    static const float MinimumNearDistance;

    /** Maximum distance to the far clipping plane; objects further from the observer than this distance
      * will always be culled.
      */
    static const float MaximumFarDistance;

private:
    void buildVisibleLightSourceList(const Eigen::Vector3d& cameraPosition);
    void splitDepthBuffer();
    void coalesceDepthBuffer();
    void renderDepthBufferSpan(const DepthBufferSpan& span, const PlanarProjection& projection);
    bool renderDepthBufferSpanShadows(unsigned int shadowIndex,
                                      const DepthBufferSpan& span,
                                      const Eigen::Vector3d& lightPosition);
    bool renderDepthBufferSpanOmniShadows(unsigned int shadowIndex,
                                          const DepthBufferSpan& span,
                                          const LightSource* light,
                                          const Eigen::Vector3d& lightPosition);
    void setupEclipseShadows(const VisibleItem& item);
    void addVisibleItem(const Entity* entity,
                        const Geometry* geometry,
                        const Eigen::Vector3d& position,
                        const Eigen::Vector3d& cameraRelativePosition,
                        const Eigen::Vector3f& cameraSpacePosition,
                        const Eigen::Quaternionf& orientation,
                        float nearAdjust);
    void drawItem(const VisibleItem& item);
    Eigen::Matrix4f setupShadowRendering(const Framebuffer* shadowMap,
                                         const Eigen::Vector3f& lightDirection,
                                         float shadowGroupSize);

    void setDepthRange(float front, float back);

private:
    RenderContext* m_renderContext;

    const Universe* m_universe;
    double m_currentTime;

    VisibleItemVector m_visibleItems;
    VisibleItemVector m_splittableItems;
    std::vector<DepthBufferSpan> m_depthBufferSpans;
    std::vector<DepthBufferSpan> m_mergedDepthBufferSpans;
    std::vector<LightSourceItem> m_lightSources;
    std::vector<VisibleLightSourceItem> m_visibleLightSources;

    Spectrum m_ambientLight;
    std::vector<counted_ptr<SkyLayer> > m_skyLayers;

    std::vector<counted_ptr<Framebuffer> > m_shadowMaps;
    std::vector<counted_ptr<CubeMapFramebuffer> > m_omniShadowMaps;

    bool m_shadowsEnabled;
    bool m_eclipseShadowsEnabled;
    bool m_visualizersEnabled;
    bool m_skyLayersEnabled;
    float m_depthRangeFront;
    float m_depthRangeBack;

    counted_ptr<Framebuffer> m_renderSurface;
    Viewport m_renderViewport;
    bool m_renderColorMask[4];

    Frustum m_viewFrustum;

    const LightingEnvironment* m_lighting;
    counted_ptr<LightSource> m_sun;

    counted_ptr<EclipseShadowVolumeSet> m_eclipseShadows;

    bool m_viewIndependentInitializationRequired;

    counted_ptr<TextureFont> m_defaultFont;
};

}

#endif // _VESTA_UNIVERSE_RENDERER_H_
