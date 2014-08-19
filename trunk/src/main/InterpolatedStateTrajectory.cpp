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

#include "InterpolatedStateTrajectory.h"
#include <algorithm>
#include <cassert>

using namespace vesta;
using namespace Eigen;
using namespace std;


// A weak ordering on time/orientation records used for binary search by
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


// A weak ordering on time/position records used for binary search by
// time.
class TimePositionOrdering
{
public:
    bool operator()(const InterpolatedStateTrajectory::TimePosition& t0,
                    const InterpolatedStateTrajectory::TimePosition& t1)
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
    if (!states.empty())
    {
        setValidTimeRange(states.front().tsec, states.back().tsec);
    }

    for (TimeStateList::const_iterator iter = states.begin(); iter != states.end(); ++iter)
    {
        m_boundingRadius = std::max(m_boundingRadius, iter->state.position().norm());
    }
}


/** Create a new interpolated state trajectory with the specified list
  * of position/state records. Velocities are estimated by three-point
  * differences for all segments other than the ends.
  */
InterpolatedStateTrajectory::InterpolatedStateTrajectory(const TimePositionList& positions) :
    m_period(0.0),
    m_boundingRadius(0.0)
{
    m_positions = positions;
    if (!positions.empty())
    {
        setValidTimeRange(positions.front().tsec, positions.back().tsec);
    }

    for (TimePositionList::const_iterator iter = positions.begin(); iter != positions.end(); ++iter)
    {
        m_boundingRadius = std::max(m_boundingRadius, iter->position.norm());
    }
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


static Vector3d
estimateVelocity(const InterpolatedStateTrajectory::TimePositionList& positions, unsigned int index)
{
    assert(index < positions.size());

    if (index == 0)
    {
        assert(positions.size() > 1);

        // One-sided difference for first point
        double h = positions[1].tsec - positions[0].tsec;
        return (positions[1].position - positions[0].position) / h;
    }
    else if (index == positions.size() - 1)
    {
        assert(index > 0);

        // One-sided difference for last point
        double h = positions[index].tsec - positions[index - 1].tsec;
        return (positions[index].position - positions[index - 1].position) / h;
    }
    else
    {
        assert(index > 0 && index + 1 < positions.size());

        // Three-point difference for points in the middle
        double h0 = positions[index].tsec - positions[index - 1].tsec;
        double h1 = positions[index + 1].tsec - positions[index].tsec;
        return 0.5 * ((positions[index].position - positions[index - 1].position) / h0 +
                      (positions[index + 1].position - positions[index].position) / h1);
    }
}


/** Calculate the state vector at the specified time (seconds since J2000 TDB).
  *
  * The input time is clamped to so that it lies within the range between
  * the first and last record.
  */
StateVector
InterpolatedStateTrajectory::state(double tdbSec) const
{
    if (!m_states.empty())
    {
        // Use state vector table
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
    else if (!m_positions.empty())
    {
        // Use position table

        TimePosition ts;
        ts.tsec = tdbSec;
        TimePositionList::const_iterator iter = lower_bound(m_positions.begin(), m_positions.end(), ts, TimePositionOrdering());

        if (iter == m_positions.begin())
        {
            Vector3d velocity = estimateVelocity(m_positions, 0);
            return StateVector(m_positions.front().position, velocity);
        }
        else if (iter == m_positions.end())
        {
            Vector3d velocity = estimateVelocity(m_positions, m_positions.size() - 1);
            return StateVector(m_positions.back().position, velocity);
        }
        else
        {
            TimePosition s0 = *(iter - 1);
            TimePosition s1 = *iter;
            Vector3d v0 = estimateVelocity(m_positions, iter - m_positions.begin() - 1);
            Vector3d v1 = estimateVelocity(m_positions, iter - m_positions.begin());
            double h = s1.tsec - s0.tsec;
            double t = (tdbSec - s0.tsec) / h;

            StateVector s = cubicHermitInterpolate(s0.position, v0 * h, s1.position, v1 * h, t);
            return StateVector(s.position(), s.velocity() / h);
        }
    }
    else
    {
        return StateVector(Vector3d::Zero(), Vector3d::Zero());
    }
}


double
InterpolatedStateTrajectory::boundingSphereRadius() const
{
    return m_boundingRadius;
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


unsigned int
InterpolatedStateTrajectory::stateCount() const
{
    if (!m_states.empty())
    {
        return m_states.size();
    }
    else
    {
        return m_positions.size();
    }
}


double
InterpolatedStateTrajectory::time(unsigned int index) const
{
    if (!m_states.empty())
    {
        if (index < m_states.size())
        {
            return m_states[index].tsec;
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        if (index < m_positions.size())
        {
            return m_positions[index].tsec;
        }
        else
        {
            return 0.0;
        }
    }
}
