/*
 * $Revision: 554 $ $Date: 2010-11-09 17:27:59 +0100 (Tue, 09 Nov 2010) $
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


/** Create a new FixedPointTrajectory for the specified point.
  */
FixedPointTrajectory::FixedPointTrajectory(const Vector3d& point) :
    m_state(StateVector(point, Vector3d::Zero()))
{
}


/** Create a new FixedPointTrajectory with the specified state.
  */
FixedPointTrajectory::FixedPointTrajectory(const StateVector& state) :
     m_state(state)
{
}


StateVector
FixedPointTrajectory::state(double /* t */) const
{
    return m_state;
}


double
FixedPointTrajectory::boundingSphereRadius() const
{
    return m_state.position().norm();
}


/** Change the state vector of this fixed point trajectory.
  */
void
FixedPointTrajectory::setState(const StateVector& state)
{
    m_state = state;
}
