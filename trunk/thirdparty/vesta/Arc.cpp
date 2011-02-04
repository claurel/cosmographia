/*
 * $Revision: 530 $ $Date: 2010-10-12 11:26:43 -0700 (Tue, 12 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Arc.h"
#include "Entity.h"
#include "InertialFrame.h"
#include "Trajectory.h"
#include "RotationModel.h"
#include "FixedPointTrajectory.h"
#include "FixedRotationModel.h"

using namespace vesta;
using namespace Eigen;


/** Create a default arc:
 *  \list
 *  \li Both the trajectory and body frames are EME J2000
 *  \li The trajectory is a fixed point at the origin.
 *  \li The rotation model is a constant rotation matrix (the identity matrix)
 *  \endlist
 */
Arc::Arc() :
    m_duration(0.0)
{
    m_trajectoryFrame = InertialFrame::equatorJ2000();
    m_bodyFrame = InertialFrame::equatorJ2000();
    m_trajectory = new FixedPointTrajectory(Vector3d::Zero());
    m_rotationModel = new FixedRotationModel(Quaterniond::Identity());
}


Arc::~Arc()
{
}


/** Set the duration of this arc.
 *
 * @param t the duration in units of Julian days (exactly 86400 seconds/JD)
 */
void
Arc::setDuration(double t)
{
    m_duration = t;
}


/** Set the center object for this arc.
  */
void
Arc::setCenter(Entity* center)
{
    m_center = center;
}


/** Set the frame of the trajectory in this arc.
  */
void
Arc::setTrajectoryFrame(Frame* f)
{
    m_trajectoryFrame = f;
}


/** Return the frame of the rotation mode in this arc.
  */
void
Arc::setBodyFrame(Frame* f)
{
    m_bodyFrame = f;
}


/** Set the trajectory for this arc. The trajectory describes the
  * translational motion of a body over the duration of the arc.
  */
void
Arc::setTrajectory(Trajectory* trajectory)
{
    m_trajectory = trajectory;
}


/** Return the rotation model for this arc. The rotation model
  * describes the rotational motion of a body over the duration
  * of the arc.
  */
void
Arc::setRotationModel(RotationModel* rm)
{
    m_rotationModel = rm;
}

