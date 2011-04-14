/*
 * $Revision: 565 $ $Date: 2011-02-15 16:00:43 -0800 (Tue, 15 Feb 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_GLARE_OVERLAY_H_
#define _VESTA_GLARE_OVERLAY_H_

#include "RenderContext.h"
#include "OGLHeaders.h"
#include <vector>

namespace vesta
{

/** The GlareOverlay class tracks and manages glare effects from light sources.
  * In general, there should be one glare layer for each view rendered with
  * UniverseRenderer.
  */
class GlareOverlay : public Object
{
    friend class UniverseRenderer;

public:
    ~GlareOverlay();

    /** Get the rate at which glare brightness changes when a light source becomes visible
      * or invisible.
      *
      * \see setAdaptationRate
      */
    float adaptationRate() const
    {
        return m_adaptationRate;
    }

    /** Set the rate at which glare brightness changes when a light source becomes visible
      * or invisible. The rate should be a value greater than zero and less than or equal
      * to one. Setting the rate to one will make glare appear at full brightness as soon
      * as a light source is revealed. Glare is less visually jarring at lower adaptation
      * rates, around 0.1 - 0.15.
      */
    void setAdaptationRate(float rate)
    {
        m_adaptationRate = rate;
    }

    /** Return the radius of the glare in pixels. The glare may exceed this size if the
      * apparent screen size of the light source geometry is larger.
      */
    float glareSize()
    {
        return m_glareSize;
    }

    /** Set the radius of the glare in pixels. The glare may exceed this size if the
      * apparent screen size of the light source geometry is larger. The default size
      * is 100 pixels.
      */
    void setGlareSize(float radiusInPixels)
    {
        m_glareSize = radiusInPixels;
    }

    void adjustBrightness();

private:
    GlareOverlay();

    bool initialize();
    void renderGlare(RenderContext& rc, const LightSource* m_lightSource, const Eigen::Vector3f& glarePosition, float lightRadius);
    void trackGlare(RenderContext& rc, const LightSource* m_lightSource, const Eigen::Vector3f& glarePosition, float lightRadius);

    GLuint getFreeOcclusionQuery();
    void drawOcclusionTestGeometry(RenderContext& rc, const Eigen::Vector3f& position, float lightRadius);
    void drawGlareGeometry(RenderContext& rc, const Eigen::Vector3f& position, float lightRadius);

    struct GlareItem
    {
        GlareItem() : m_brightness(0.0f), m_occlusionQuery(0) {}
        counted_ptr<LightSource> m_lightSource;
        float m_brightness;
        GLuint m_occlusionQuery;
    };

    std::vector<GlareItem> m_activeGlareItems;
    std::vector<GLuint> m_freeOcclusionQueries;

    float m_adaptationRate;
    float m_glareSize;
};

}

#endif // _VESTA_GLARE_OVERLAY_H_
