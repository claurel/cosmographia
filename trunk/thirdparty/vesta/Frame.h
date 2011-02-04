/*
 * $Revision: 523 $ $Date: 2010-10-07 10:06:21 -0700 (Thu, 07 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_FRAME_H_
#define _VESTA_FRAME_H_

#include "Object.h"
#include <Eigen/Core>
#include <Eigen/Geometry>


namespace vesta
{

typedef Eigen::Matrix<double, 6, 6> StateTransform;

/** Abstract base class for reference frames. A reference frame in vesta is a
  * simply set of three orthogonal axes. Subclasses must override the orientation()
  * and angularVelocity() methods.
  */
class Frame : public Object
{
public:
    virtual ~Frame() {}

    /** Compute the orientation of the frame at the specified time.
      * @param tsec The time given as the number of seconds since 1 Jan 2000 12:00:00 TDB.
      */
    virtual Eigen::Quaterniond orientation(double tsec) const = 0;

    /** Compute the angular of the frame at the specified time. The units
      * of angular velocity are radians per second.
      * @param tsec The time given as the number of seconds since 1 Jan 2000 12:00:00 TDB.
      */
    virtual Eigen::Vector3d angularVelocity(double tsec) const = 0;

    StateTransform stateTransform(double tsec);

    StateTransform inverseStateTransform(double tsec);
};

}

#endif // _VESTA_FRAME_H_
