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

#include "SimpleTrajectoryGeometry.h"
#include <vesta/RenderContext.h>
#include <algorithm>
#include <iomanip>

using namespace vesta;
using namespace Eigen;



SimpleTrajectoryGeometry::SimpleTrajectoryGeometry() :
    m_color(Spectrum(1.0f, 1.0f, 1.0f)),
    m_opacity(1.0f),
    m_startTime(0.0),
    m_endTime(0.0),
    m_boundingRadius(0.0),
    m_displayedPortion(TrajectoryGeometry::Entire),
    m_windowDuration(0.0),
    m_windowLead(0.),
    m_fadeFraction(0.0),
    m_lineWidth(1.0f)
{
    setClippingPolicy(SplitToPreventClipping);
}


SimpleTrajectoryGeometry::~SimpleTrajectoryGeometry()
{
}


Eigen::Vector3d
SimpleTrajectoryGeometry::interpolateSamples(double t, double dt, const TrajectorySample& s0, const TrajectorySample& s1) const
{
    // Use cubic Hermite interpolation between trajectory samples; linear interpolation is too inaccurate and
    // results in the end of the trajectory plot being noticeably offset from current position of the object.
    Matrix<double, 3, 4> m;
    m.col(0) = s0.position;
    m.col(1) = s0.velocity * dt;
    m.col(2) = 3.0 * (s1.position - s0.position) - (2.0 * s0.velocity + s1.velocity) * dt;
    m.col(3) = 2.0 * (s0.position - s1.position) + (s1.velocity + s0.velocity) * dt;
    return m * Vector4d(1.0f, t, t * t, t * t * t);
}


void
SimpleTrajectoryGeometry::render(RenderContext& rc, double clock) const
{
    double t0 = firstSampleTime();
    double t1 = lastSampleTime();

    double fadeRate = 0.0;
    double fadeStartTime = 0.0;
    double fadeStartValue = 1.0;

    // Only draw during the appropriate render pass
    if ((rc.pass() == RenderContext::OpaquePass && !isOpaque()) ||
        (rc.pass() == RenderContext::TranslucentPass && isOpaque()))
    {
        return;
    }
    
    if (displayedPortion() == TrajectoryGeometry::WindowBeforeCurrentTime)
    {
        t0 = clock + m_windowLead - m_windowDuration;
        t1 = clock + m_windowLead;

        fadeStartTime = t0;
        fadeStartValue = 0.0;
        double fadeEndTime = fadeStartTime + m_windowDuration * m_fadeFraction;
        fadeRate = 1.0 / (fadeEndTime - fadeStartTime);
    }

    // Nothing to be drawn
    if (t1 <= t0)
    {
        return;
    }

    // Basic opacity of the plot. It may be modified based on three things:
    //   - Approximate size in pixels of the trajectory (small trajectories will fade out)
    //   - Distance from the camera to the 'front' (usually the current position of the oribiting body)
    //   - 'Age' of the trajectory: typically the most recent portions are drawn more opaque than the
    //        older parts. This is handled by setting per-vertex colors.
    float opacity = 0.99f * m_opacity;

    const float sizeFadeStart = 30.0f;
    const float sizeFadeEnd = 15.0f;
    float pixelSize = boundingSphereRadius() / (rc.modelview().translation().norm() * rc.pixelSize());
    if (pixelSize < sizeFadeStart)
    {
        opacity *= std::max(0.0f, (pixelSize - sizeFadeEnd) / (sizeFadeStart - sizeFadeEnd));
    }

    if (opacity <= 0.0f)
    {
        // Complete fade out; no need to draw anything.
        return;
    }

    rc.pushModelView();
    if (m_frame.isValid())
    {
        rc.rotateModelView(m_frame->orientation(clock).cast<float>());
    }

    TrajectoryVertex vertex;
    vertex.color[0] = (unsigned char) (m_color.red() * 255.99f);
    vertex.color[1] = (unsigned char) (m_color.green() * 255.99f);
    vertex.color[2] = (unsigned char) (m_color.blue() * 255.99f);
    vertex.color[3] = 255;

    m_vertexData.clear();

    unsigned int sampleIndex = 0;
    while (sampleIndex < m_samples.size() && t0 > m_samples[sampleIndex].timeTag)
    {
        ++sampleIndex;
    }

    if (sampleIndex > 0 && sampleIndex < m_samples.size())
    {
        double dt = m_samples[sampleIndex].timeTag - m_samples[sampleIndex - 1].timeTag;
        double t = (t0 - m_samples[sampleIndex - 1].timeTag) / dt;
        Vector3d interpolated = interpolateSamples(t, dt, m_samples[sampleIndex - 1], m_samples[sampleIndex]);
        float alpha = std::max(0.0f, std::min(1.0f, float(fadeStartValue + (t0 - fadeStartTime) * fadeRate)));

        vertex.position = interpolated.cast<float>();
        vertex.color[3] = (unsigned char) (alpha * 255.99f);
        m_vertexData.push_back(vertex);
    }

    while (sampleIndex < m_samples.size() && t1 > m_samples[sampleIndex].timeTag)
    {
        float alpha = std::max(0.0f, std::min(1.0f, float(fadeStartValue + (m_samples[sampleIndex].timeTag - fadeStartTime) * fadeRate)));

        vertex.position = m_samples[sampleIndex].position.cast<float>();
        vertex.color[3] = (unsigned char) (alpha * 255.99f);
        m_vertexData.push_back(vertex);

        ++sampleIndex;
    }

    if (sampleIndex > 0 && sampleIndex < m_samples.size())
    {
        double dt = m_samples[sampleIndex].timeTag - m_samples[sampleIndex - 1].timeTag;
        double t = (t1 - m_samples[sampleIndex - 1].timeTag) / dt;
        Vector3d interpolated = interpolateSamples(t, dt, m_samples[sampleIndex - 1], m_samples[sampleIndex]);
        float alpha = std::max(0.0f, std::min(1.0f, float(fadeStartValue + (t1 - fadeStartTime) * fadeRate)));

        vertex.position = interpolated.cast<float>();
        vertex.color[3] = (unsigned char) (alpha * 255.99f);
        m_vertexData.push_back(vertex);
    }

    // Fade trajectory based on size
    Vector3f frontPosition = rc.modelview() * vertex.position;
    float frontDistance = frontPosition.norm();

    // Fade trajectory based on distance to front point. This is helpful because the simple trajectory model
    // is not precise, and fading hides the discrepancy between the plot and the body's current position.
    const float fadeStart = 0.04f;
    const float fadeFinish = 0.01f;
    if (frontDistance < fadeStart * boundingSphereRadius())
    {
        opacity *= std::max(0.0f, (frontDistance / boundingSphereRadius() - fadeFinish) / (fadeStart - fadeFinish));
    }

    Material material;
    material.setDiffuse(Spectrum::White());
    material.setOpacity(opacity);
    rc.bindMaterial(&material);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_vertexData.size() > 1 && opacity > 0.0f)
    {
        rc.bindVertexArray(VertexSpec::PositionColor, m_vertexData[0].position.data(), sizeof(TrajectoryVertex));
        rc.drawPrimitives(PrimitiveBatch(PrimitiveBatch::LineStrip, m_vertexData.size() - 1, 0));
        rc.unbindVertexArray();
    }

    glDisable(GL_BLEND);

    rc.popModelView();
}


