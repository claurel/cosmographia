// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
