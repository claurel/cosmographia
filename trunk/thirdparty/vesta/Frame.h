/*
 * $Revision: 565 $ $Date: 2011-02-15 16:00:43 -0800 (Tue, 15 Feb 2011) $
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

/** StateTransform is a 6x6 matrix used for converting a state vector from
  * one frame to a second frame.
  */
typedef Eigen::Matrix<double, 6, 6> StateTransform;

/** Abstract base class for reference frames. A reference frame in vesta is a
  * simply set of three orthogonal axes that are potentiall rotating.
  * Subclasses must override the orientation() and angularVelocity() methods.
  * The orientation and angular velocity of a frame are reported relative to
  * the inertial International Celestial Reference Frame.
  */
class Frame : public Object
{
public:
    virtual ~Frame() {}

    /** Compute the orientation of the frame with respect to the ICRF at the
      * specified time. The orientation is returned as a unit quaternion. This
      * can be used to transform the vectors in the frame to ICRF. For example,
      * the following code will compute the frame's z direction in the ICRF:
      *
      * \code
      * Vector3d framePos(0.0, 0.0, 1.0);
      * Quaterniond q = frame.orientation(t);
      * Vector3d icrfPos = q * framePos;
      * \endcode
      *
      * \param tsec The time given as the number of seconds since 1 Jan 2000 12:00:00 TDB.
      */
    virtual Eigen::Quaterniond orientation(double tsec) const = 0;

    /** Compute the angular of the frame at the specified time. The units
      * of angular velocity are radians per second.
      *
      * \param tsec The time given as the number of seconds since 1 Jan 2000 12:00:00 TDB.
      */
    virtual Eigen::Vector3d angularVelocity(double tsec) const = 0;

    StateTransform stateTransform(double tsec) const;
    StateTransform inverseStateTransform(double tsec) const;

    static StateTransform stateTransform(const Frame* from, const Frame* to, double tsec);
};

}

#endif // _VESTA_FRAME_H_
