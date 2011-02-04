/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "FixedRotationModel.h"

using namespace vesta;
using namespace Eigen;


FixedRotationModel::FixedRotationModel(const Quaterniond& orientation) :
    m_orientation(orientation)
{
}


Quaterniond
FixedRotationModel::orientation(double /* t */) const
{
    return m_orientation;
}


Vector3d
FixedRotationModel::angularVelocity(double /* t */) const
{
    return Vector3d::Zero();
}

