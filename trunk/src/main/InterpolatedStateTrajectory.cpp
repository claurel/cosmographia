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

#include "InterpolatedStateTrajectory.h"

using namespace vesta;
using namespace Eigen;


// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
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

#include "InterpolatedStateTrajectory.h"
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


// A weak ordering on time/state records used for binary search by
// time.
class TimeStateOrdering
{
public:
    bool operator()(const InterpolatedStateTrajectory::TimeState& t0,
                    const InterpolatedStateTrajectory::TimeState& t1)
    {
        return t0.tsec < t1.tsec;
    }
};


/** Create a new interpolated state trajectory with the specified list
  * of time/state records.
  */
InterpolatedStateTrajectory::InterpolatedStateTrajectory(const TimeStateList& states) :
    m_period(0.0),
    m_boundingRadius(0.0)
{
    m_states = states;
}


InterpolatedStateTrajectory::~InterpolatedStateTrajectory()
{
}


// Perform cubici Hermite interpolation on the unit interval with
// the position and tangent at 0 given by r0, v0; and the position
// and tangent at 1 by r1, v1.
static StateVector
cubicHermitInterpolate(const Vector3d& r0, const Vector3d& v0,
                       const Vector3d& r1, const Vector3d& v1,
                       double t)
{
    double t2 = t * t;
    double t3 = t2 * t;

    // Derivatives
    double dt = 1.0;
    double dt2 = 2.0 * t;
    double dt3 = 3.0 * t2;

    Vector3d p = ((2 * t3 - 3 * t2 + 1) * r0 +
                  (t3 - 2 * t2 + t)     * v0 +
                  (-2 * t3 + 3 * t2)    * r1 +
                  (t3 - t2)             * v1);
    Vector3d v = ((2 * dt3 - 3 * dt2)   * r0 +
                  (dt3 - 2 * dt2 + dt)  * v0 +
                  (-2 * dt3 + 3 * dt2)  * r1 +
                  (dt3 - dt2)           * v1);

    return StateVector(p, v);
}


/** Calculate the state vector at the specified time (seconds since J2000 TDB)
  */
StateVector
InterpolatedStateTrajectory::state(double tdbSec) const
{
    if (m_states.empty())
    {
        return StateVector(Vector3d::Zero(), Vector3d::Zero());
    }

    TimeState ts;
    ts.tsec = tdbSec;
    TimeStateList::const_iterator iter = lower_bound(m_states.begin(), m_states.end(), ts, TimeStateOrdering());

    if (iter == m_states.begin())
    {
        return m_states.front().state;
    }
    else if (iter == m_states.end())
    {
        return m_states.back().state;
    }
    else
    {
        TimeState s0 = *(iter - 1);
        TimeState s1 = *iter;
        double h = s1.tsec - s0.tsec;
        double t = (tdbSec - s0.tsec) / h;


        StateVector s = cubicHermitInterpolate(s0.state.position(), s0.state.velocity() * h,
                                               s1.state.position(), s1.state.velocity() * h,
                                               t);
        return StateVector(s.position(), s.velocity() / h);
    }
}


double
InterpolatedStateTrajectory::boundingSphereRadius() const
{
    return 1.0e10;
}


bool
InterpolatedStateTrajectory::isPeriodic() const
{
    return m_period != 0.0;
}


double
InterpolatedStateTrajectory::period() const
{
    return m_period;
}


void
InterpolatedStateTrajectory::setPeriod(double period)
{
    m_period = period;
}
