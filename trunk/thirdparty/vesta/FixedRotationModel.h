/*
 * $Revision: 555 $ $Date: 2010-11-10 11:01:25 +0100 (Wed, 10 Nov 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_FIXED_ROTATIONMODEL_H_
#define _VESTA_FIXED_ROTATIONMODEL_H_

#include "RotationModel.h"

namespace vesta
{

class FixedRotationModel : public RotationModel
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    FixedRotationModel(const Eigen::Quaterniond& orientation);

    Eigen::Quaterniond orientation(double t) const;
    Eigen::Vector3d angularVelocity(double t) const;

    void setOrientation(const Eigen::Quaterniond& orientation);

private:
    Eigen::Quaterniond m_orientation;
};

}

#endif // _VESTA_FIXED_ROTATIONMODEL_H_
