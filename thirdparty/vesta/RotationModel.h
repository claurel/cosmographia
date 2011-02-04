/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ROTATIONMODEL_H_
#define _VESTA_ROTATIONMODEL_H_

#include "Object.h"
#include <Eigen/Core>
#include <Eigen/Geometry>


namespace vesta
{

class RotationModel : public Object
{
public:
    RotationModel() {}
    virtual ~RotationModel() {}

    /*! Compute the orientation at the specified time.
     *  @param t the number of seconds since 1 Jan 2000 12:00:00 UTC.
     */
    virtual Eigen::Quaterniond orientation(double t) const = 0;

    /*! Compute the angular velocity at the specified time. The units
     *  of angular velocity are radians per second.
     *  @param t the number of seconds since 1 Jan 2000 12:00:00 UTC.
     */
    virtual Eigen::Vector3d angularVelocity(double t) const = 0;
};

}

#endif // _VESTA_ROTATIONMODEL_H_
