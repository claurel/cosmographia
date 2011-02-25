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

#ifndef _LINEAR_COMBINATION_TRAJECTORY_H_
#define _LINEAR_COMBINATION_TRAJECTORY_H_

#include <vesta/Trajectory.h>


/** LinearCombinationTrajectory is a trajectory constructed from two
  * other trajectories. The state is a sum of the states of the two
  * trajectories multiplied by weighting factors:
  *
  * s = w0 * s0 + w1 * s1
  *
  * It is used to create a trajectory for the Earth when just the orbits
  * of the Earth-Moon barycenter and Moon (relative to the EMB) are given
  * in the ephemeris. In this situation the weighting factors are 1.0 for
  * the EMB orbit and -(Moon mass / Earth+Moon mass).
  */
class LinearCombinationTrajectory : public vesta::Trajectory
{
public:
    LinearCombinationTrajectory(vesta::Trajectory* trajectory0,
                                double weight0,
                                vesta::Trajectory* trajectory1,
                                double weight1);
    ~LinearCombinationTrajectory();

    virtual vesta::StateVector state(double tdbSec) const;
    virtual double boundingSphereRadius() const;
    virtual bool isPeriodic() const;
    virtual double period() const;
    void setPeriod(double period);

private:
    vesta::counted_ptr<vesta::Trajectory> m_trajectory0;
    vesta::counted_ptr<vesta::Trajectory> m_trajectory1;
    double m_weight0;
    double m_weight1;
    double m_period;
};

#endif // _LINEAR_COMBINATION_TRAJECTORY_H_
