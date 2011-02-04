/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "FixedPointTrajectory.h"

using namespace vesta;
using namespace Eigen;


FixedPointTrajectory::FixedPointTrajectory(const Vector3d& point) :
    m_point(point)
{
}


StateVector
FixedPointTrajectory::state(double /* t */) const
{
    return StateVector(m_point, Vector3d(0.0, 0.0, 0.0));
}


double
FixedPointTrajectory::boundingSphereRadius() const
{
    return m_point.norm();
}
