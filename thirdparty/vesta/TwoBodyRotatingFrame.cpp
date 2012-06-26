/*
 * $Revision: 657 $ $Date: 2012-03-23 01:01:30 -0700 (Fri, 23 Mar 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "TwoBodyRotatingFrame.h"

using namespace vesta;
using namespace Eigen;


/** Construct a new two-body rotating frame with a primary
  * (central) and secondary (orbiting) body.
  */
TwoBodyRotatingFrame::TwoBodyRotatingFrame(Entity* primary, Entity* secondary) :
    m_primary(primary),
    m_secondary(secondary)
{
        m_velocityAlligned = true;
}


TwoBodyRotatingFrame::~TwoBodyRotatingFrame()
{
}


/** Get the orientation of the frame at the specified time. The frame's orientation
  * is undefined whenever one or more of the following is true:
  *  - the state of either the primary or secondary object is undefined
  *  - the positions of the primary and secondary object are identical
  *  - the secondary is stationary with respect to the primary
  *  - the position and velocity vectors are exactly aligned
  */
Quaterniond
TwoBodyRotatingFrame::orientation(double t) const
{
    StateVector state = m_secondary->state(t) - m_primary->state(t);
    if (state.position().isZero() || state.velocity().isZero())
    {
        return Quaterniond::Identity();
    }

    // Compute the axes of the two body rotating frame,
    // convert this to a matrix, then derive a quaternion
    // from this matrix.

    // x-axis points in direction from the primary to the secondary
    Vector3d xAxis = state.position().normalized();
    Vector3d v = state.velocity().normalized();

    Vector3d zAxis;
    if (m_velocityAlligned) {
        // z-axis normal to both the x-axis and the velocity vector
        zAxis = xAxis.cross(v);
        if (zAxis.isZero())
        {
            return Quaterniond::Identity();
        }
    }
    else {
        // z-axis normal to both the x-axis and the z axis of the primary
        zAxis = xAxis.cross(m_primary->orientation(t) * Vector3d::UnitX());
        if (zAxis.isZero())
        {
            return Quaterniond::Identity();
        }
    }

    zAxis.normalize();
    Vector3d yAxis = zAxis.cross(xAxis);

    Matrix3d m;
    m << xAxis, yAxis, zAxis;

    return Quaterniond(m);
}


/** Get the angular velocity of the two-body rotating frame. It
  * is undefined whenever:
  *  - the state of either the primary or secondary object is undefined
  *  - the positions of the primary and secondary object are identical
  */
Vector3d
TwoBodyRotatingFrame::angularVelocity(double t) const
{
    StateVector state = m_secondary->state(t) - m_primary->state(t);
    if (state.position().isZero())
    {
        return Vector3d::Zero();
    }

    return state.position().cross(state.velocity()) / state.position().squaredNorm();
}
