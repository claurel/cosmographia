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
#include <Eigen/Core>
#include <Eigen/Geometry>


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
        return m_semiAxes.cwise().square().cwise().inverse().asDiagonal() * v;
    }

    GeneralEllipse intersection(const Eigen::Hyperplane<double, 3>& plane, bool* foundIntersection) const;
    GeneralEllipse limb(const Eigen::Vector3d& p) const;

private:
    Eigen::Vector3d m_semiAxes;
};

}

#endif // _VESTA_ELLIPSOID_H_
