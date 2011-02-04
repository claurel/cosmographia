/*
 * $Revision: 475 $ $Date: 2010-08-31 08:09:34 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */
#ifndef _VESTA_OBSERVER_H_
#define _VESTA_OBSERVER_H_

#include "Entity.h"
#include "Frame.h"


namespace vesta
{

/** An Observer has a position and orientation. The position is relative to a
  * center object in the position frame.
  */
class Observer : public Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Observer(Entity* center);
    virtual ~Observer();

    Entity* center() const
    {
        return m_center.ptr();
    }

    void setCenter(Entity* center);
    void updateCenter(Entity* center, double t);

    /** Return the frame for the position.
      */
    Frame* positionFrame() const
    {
        return m_positionFrame.ptr();
    }

    void setPositionFrame(Frame* f);
    void updatePositionFrame(Frame* positionFrame, double t);

    /** Return the frame for the observer's orientation.
      */
    Frame* pointingFrame() const
    {
        return m_pointingFrame.ptr();
    }

    void setPointingFrame(Frame* f);
    void updatePointingFrame(Frame* pointingFrame, double t);

    /** Get the position of the observer with respect to the center, in the
      * observer's position frame.
      */
    Eigen::Vector3d position() const
    {
        return m_position;
    }

    void setPosition(const Eigen::Vector3d& position);

    /** Get the orientation of the observer in the observer's pointing frame.
      */
    Eigen::Quaterniond orientation() const
    {
        return m_orientation;
    }

    void setOrientation(const Eigen::Quaterniond& orientation);

    void rotate(const Eigen::Quaterniond& rotation);
    void orbit(const Eigen::Quaterniond& rotation);
    void changeDistance(double factor);

    Eigen::Vector3d absolutePosition(double t) const;

    Eigen::Quaterniond absoluteOrientation(double t) const;

private:
    counted_ptr<Entity> m_center;
    counted_ptr<Frame> m_positionFrame;
    counted_ptr<Frame> m_pointingFrame;
    Eigen::Vector3d m_position;
    Eigen::Quaterniond m_orientation;
};

}

#endif // _VESTA_OBSERVER_H_
