/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_FIXED_POINT_TRAJECTORY_H_
#define _VESTA_FIXED_POINT_TRAJECTORY_H_

#include "Trajectory.h"

namespace vesta
{

class FixedPointTrajectory : public Trajectory
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    FixedPointTrajectory(const Eigen::Vector3d& point);

    virtual StateVector state(double t) const;
    virtual double boundingSphereRadius() const;

private:
    Eigen::Vector3d m_point;
};

}

#endif // _VESTA_FIXED_POINT_TRAJECTORY_H_
