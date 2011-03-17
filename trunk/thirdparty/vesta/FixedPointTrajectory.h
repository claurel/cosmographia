/*
 * $Revision: 554 $ $Date: 2010-11-09 08:27:59 -0800 (Tue, 09 Nov 2010) $
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
    FixedPointTrajectory(const StateVector& state);

    virtual StateVector state(double t) const;
    virtual double boundingSphereRadius() const;

    void setState(const StateVector& state);

private:
    StateVector m_state;
};

}

#endif // _VESTA_FIXED_POINT_TRAJECTORY_H_
