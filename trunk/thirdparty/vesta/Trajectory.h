/*
 * $Revision: 507 $ $Date: 2010-09-15 15:17:27 -0700 (Wed, 15 Sep 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_TRAJECTORY_H_
#define _VESTA_TRAJECTORY_H_

#include "Object.h"
#include "StateVector.h"
#include <Eigen/Core>
#include <limits>


namespace vesta
{

class Trajectory : public Object
{
public:
    Trajectory() :
            m_startTime(-std::numeric_limits<double>::infinity()),
            m_endTime(std::numeric_limits<double>::infinity())
    {}
    virtual ~Trajectory() {}

    /*! Compute the state vector at the specified time. Time is
     *  given as the number of seconds since 1 Jan 2000 12:00:00 UTC.
     *  The returned state vector may not be accurate outside the
     *  valid time range of the trajectory.
     */
    virtual StateVector state(double t) const = 0;

    /*! Return the radius of a sphere centered at the origin that can
     *  contain the entire orbit. This sphere used to avoid calculating
     *  positions of objects that can't possible be visible.
     */
    virtual double boundingSphereRadius() const = 0;

    /*! The default implementation computes the complete state vector
     *  and discards velocity. Subclasses may override this method to
     *  provide a cheaper function for when just the position is requird.
     */
    virtual Eigen::Vector3d position(double t) const
    {
        return state(t).position();
    }

    /*! The default implementation computes the complete state vector
     *  and discards position. Subclasses may override this method to
     *  provide a cheaper function for when just the velocity is requird.
     */
    virtual Eigen::Vector3d velocity(double t) const
    {
        return state(t).velocity();
    }

    /*! Return true if the trajectory is periodic.
     */
    virtual bool isPeriodic() const
    {
        return false;
    }

    /*! Return the period of the trajectory in seconds. If the trajectory
     *  is aperiodic, this method returns zero.
     */
    virtual double period() const
    {
        return 0.0;
    }

    /** Return the start of the valid time range for this trajectory.
      */
    double startTime() const
    {
        return m_startTime;
    }

    /** Set the start of the valid time range for this trajectory.
      */
    void setStartTime(double startTime)
    {
        m_startTime = startTime;
    }

    /** Return the end of the valid time range for this trajectory.
      */
    double endTime() const
    {
        return m_endTime;
    }

    /** Set the start of the valid time range for this trajectory.
      */
    void setEndTime(double endTime)
    {
        m_endTime = endTime;
    }

    /** Set the valid time range for this trajectory. This convenience
      * method is equivalent to calling setStartTime() and setEndTime().
      */
    void setValidTimeRange(double startTime, double endTime)
    {
        m_startTime = startTime;
        m_endTime = endTime;
    }

private:
    double m_startTime;
    double m_endTime;
};

}

#endif // _VESTA_TRAJECTORY_H_
