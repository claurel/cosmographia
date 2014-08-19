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

#include "LinearCombinationTrajectory.h"
#include <cmath>

using namespace vesta;
using namespace Eigen;


/** Create a LinearCombinationTrajectory. It is legal for either or both of
  * the 'child' trajectories to be null; the assumed value for the state vector
  * of a null trajector is zero. This fact can be used to create a
  * LinearCombinationTrajectory that just scales the state of a single
  * trajectory.
  */
LinearCombinationTrajectory::LinearCombinationTrajectory(Trajectory* trajectory0,
                                                         double weight0,
                                                         Trajectory* trajectory1,
                                                         double weight1) :
    m_trajectory0(trajectory0),
    m_trajectory1(trajectory1),
    m_weight0(weight0),
    m_weight1(weight1),
    m_period(0.0)
{
}


LinearCombinationTrajectory::~LinearCombinationTrajectory()
{
}


vesta::StateVector
LinearCombinationTrajectory::state(double tdbSec) const
{
    StateVector s0 = StateVector(Vector6d::Zero());
    StateVector s1 = StateVector(Vector6d::Zero());

    if (m_trajectory0.isValid())
    {
        s0 = m_trajectory0->state(tdbSec);
    }

    if (m_trajectory1.isValid())
    {
        s1 = m_trajectory1->state(tdbSec);
    }

    return StateVector(m_weight0 * s0.state()) + StateVector(m_weight1 * s1.state());
}


double
LinearCombinationTrajectory::boundingSphereRadius() const
{
    double r0 = 0.0;
    double r1 = 0.0;

    if (m_trajectory0.isValid())
    {
        r0 = m_trajectory0->boundingSphereRadius();
    }

    if (m_trajectory1.isValid())
    {
        r1 = m_trajectory1->boundingSphereRadius();
    }

    return std::abs(m_weight0) * r0 + std::abs(m_weight1) * r1;
}


bool
LinearCombinationTrajectory::isPeriodic() const
{
    return m_period != 0.0;
}


/** Return the period of the trajectory in seconds (or zero if the
  * trajectory is not approximately periodic.
  */
double
LinearCombinationTrajectory::period() const
{
    return m_period;
}


/** Set the period of the trajectory in seconds. If the period is set
  * to zero, the trajectory is treated as aperiodic. The period is
  * relevant for plotting.
  */
void
LinearCombinationTrajectory::setPeriod(double period)
{
    m_period = period;
}
