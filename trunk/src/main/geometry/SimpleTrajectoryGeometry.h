// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

