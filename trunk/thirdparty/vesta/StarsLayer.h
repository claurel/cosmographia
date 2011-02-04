/*
 * $Revision: 375 $ $Date: 2010-07-20 12:25:37 -0700 (Tue, 20 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_STARS_LAYER_H_
#define _VESTA_STARS_LAYER_H_

#include "SkyLayer.h"
#include "StarCatalog.h"


namespace vesta
{

class GLVertexBuffer;
class GLShaderProgram;

class StarsLayer : public SkyLayer
{
public:
    StarsLayer();
    explicit StarsLayer(StarCatalog* starCatalog);
    ~StarsLayer();

    StarCatalog* starCatalog() const
    {
        return m_starCatalog.ptr();
    }

    void setStarCatalog(StarCatalog* starCatalog);

    virtual void render(RenderContext& rc);

    enum StarStyle
    {
        PointStars    = 0,
        GaussianStars = 1,
    };

    /** Get the style used for star rendering. GaussianStars is more realistic, but
      * is only available on graphics hardware that supports GLSL shaders.
      */
    StarStyle style() const
    {
        return m_style;
    }

    void setStyle(StarStyle style);

    /** Return the apparent magnitude of the faintest stars that will be visible.
      */
    float limitingMagnitude() const
    {
        return m_limitingMagnitude;
    }

    /** Set the magnitude of the faintest stars that will be visible. A value of
      * 6.5 is approximately right for a human observer under clear, dark skies.
      */
    void setLimitingMagnitude(float limitingMagnitude);

    /** Get the brightness of the diffraction spike effect.
      *
      * @see StarsLayer::setDiffractionSpikeBrightness
      *
      * \return a value between 0 and 1
      */
    float diffractionSpikeBrightness()
    {
        return m_diffractionSpikeBrightness;
    }

    /** Set the brightness of the diffraction spike effect. Diffraction spikes are
      * an optical artifact caused by the secondary mirror support structures in
      * a reflecting telescope. They can be used to make stars appear more
      * brilliant when rendered on a device with limited dynamic range, such as
      * a standard computer monitor.
      *
      * \param brightness a value between 0 and 1 that gives the intensity of the
      * difraction spikes (where 0 disables the effect and 1 is maximum intensity)
      */
    void setDiffractionSpikeBrightness(float brightness)
    {
        m_diffractionSpikeBrightness = brightness;
    }

private:
    void updateVertexBuffer();

private:
    counted_ptr<StarCatalog> m_starCatalog;
    char* m_vertexArray;
    counted_ptr<GLVertexBuffer> m_vertexBuffer;
    counted_ptr<GLShaderProgram> m_starShader;
    counted_ptr<GLShaderProgram> m_starShaderSRGB;
    bool m_vertexBufferCurrent;
    bool m_starShaderCompiled;
    StarStyle m_style;
    float m_limitingMagnitude;
    float m_diffractionSpikeBrightness;
};

}
#endif // _VESTA_STARS_LAYER_H_
