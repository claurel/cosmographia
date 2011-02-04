/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_KEPLERIAN_TRAJECTORY_H_
#define _VESTA_KEPLERIAN_TRAJECTORY_H_

#include "Trajectory.h"
#include "OrbitalElements.h"
#include <Eigen/Geometry>

namespace vesta
{

class KeplerianTrajectory : public Trajectory
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    KeplerianTrajectory(const OrbitalElements& elements);

    virtual StateVector state(double t) const;
    virtual double boundingSphereRadius() const;

    virtual bool isPeriodic() const
    {
        // Circular and elliptical orbits are periodic; hyperbolic
        // and parabolic orbits are not.
        return m_elements.eccentricity < 1.0;
    }

    virtual double period() const;

private:
    OrbitalElements m_elements;
    Eigen::Quaterniond m_orbitOrientation;
};

}

#endif // _VESTA_KEPLERIAN_TRAJECTORY_H_
