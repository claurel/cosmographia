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

#include "InterpolatedRotation.h"
#include <algorithm>
#include <cassert>

using namespace vesta;
using namespace Eigen;
using namespace std;


// A weak ordering on time/state records used for binary search by
// time.
class TimeOrientationOrdering
{
public:
    bool operator()(const InterpolatedRotation::TimeOrientation& t0,
                    const InterpolatedRotation::TimeOrientation& t1)
    {
        return t0.tsec < t1.tsec;
    }
};


/** Create a new interpolated rotation model with the specified list
  * of time/orientation records.
  */
InterpolatedRotation::InterpolatedRotation(const TimeOrientationList& orientations)
{
    m_orientations = orientations;
}


InterpolatedRotation::~InterpolatedRotation()
{
}


/** Calculate the orientation at the specified time (seconds since J2000 TDB).
  * The interpolation technique is spherical linear (slerp).
  *
  * The input time is clamped to so that it lies within the range between
  * the first and last record.
  */
Quaterniond
InterpolatedRotation::orientation(double tdbSec) const
{
    if (!m_orientations.empty())
    {
        TimeOrientation t;
        t.tsec = tdbSec;
        TimeOrientationList::const_iterator iter = lower_bound(m_orientations.begin(), m_orientations.end(), t, TimeOrientationOrdering());

        if (iter == m_orientations.begin())
        {
            return m_orientations.front().orientation;
        }
        else if (iter == m_orientations.end())
        {
            return m_orientations.back().orientation;
        }
        else
        {
            TimeOrientation s0 = *(iter - 1);
            TimeOrientation s1 = *iter;
            double t = (tdbSec - s0.tsec) / (s1.tsec - s0.tsec);

            return s0.orientation.slerp(t, s1.orientation);
        }
    }
    else
    {
        return Quaterniond::Identity();
    }
}


Vector3d
InterpolatedRotation::angularVelocity(double tdbSec) const
{
    if (m_orientations.size() > 1)
    {
        TimeOrientation t0;
        TimeOrientation t1;

        TimeOrientation t;
        t.tsec = tdbSec;
        TimeOrientationList::const_iterator iter = lower_bound(m_orientations.begin(), m_orientations.end(), t, TimeOrientationOrdering());

        if (iter == m_orientations.begin())
        {
            t0 = m_orientations[0];
            t1 = m_orientations[1];
        }
        else if (iter == m_orientations.end())
        {
            unsigned int n = m_orientations.size();
            t0 = m_orientations[n - 2];
            t1 = m_orientations[n - 1];
        }
        else
        {
            t0 = *(iter - 1);
            t1 = *iter;
        }

        double h = t1.tsec - t0.tsec;

        // The derivative of a quaternion function q(t) (where t is a scalar) is
        // given by:
        //  1/2 w(t) q(t)
        //
        // Where w(t) given by a * v(t), with a the scalar angular velocity and
        // v(t) a unit direction vector.

        Quaterniond dq = t1.orientation * t0.orientation.conjugate();
        const double one = 1.0 - machine_epsilon<double>();

        if (abs(dq.w()) > one)
        {
            return Vector3d::Zero();
        }
        else
        {
            double halfTheta = acos(dq.w());
            return dq.vec().normalized() * (2.0 * halfTheta / h);
        }
    }
    else
    {
        return Vector3d::Zero();
    }
}
