// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#ifndef _INTERPOLATED_ROTATION_H_
#define _INTERPOLATED_ROTATION_H_

#include <vesta/RotationModel.h>
#include <Eigen/StdVector>
#include <vector>


/** An InterpolatedStateTrajectory computes state vectors by interpolating
  * between entries in a table of time/state vector pairs or time/position
  * pairs with estimated velocities. Because the records are time-tagged,
  * they need not be evenly spaced in time.
  *
  * Currently, the interpolation method is always cubic Hermite.
  *
  * Note that providing velocities greatly improves the accuracy of the
  * interpolated approximation with respect to the original trajectory. When
  * available, velocities should be given; if memory is constrained, it is
  * better accuracy can be achieved by reducing the number of records by
  * half rather than using postions instead of state vectors.
  */
class InterpolatedRotation : public vesta::RotationModel
{
public:
    struct TimeOrientation
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        double tsec;
        Eigen::Quaterniond orientation;
    };
    typedef std::vector<TimeOrientation, Eigen::aligned_allocator<TimeOrientation> > TimeOrientationList;

    InterpolatedRotation(const TimeOrientationList& orientations);
    ~InterpolatedRotation();

    virtual Eigen::Quaterniond orientation(double tdbSec) const;
    virtual Eigen::Vector3d angularVelocity(double tdbSec) const;

private:
    TimeOrientationList m_orientations;
};

#endif // _INTERPOLATED_ROTATION_H_