void
SimpleTrajectoryGeometry::addSample(double t, const vesta::StateVector& s)
{
    TrajectorySample sample;
    sample.position = s.position();
    sample.velocity = s.velocity();
    sample.timeTag = t;

    if (m_samples.empty())
    {
        m_startTime = t;
        m_endTime = t;
        m_samples.push_back(sample);
    }
    else if (t > m_samples.back().timeTag)
    {
        m_samples.push_back(sample);
    }
    else if (sample.timeTag < m_samples.front().timeTag)
    {
        m_samples.push_front(sample);
    }
    else
    {
        // Can't add sample in middle
        return;
    }

    m_boundingRadius = std::max(m_boundingRadius, sample.position.norm());
}


void
SimpleTrajectoryGeometry::clearSamples()
{
    m_boundingRadius = 0.0;
    m_samples.clear();
    m_startTime = 0.0;
    m_endTime = 0.0;
}


void
SimpleTrajectoryGeometry::computeSamples(const vesta::TrajectoryPlotGenerator* generator, double startTime, double endTime, unsigned int stepCount)
{
    clearSamples();
    double t0 = std::max(generator->startTime(), startTime);
    double t1 = std::min(generator->endTime(), endTime);
    double dt = t1 - t0;
    if (dt <= 0.0 || stepCount == 0)
    {
        return;
    }

    double invStep = 1.0 / double(stepCount);

    for (unsigned int i = 0; i <= stepCount; ++i)
    {
        double t = t0 + dt * (i * invStep);
        addSample(t, generator->state(t));
    }
}


void
SimpleTrajectoryGeometry::updateSamples(const vesta::TrajectoryPlotGenerator* generator, double startTime, double endTime, unsigned int stepCount)
{
    double dt = endTime - startTime;
    if (dt <= 0.0 || stepCount == 0)
    {
        return;
    }

    double stepTime = dt / std::max(1u, stepCount - 1);
    double t0 = std::max(generator->startTime(), startTime - stepTime);
    double t1 = std::min(generator->endTime(), endTime + stepTime);

    // We need to recompute everything when there are no samples already or when the time range is
    // disjoint with the current sampled time range.
    if (m_samples.empty() || timeRangeDisjointWithSampleTimeRange(t0, t1))
    {
        computeSamples(generator, t0, t1, stepCount);
    }
    else
    {
        // Add samples at beginning
        if (t0 < firstSampleTime())
        {
            for (double t = firstSampleTime() - stepTime; t > t0; t -= stepTime)
            {
                t = std::max(t, t0);
                addSample(t, generator->state(t));
            }
        }

        // Add samples at end
        if (t1 > lastSampleTime())
        {
            for (double t = lastSampleTime() + stepTime; t < t1; t += stepTime)
            {
                t = std::min(t, t1);
                addSample(t, generator->state(t));
            }
        }

        removeSamplesBeforeTime(t0);
        removeSamplesAfterTime(t1);
    }

    m_startTime = t0;
    m_endTime = t1;
}


void
SimpleTrajectoryGeometry::removeSamplesBeforeTime(double t)
{
    while (!m_samples.empty() && m_samples.front().timeTag < t)
    {
        m_samples.pop_front();
    }
}


void
SimpleTrajectoryGeometry::removeSamplesAfterTime(double t)
{
    while (!m_samples.empty() && m_samples.back().timeTag > t)
    {
        m_samples.pop_back();
    }
}


double
SimpleTrajectoryGeometry::firstSampleTime() const
{
    return m_samples.empty() ? 0 : m_samples.front().timeTag;
}


double
SimpleTrajectoryGeometry::lastSampleTime() const
{
    return m_samples.empty() ? 0 : m_samples.back().timeTag;
}


bool
SimpleTrajectoryGeometry::timeRangeDisjointWithSampleTimeRange(double startTime, double endTime) const
{
     return endTime <= firstSampleTime() || startTime >= lastSampleTime();
}
