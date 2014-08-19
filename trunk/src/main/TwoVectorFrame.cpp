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

#include "TwoVectorFrame.h"
#include <vesta/Debug.h>
#include <Eigen/LU>

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
    else if (!orthogonalAxes(primaryAxis, secondaryAxis))
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
        m.col(axis0) = v0;
        m.col(axis1) = v2.cross(v0);
        if (rightHanded)
        {
            m.col(axis2) = v2;
        }
        else
        {
            m.col(axis2) = -v2;
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
        return m_target->state(tdbSec).velocity() - m_observer->state(tdbSec).velocity();
    }
    else
    {
        return Vector3d::Zero();
    }
}


/** Create a new constant frame direction.
  *
  * \param frame the frame in which the vector is fixed
  * \param a direction vector (must be normalized)
  */
ConstantFrameDirection::ConstantFrameDirection(vesta::Frame *frame, const Eigen::Vector3d &vector) :
    m_frame(frame),
    m_vector(vector)
{
}


ConstantFrameDirection::~ConstantFrameDirection()
{
}


Vector3d
ConstantFrameDirection::direction(double tdbSec) const
{
    if (m_frame.isValid())
    {
        return m_frame->orientation(tdbSec) * m_vector;
    }
    else
    {
        return Vector3d::Zero();
    }
}
