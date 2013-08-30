// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#ifndef _SIMPLE_TRAJECTORY_GEOMETRY_H_
#define _SIMPLE_TRAJECTORY_GEOMETRY_H_

#include <vesta/TrajectoryGeometry.h>
#include <Eigen/Core>
#include <deque>
#include <vector>


class SimpleTrajectoryGeometry : public vesta::Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    SimpleTrajectoryGeometry();
    virtual ~SimpleTrajectoryGeometry();

    void render(vesta::RenderContext& rc, double clock) const;

    float boundingSphereRadius() const
    {
        return m_boundingRadius * 1.05f;
    }

    virtual bool isOpaque() const
    {
        return m_opacity >= 1.0f;
    }

    vesta::Frame* frame() const
    {
        return m_frame.ptr();
    }

    void setFrame(vesta::Frame* frame)
    {
        m_frame = frame;
    }

    vesta::Spectrum color() const
    {
        return m_color;
    }

    void setColor(const vesta::Spectrum& color)
    {
        m_color = color;
    }

    float opacity() const
    {
        return m_opacity;
    }

    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    void addSample(double t, const vesta::StateVector& s);
    void clearSamples();
    void computeSamples(const vesta::TrajectoryPlotGenerator* generator, double startTime, double endTime, unsigned int stepCount);
    void updateSamples(const vesta::TrajectoryPlotGenerator* generator, double startTime, double endTime, unsigned int stepCount);

    vesta::TrajectoryGeometry::TrajectoryPortion displayedPortion() const
    {
        return m_displayedPortion;
    }

    void setDisplayedPortion(vesta::TrajectoryGeometry::TrajectoryPortion portion)
    {
        m_displayedPortion = portion;
    }

    double windowDuration() const
    {
        return m_windowDuration;
    }

    void setWindowDuration(double duration)
    {
        m_windowDuration = duration;
    }

    double windowLead() const
    {
        return m_windowLead;
    }

    void setWindowLead(double duration)
    {
        m_windowLead = duration;
    }

    double fadeFraction() const
    {
        return m_fadeFraction;
    }

    void setFadeFraction(double fadeFraction)
    {
        m_fadeFraction = fadeFraction;
    }

    float lineWidth() const
    {
        return m_lineWidth;
    }

    void setLineWidth(float width)
    {
        m_lineWidth = width;
    }

private:
    struct TrajectorySample
    {
        Eigen::Vector3d position;
        Eigen::Vector3d velocity;
        double timeTag;
    };

    struct TrajectoryVertex
    {
        Eigen::Vector3f position;
        unsigned char color[4];
    };

    void removeSamplesBeforeTime(double t);
    void removeSamplesAfterTime(double t);
    double firstSampleTime() const;
    double lastSampleTime() const;
    bool timeRangeDisjointWithSampleTimeRange(double startTime, double endTime) const;
    Eigen::Vector3d interpolateSamples(double t, double dt, const TrajectorySample& s0, const TrajectorySample& s1) const;

private:
    vesta::counted_ptr<vesta::Frame> m_frame;
    vesta::Spectrum m_color;
    float m_opacity;
    double m_startTime;
    double m_endTime;
    double m_boundingRadius;
    vesta::TrajectoryGeometry::TrajectoryPortion m_displayedPortion;
    double m_windowDuration;
    double m_windowLead;
    double m_fadeFraction;
    float m_lineWidth;

    std::deque<TrajectorySample> m_samples;
    mutable std::vector<TrajectoryVertex> m_vertexData;
};

#endif // _SIMPLE_TRAJECTORY_GEOMETRY_H_

