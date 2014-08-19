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
