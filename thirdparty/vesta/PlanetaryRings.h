/*
 * $Revision: 253 $ $Date: 2010-04-30 06:31:38 -0500 (Fri, 30 Apr 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PLANETARY_RINGS_H_
#define _VESTA_PLANETARY_RINGS_H_

#include <Eigen/Core>
#include "Geometry.h"

namespace vesta
{
class TextureMap;

/** PlanetaryRings is a Geometry object used for ring systems around planets
  * such as Saturn. It can be inserted in the universe as a separate object,
  * or attached to a WorldGeometry via the setRings method. The latter is
  * preferred as it allows for special shadow calculations to be performed.
  */
class PlanetaryRings : public Geometry
{
public:
    /** Create a new PlanetaryRings geometry with the specified distances
      * for the inner and outer edges (both in kilometers)
      */
    PlanetaryRings(float innerRadius, float outerRadius);
    virtual ~PlanetaryRings();

    virtual void render(RenderContext& rc,
                        double clock) const;

    virtual float boundingSphereRadius() const;

    virtual bool isOpaque() const
    {
        return false;
    }

    /** Planetary rings are treated as ellipsoidal even though the geometry
      * is a degenerate ellipsoid.
      */
    virtual bool isEllipsoidal() const
    {
        return true;
    }

    virtual AlignedEllipsoid ellipsoid() const;

    /** Get the radius of the inner edge of the ring system (in kilometers)
      */
    float innerRadius() const
    {
        return m_innerRadius;
    }

    /** Set the radius of the inner edge of the ring system (in kilometers)
      */
    void setInnerRadius(float innerRadius)
    {
        m_innerRadius = innerRadius;
    }

    /** Get the radius of the outer edge of the ring system (in kilometers)
      */
    float outerRadius() const
    {
        return m_outerRadius;
    }

    /** Set the outer radius of the ring system in kilometers.
      */
    void setOuterRadius(float outerRadius)
    {
        m_outerRadius = outerRadius;
    }

    /** Get the texture that is applied to the rings.
      */
    TextureMap* texture() const
    {
        return m_texture.ptr();
    }

    void setTexture(TextureMap* texture);

private:
    float m_innerRadius;
    float m_outerRadius;
    counted_ptr<TextureMap> m_texture;
};

}

#endif // _VESTA_PLANETARY_RINGS_H_

