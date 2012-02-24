/*
 * $Revision: 565 $ $Date: 2011-02-15 16:00:43 -0800 (Tue, 15 Feb 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ELLIPSOID_H_
#define _VESTA_ELLIPSOID_H_

#include "GeneralEllipse.h"
#include "PlanetographicCoord.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>


namespace vesta
{

/** AlignedEllipsoid represents an ellipsoid with principal axes
  * aligned to coordinate axes.
  */
class AlignedEllipsoid
{
public:
    /** Create an ellipsoid with aligned principal axes of specified lengths.
      *
      * \param semiAxes lengths of the semi-axes aligned with the x- y- and z- coordinate axes.
      */
    AlignedEllipsoid(const Eigen::Vector3d& semiAxes) :
        m_semiAxes(semiAxes)
    {
    }

    /** Get the ellipsoid semi-axes.
     */
    Eigen::Vector3d semiAxes() const
    {
        return m_semiAxes;
    }

    /** Get the surface normal at some point on the ellipsoid.
      */
    Eigen::Vector3d normal(const Eigen::Vector3d& v)
    {
        return (m_semiAxes.cwise().square().cwise().inverse().asDiagonal() * v).normalized();
    }

    /** Get the length of the semi-major axis.
      */
    double semiMajorAxisLength() const
    {
        return m_semiAxes.maxCoeff();
    }

    /** Return true if one or more of the ellipsoid axes is zero length.
      * This is the case when the "ellipsoid" is actually an ellipse,
      * line segment, or point.
      */
    bool isDegenerate() const
    {
        return m_semiAxes.x() <= 0.0 || m_semiAxes.y() <= 0.0 || m_semiAxes.z() <= 0.0;
    }

    /** Convert a planetographic coordinate to rectangular coordinates.
      * When c lies on the ellipsoid, the latitude is the angle between the ellipsoid
      * normal at c and the xy-plane.
      */
    Eigen::Vector3d planetographicToRectangular(const PlanetographicCoord3& c)
    {
        // Compute the planetographic normal
        double cosLat = std::cos(c.latitude());
        Eigen::Vector3d n(cosLat * std::cos(c.longitude()),
                          cosLat * std::sin(c.longitude()),
                          std::sin(c.latitude()));

        Eigen::Vector3d k = m_semiAxes.cwise().square().cwise() * n;
        double s = std::sqrt(k.dot(n));

        Eigen::Vector3d surfacePoint = k / s;
        return surfacePoint + c.height() * n;
    }


    PlanetographicCoord3 rectangularToPlanetographic(const Eigen::Vector3d& r);
    Eigen::Vector3d nearestPoint(const Eigen::Vector3d& r);
    double distance(const Eigen::Vector3d& r);

    GeneralEllipse intersection(const Eigen::Hyperplane<double, 3>& plane, bool* foundIntersection) const;
    GeneralEllipse limb(const Eigen::Vector3d& p) const;

    GeneralEllipse orthogonalProjection(const Eigen::Vector3d& planeNormal) const;

private:
    Eigen::Vector3d m_semiAxes;
};

}

#endif // _VESTA_ELLIPSOID_H_
