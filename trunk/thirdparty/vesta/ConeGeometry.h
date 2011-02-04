/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_CONE_GEOMETRY_H_
#define _VESTA_CONE_GEOMETRY_H_

#include <Eigen/Core>
#include "Geometry.h"
#include "Spectrum.h"


namespace vesta
{

/** ConeGeometry is a Geometry object with a single cone. The
  * apex is at the origin, and the base of the cone is perpendicular
  * to the axis and centered on the axis.
  *
  * The cone is intended to be used to show instrument fields
  * of view rather than physical objects. Thus, no surface normals
  * are generated and material properties other than color and
  * opacity may be set.
  */
class ConeGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    ConeGeometry(double apexAngle,
                 double height);
    virtual ~ConeGeometry();

    void render(RenderContext& rc,
                double clock) const;

    float boundingSphereRadius() const;

    virtual bool isOpaque() const
    {
        return m_opacity >= 1.0f;
    }

    double apexAngle() const
    {
        return m_apexAngle;
    }

    void setApexAngle(double apexAngle)
    {
        m_apexAngle = apexAngle;
    }

    double height() const
    {
        return m_height;
    }

    void setHeight(double height)
    {
        m_height = height;
    }

    Spectrum color() const
    {
        return m_color;
    }

    void setColor(const Spectrum& color)
    {
        m_color = color;
    }

    /** Get the opacity (0 = completely transparent, 1 = opaque) of
      * the arrow geometry.
      */
    float opacity() const
    {
        return m_opacity;
    }

    /** Set opacity of the arrows (0 = completely transparent, 1 = opaque).
      */
    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    Eigen::Vector3d axis() const
    {
        return m_axis;
    }

    /** Set the axis of the cone. The axis must be a normalized vector.
      */
    void setAxis(const Eigen::Vector3d& axis)
    {
        m_axis = axis;
    }


private:
    double m_apexAngle;
    double m_height;
    Spectrum m_color;
    float m_opacity;
    Eigen::Vector3d m_axis;
};

}

#endif // _VESTA_CONE_GEOMETRY_H_

