/*
 * $Revision: 684 $ $Date: 2012-07-16 21:12:16 -0700 (Mon, 16 Jul 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_TRAJECTORY_GEOMETRY_H_
#define _VESTA_TRAJECTORY_GEOMETRY_H_

#include "Geometry.h"
#include "Spectrum.h"
#include "Frame.h"
#include <Eigen/Core>

class CurvePlot;

namespace vesta
{

class Trajectory;


class TrajectoryPlotGenerator
{
public:
    virtual StateVector state(double tsec) const = 0;
    virtual double startTime() const = 0;
    virtual double endTime() const = 0;
};


/** TrajectoryGeometry is used for plotting the paths of bodies through
  * space. It provides flexibility in how the plots are drawn. Depending on
  * settings, an entire trajectory can be shown or just a portion of it.
  *
  * When a new trajectory, it is empty. Points may be added to the trajectory
  * one-by-one useing the addSample() method or automatically via computedSamples()
  * and updateSamples(). A 'sample' is a time tagged state vectory. Cubic interpolation
  * is used to generate intermediate points, so there will never be any 'kinks' in
  * the plot.
  */
class TrajectoryGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    TrajectoryGeometry();
    virtual ~TrajectoryGeometry();

    void render(RenderContext& rc, double clock) const;

    float boundingSphereRadius() const
    {
        return static_cast<float>(m_boundingRadius);
    }

    virtual bool isOpaque() const
    {
        return m_opacity >= 1.0f;
    }

    /** Get the reference frame for this trajectory plot.
      */
    Frame* frame() const
    {
        return m_frame.ptr();
    }

    /** Set the reference frame for this trajectory plot. If not set, the
      * inertial International Celestial Reference Frame (the native frame
      * of VESTA) is used.
      */
    void setFrame(Frame* frame)
    {
        m_frame = frame;
    }

    /** Return the color used for the trajectory plot.
      */
    Spectrum color() const
    {
        return m_color;
    }

    /** Set the color used for the trajectory plot. By default,
      * trajectories are plotted in white.
      */
    void setColor(const Spectrum& color)
    {
        m_color = color;
    }

    /** Get the opacity (0 = completely transparent, 1 = opaque) of
      * the arrow geometry.
      */
    float opacity() const
    {
        return m_opacity;
    }

    /** Set opacity of the arrows (0 = completely transparent, 1 = opaque).
      */
    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    void addSample(double t, const StateVector& s);
    void clearSamples();
    void computeSamples(const Trajectory* trajectory, double startTime, double endTime, unsigned int steps);
    void updateSamples(const Trajectory* trajectory, double startTime, double endTime, unsigned int steps);
    void computeSamples(const TrajectoryPlotGenerator* generator, double startTime, double endTime, unsigned int steps);
    void updateSamples(const TrajectoryPlotGenerator* generator, double startTime, double endTime, unsigned int steps);

    enum TrajectoryPortion
    {
        Entire                  = 0,
        StartToCurrentTime      = 1,
        CurrentTimeToEnd        = 2,
        WindowBeforeCurrentTime = 3,
    };

    /** Return the portion of the trajectory that will be displayed.
      */
    TrajectoryPortion displayedPortion() const
    {
        return m_displayedPortion;
    }

    /** Set the portion of the trajectory that will be displayed. There are three
      * options:
      *    Entire - show the complete trajectory from beginning to end
      *    StartToCurrentTime - show the trajectory from the first point through the current time
      *    CurrenTimeToEnd - show the trajectory from the current time through the end point
      *    WindowBeforeCurrentTime - show the trajectory over the span [ currentTime - windowDuration + windowLead, currentTime + windowLead ]
      *
      * In order to use WindowBeforeCurrentTime, the window duration must be set to an appropriate
      * value. The default is 0, so the trajectory won't be shown at all without calling setWindowDuration()
      * to a non-zero value. To plot the trajectory at times ahead of the
      * current time, the windowLead can be set to a positive value.
      */
    void setDisplayedPortion(TrajectoryPortion portion)
    {
        m_displayedPortion = portion;
    }

    /** Get the window of time over which the trajectory is shown. This value is
      * only used when the displayed portion is set to WindowBeforeCurrentTime.
      *
      * \returns the window duration in seconds
      */
    double windowDuration() const
    {
        return m_windowDuration;
    }

    /** Set the window of time over which the trajectory is shown. This value is
      * only used when the displayed portion is set to WindowBeforeCurrentTime.
      *
      * \param duration the window duration in seconds
      */
    void setWindowDuration(double duration)
    {
        m_windowDuration = duration;
    }

    /** Get the window lead. At a given time t, the time interval that will
      * be plotted is [ t - duration + lead, t + lead ]. The window lead is
      * only used when the displayed portion is set to WindowBeforeCurrentTime.
      *
      * \returns the window lead in seconds
      */
    double windowLead() const
    {
        return m_windowLead;
    }

    /** Get the window lead. At a given time t, the time interval that will
      * be plotted is [ t - duration + lead, t + lead ]. The window lead is
      * only used when the displayed portion is set to WindowBeforeCurrentTime.
      *
      * \returns the window lead in seconds
      */
    void setWindowLead(double duration)
    {
        m_windowLead = duration;
    }

    /** Get the fraction of the window duration over which the trajectory plot
      * fades to transparent.
      */
    double fadeFraction() const
    {
        return m_fadeFraction;
    }

    /** Set the fraction of the window duration over which the trajectory plot
      * fades to transparent. Setting it to zero (which is the initial value)
      * disables fading completely. Fading is only used when the displayed portion
      * of the orbit is a fixed time window (e.g. WindowBeforeCurrentTime).
      *
      * Example: the following code sets the TrajectoryGeometry to plot a
      * trajectory for one orbital period over a time range ending at the
      * current simulation time. The plot is opaque except for the earliest
      * 25%, where it fades linearly to completely transparent.
      *
      * \code
      * Trajectory* traj = ...;
      * plot->setWindowDuration(traj->period());
      * plot->setDisplayedPortion(TrajectoryGeometry::WindowBeforeCurrentTime);
      * plot->setFadeFraction(0.25);
      * \endcode
      */
    void setFadeFraction(double fadeFraction)
    {
        m_fadeFraction = fadeFraction;
    }


    /** Get the width of the lines used to plot the trajectory.
      */
    float lineWidth() const
    {
        return m_lineWidth;
    }

    /** Set the width of the lines used to plot the trajectory. By default,
      * lines of width 1 are used.
      */
    void setLineWidth(float width)
    {
        m_lineWidth = width;
    }

private:
    counted_ptr<Frame> m_frame;
    Spectrum m_color;
    float m_opacity;
    CurvePlot* m_curvePlot;
    double m_startTime;
    double m_endTime;
    double m_boundingRadius;
    TrajectoryPortion m_displayedPortion;
    double m_windowDuration;
    double m_windowLead;
    double m_fadeFraction;
    float m_lineWidth;
};

}

#endif // _VESTA_TRAJECTORY_GEOMETRY_H_

