/*
 * $Revision: 657 $ $Date: 2012-03-23 01:01:30 -0700 (Fri, 23 Mar 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_TWO_BODY_ROTATING_FRAME_H_
#define _VESTA_TWO_BODY_ROTATING_FRAME_H_

#include "Frame.h"
#include "Entity.h"


namespace vesta
{

/** A two-body rotating frame has the following axes:
  * \list
  * \li +x points from the primary to the secondary body
  * \li +z is normal to both the velocity and x-axis
  * \li +y points in the direction of the velocity of the
  *    secondary relative to the primary, and is
  *    orthogonal to both the x and z axes. Note that y
  *    will not in general point precisely in the direction
  *    of the velocity.
  * \endlist
  *
  * +z is chosen so that the coordinate system is right-handed.
  *
  */
class TwoBodyRotatingFrame : public Frame
{
public:
    TwoBodyRotatingFrame(Entity* primary, Entity* secondary);
    virtual ~TwoBodyRotatingFrame();

    virtual Eigen::Quaterniond orientation(double t) const;
    virtual Eigen::Vector3d angularVelocity(double t) const;

    /** Get the central object of the two-body frame. */
    Entity* primary() const
    {
        return m_primary.ptr();
    }

    /** Get the secondary object of the two-body frame. */
    Entity* secondary() const
    {
        return m_secondary.ptr();
    }

    void setVelocityAlligned(bool v) {
        m_velocityAlligned = v;
    }

private:
    counted_ptr<Entity> m_primary;
    counted_ptr<Entity> m_secondary;
    bool m_velocityAlligned;
};

}

#endif // _VESTA_TWO_BODY_ROTATING_FRAME_H_
