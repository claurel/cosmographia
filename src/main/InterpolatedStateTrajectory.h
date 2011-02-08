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

private:
    double m_period;
    double m_boundingRadius;
    TimeStateList m_states;
    TimePositionList m_positions;
};

#endif // _INTERPOLATED_STATE_TRAJECTORY_H_
