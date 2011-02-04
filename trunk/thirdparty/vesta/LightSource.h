/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_LIGHT_SOURCE_H_
#define _VESTA_LIGHT_SOURCE_H_

#include "Object.h"
#include "Spectrum.h"
#include <Eigen/Core>


namespace vesta
{

class LightSource : public Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    LightSource();
    ~LightSource()
    {
    }

    /** Get the luminosity of the light source in watts.
      */
    float luminosity() const
    {
        return m_luminosity;
    }

    /** Set the luminosity of the light source in watts.
      */
    void setLuminosity(float luminosity)
    {
        m_luminosity = luminosity;
    }

    /** Get the color of the light source.
      */
    Spectrum spectrum() const
    {
        return m_spectrum;
    }

    /** Set the light source color.
      */
    void setSpectrum(const Spectrum& spectrum)
    {
        m_spectrum = spectrum;
    }

    /** Get the range of the light source in kilometers. This value is ignored
      * when rendering in 'physical mode'. Otherwise, the scene is rendered with
      * quadratic falloff so that the light is visually undetectable beyond the
      * range. In practice, this means that the light intensity at range will be
      * 1/256.
      */
    float range() const
    {
        return m_range;
    }

    /** Set the range of the light source in kilometers.
      * @see LightSource::range()
      */
    void setRange(float range)
    {
        m_range = range;
    }

    /** Return true if this light casts shadows. By default lights do not
      * cast shadows.
      */
    bool isShadowCaster() const
    {
        return m_shadowCaster;
    }

    /** Set whether this light should cast shadows onto other objects.
      */
    void setShadowCaster(bool castsShadows)
    {
        m_shadowCaster = castsShadows;
    }

private:
    float m_luminosity;
    Spectrum m_spectrum;
    float m_range;
    bool m_shadowCaster;
};

}

#endif // _VESTA_LIGHT_SOURCE_H_
