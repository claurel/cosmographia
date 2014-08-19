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

#ifndef _INTERPOLATED_STATE_TRAJECTORY_H_
#define _INTERPOLATED_STATE_TRAJECTORY_H_

#include <vesta/Trajectory.h>
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
class InterpolatedStateTrajectory : public vesta::Trajectory
{
public:
    struct TimeState
    {
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
        double tsec;
        vesta::StateVector state;
    };
    typedef std::vector<TimeState, Eigen::aligned_allocator<TimeState> > TimeStateList;

    struct TimePosition
    {
        double tsec;
        Eigen::Vector3d position;
    };
    typedef std::vector<TimePosition> TimePositionList;

    InterpolatedStateTrajectory(const TimeStateList& states);
    InterpolatedStateTrajectory(const TimePositionList& positions);
    ~InterpolatedStateTrajectory();

    virtual vesta::StateVector state(double tdbSec) const;
    virtual double boundingSphereRadius() const;
    virtual bool isPeriodic() const;
    virtual double period() const;

    void setPeriod(double period);

    unsigned int stateCount() const;
    double time(unsigned int index) const;

private:
    double m_period;
    double m_boundingRadius;
    TimeStateList m_states;
    TimePositionList m_positions;
};

#endif // _INTERPOLATED_STATE_TRAJECTORY_H_
