/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_UNIFORM_ROTATIONMODEL_H_
#define _VESTA_UNIFORM_ROTATIONMODEL_H_

#include "RotationModel.h"

namespace vesta
{

/** UniformRotation is used for rotations of a constant rate about a fixed
  * axis.
  */
class UniformRotationModel : public RotationModel
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    UniformRotationModel(const Eigen::Vector3d& axis,
                         double rotationRate,
                         double meridianAngleAtEpoch,
                         double epoch = 0.0);
    virtual ~UniformRotationModel()
    {
    }

    virtual Eigen::Quaterniond orientation(double t) const;
    virtual Eigen::Vector3d angularVelocity(double t) const;

    /** Get the axis of rotation.
      */
    Eigen::Vector3d axis() const
    {
        return m_axis;
    }

    /** Get the rotation rate in radians per second.
      */
    double rotationRate() const
    {
        return m_rotationRate;
    }

    /** Get the meridian angle in radians at the epoch date.
      */
    double meridianAngleAtEpoch() const
    {
        return m_meridianAngleAtEpoch;
    }

    /** Get the epoch date in seconds since J2000.0
      */
    double epoch() const
    {
        return m_epoch;
    }

private:
    Eigen::Vector3d m_axis;
    double m_rotationRate;
    double m_meridianAngleAtEpoch;
    double m_epoch;
};

}

#endif // _VESTA_UNIFORM_ROTATIONMODEL_H_
