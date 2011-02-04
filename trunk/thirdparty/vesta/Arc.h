/*
 * $Revision: 530 $ $Date: 2010-10-12 11:26:43 -0700 (Tue, 12 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ARC_H_
#define _VESTA_ARC_H_

#include "Object.h"
#include <Eigen/Core>

namespace vesta
{

class Entity;
class Frame;
class Trajectory;
class RotationModel;

/** An Arc is one segment of a Chronology. Within an Arc,
 *  a single trajectory expresses translational motion relative
 *  to the center object and in the trajectory frame. Similarly,
 *  rotational motion is described by a rotation model object.
 *  The frame for rotational motion is given by the Arc's body frame,
 *  which can be disctinct from the trajectory frame.
 */
class Arc : public Object
{
public:
    Arc();
    ~Arc();

    /** Return the center object for this arc.
      */
    Entity* center() const
    {
        return m_center.ptr();
    }

    void setCenter(Entity* center);

    /** Return the frame of the trajectory in this arc.
      */
    Frame* trajectoryFrame() const
    {
        return m_trajectoryFrame.ptr();
    }

    void setTrajectoryFrame(Frame* f);

    /** Return the frame of the rotation mode in this arc.
      */
    Frame* bodyFrame() const
    {
        return m_bodyFrame.ptr();
    }

    void setBodyFrame(Frame* f);

    /** Return the trajectory for this arc. The trajectory describes the
      * translational motion of a body over the duration of the arc.
      */
    Trajectory* trajectory() const
    {
        return m_trajectory.ptr();
    }

    void setTrajectory(Trajectory* trajectory);

    /** Return the rotation model for this arc. The rotation model
      * describes the rotational motion of a body over the duration
      * of the arc.
      */
    RotationModel* rotationModel() const
    {
        return m_rotationModel.ptr();
    }

    void setRotationModel(RotationModel* rm);

    /** Get the duration of the arc in seconds.
      */
    double duration() const
    {
        return m_duration;
    }

    void setDuration(double t);

private:
    counted_ptr<Entity> m_center;
    counted_ptr<Frame> m_trajectoryFrame;
    counted_ptr<Frame> m_bodyFrame;
    counted_ptr<Trajectory> m_trajectory;
    counted_ptr<RotationModel> m_rotationModel;
    double m_duration;
};

} // namespace

#endif // _VESTA_ARC_H_
