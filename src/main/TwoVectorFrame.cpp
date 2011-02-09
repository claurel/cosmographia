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

#include "TwoVectorFrame.h"
#include <vesta/Debug.h>

using namespace vesta;
using namespace Eigen;


static bool
isNegativeAxis(TwoVectorFrame::Axis a)
{
    return a == TwoVectorFrame::NegativeX ||
           a == TwoVectorFrame::NegativeY ||
           a == TwoVectorFrame::NegativeZ;
}


TwoVectorFrame::TwoVectorFrame(TwoVectorFrameDirection *primary, Axis primaryAxis,
                               TwoVectorFrameDirection *secondary, Axis secondaryAxis) :
    m_primary(primary),
    m_secondary(secondary),
    m_primaryAxis(primaryAxis),
    m_secondaryAxis(secondaryAxis),
    m_valid(false)
{
    if (!primary || !secondary)
    {
        VESTA_LOG("Invalid two vector frame: null direction");
    }
    else if (orthogonalAxes(primaryAxis, secondaryAxis))
    {
        VESTA_LOG("Invalid two vector frame: primary and secondary axes aren't orthogonal");
    }
    else
    {
        m_valid = true;
    }
}


TwoVectorFrame::~TwoVectorFrame()
{
}


/** Return the orientation of the frame at the specified time.
  *
  * This method returns identity when there frame is not defined or
  * degenerate for one of the following reasons:
  *   - One of the directions is null
  *   - The primary and secondary axes are not orthogonal
  *   - The primary or secondary vectors is zero (or very close to zero) at the specified time
  *   - The primary or secondary vectors are either aligned or exactly opposite (or very close
  *     to such a configuration.)
  */
Quaterniond TwoVectorFrame::orientation(double tdbSec) const
{
    if (!m_valid)
    {
        return Quaterniond::Identity();
    }
    else
    {
        Vector3d v0 = m_primary->direction(tdbSec);
        Vector3d v1 = m_secondary->direction(tdbSec);
        if (v0.isZero() || v1.isZero())
        {
            // The primary or secondary vectors are zero at the current time
            return Quaterniond::Identity();
        }

        v0.normalize();
        v1.normalize();
        if (isNegativeAxis(m_primaryAxis))
        {
            v0 = -v0;
        }
        if (isNegativeAxis(m_secondaryAxis))
        {
            v1 = -v1;
        }

        Vector3d v2 = v0.cross(v1);
        if (v2.isZero())
        {
            // Primary and secondary directions are (nearly) collinear and thus
            // don't determine an orientation.
            return Quaterniond::Identity();
        }
        v2.normalize();

        int dir0 = (int) m_primaryAxis;
        int dir1 = (int) m_secondaryAxis;
        int axis0 = dir0 % 3;
        int axis1 = dir1 % 3;
        bool rightHanded = ((axis0 + 1) % 3) == (axis1 % 3);

        // axis2 is whatever axis is not axis0 or axis1
        int axis2 = 3 - (axis0 + axis1);

        Matrix3d m;
        if (rightHanded)
        {
            m.row(axis0) = v0;
            m.row(axis1) = v2.cross(v0);
            m.row(axis2) = v2;
        }
        else
        {
            m.row(axis0) = v0;
            m.row(axis1) = v2.cross(v0);
            m.row(axis2) = -v2;
        }

        return Quaterniond(m);
    }
}


Vector3d TwoVectorFrame::angularVelocity(double tdbSec) const
{
    if (!m_valid)
    {
        return Vector3d::Zero();
    }

    return Vector3d::Zero();
}


/** Return whether two axes are orthogonal to each other.
  */
bool
TwoVectorFrame::orthogonalAxes(TwoVectorFrame::Axis a, TwoVectorFrame::Axis b)
{
    // Note that this calculation depends on the numeric values of
    // the Axis enumeration.
    return (int) a % 3 != (int) b % 3;
}



RelativePositionVector::RelativePositionVector(vesta::Entity* observer, vesta::Entity* target) :
    m_observer(observer),
    m_target(target)
{
}


RelativePositionVector::~RelativePositionVector()
{
}


Vector3d
RelativePositionVector::direction(double tdbSec) const
{
    if (m_observer.isValid() && m_target.isValid())
    {
        return m_target->position(tdbSec) - m_observer->position(tdbSec);
    }
    else
    {
        return Vector3d::Zero();
    }
}


RelativeVelocityVector::RelativeVelocityVector(vesta::Entity* observer, vesta::Entity* target) :
    m_observer(observer),
    m_target(target)
{
}


RelativeVelocityVector::~RelativeVelocityVector()
{
}


Vector3d
RelativeVelocityVector::direction(double tdbSec) const
{
    if (m_observer.isValid() && m_target.isValid())
    {
        return m_target->state(tdbSec).velocity() - m_target->state(tdbSec).position();
    }
    else
    {
        return Vector3d::Zero();
    }
}

