// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "MarkerLayer.h"
#include <vesta/Units.h>
#include <vesta/Geometry.h>
#include <QGLWidget>
#include <QDebug>
#include <algorithm>
#include <cmath>

using namespace vesta;
using namespace Eigen;


MarkerLayer::MarkerLayer()
{
}


MarkerLayer::~MarkerLayer()
{
}


void
MarkerLayer::addMarker(vesta::Entity* body,
                       const vesta::Spectrum& color,
                       float size,
                       Marker::Style style,
                       double startTime,
                       double lifeTime)
{
    if (!body)
    {
        return;
    }

    Marker* marker = new Marker();
    marker->setBody(body);
    marker->setColor(color);
    marker->setSize(size);
    marker->setStyle(style);
    marker->setStartTime(startTime);
    marker->setLifeTime(lifeTime);

    m_markers << marker;
}


static void drawFadeArc(float x, float y, float theta0, float theta1, float radius, const Spectrum& color)
{
    unsigned int segments = 15;

    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i <= segments; ++i)
    {
        float t = float(i) / float(segments);
        float theta = theta0 + (theta1 - theta0) * t;
        glColor4f(color.red(), color.green(), color.blue(), t);
        glVertex2f(x + cos(theta) * radius, y + sin(theta) * radius);
    }
    glEnd();
}


static void drawOutOfViewPointer(float x, float y, const Vector2f& direction, const Spectrum& color, double realTime)
{
    float size = 20.0f;
    float angle = float(toRadians(20.0));
    float c = cos(angle);
    float s = sin(angle);

    Vector2f u = direction * size;
    Vector2f v(-u.y(), u.x());

    Vector2f p0(x, y);

    // Apply a periodic motion to improve the visibility of the pointer
    p0 += direction * ((sin(realTime * 2.5) + 1.0) * size * 0.3f);
    Vector2f p1 = p0 + c * u + s * v;
    Vector2f p2 = p0 + c * u - s * v;

    // Draw a translucent triangle with an outline
    glLineWidth(2.0f);
    glColor4f(color.red(), color.green(), color.blue(), 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex2fv(p2.data());
    glVertex2fv(p1.data());
    glVertex2fv(p0.data());
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2fv(p2.data());
    glVertex2fv(p1.data());
    glVertex2fv(p0.data());
    glEnd();
    glLineWidth(1.0f);
}


void
MarkerLayer::renderMarker(const Vector3d& viewerPosition,
                          const Quaterniond& viewerOrientation,
                          const PlanarProjection& projection,
                          const Viewport& viewport,
                          double simulationTime,
                          double realTime,
                          const Marker* marker)
{
    Entity* body = marker->body();
    Vector3d position = body->position(simulationTime);
    Vector3d cameraRelativePosition = position - viewerPosition;
    Vector3d cameraSpacePosition = viewerOrientation.conjugate() * cameraRelativePosition;

    // Project the location of the body into normalized device coordinates
    Vector4d p;
    p.start<3>() = cameraSpacePosition;
    p.w() = 1.0;
    Vector4f ndc = projection.matrix() * p.cast<float>();
    bool showOutOfViewIndicator = false;

    if (ndc.w() < 0 && !marker->directionIndicatorEnabled())
    {
        // Target is behind the camera, and we're not rendering the out-of-view pointer, so
        // there's nothing to do.
        return;
    }

    bool targetOutsideView = ndc.w() < 0 || std::abs(ndc.x()) > std::abs(ndc.w()) || std::abs(ndc.y()) > std::abs(ndc.w());

    // Viewport coordinates
    float vx = 0.0f;
    float vy = 0.0f;

    if (targetOutsideView && marker->directionIndicatorEnabled())
    {
        // Clip the screen position of the target to the outer edge of the viewport
        if (std::abs(ndc.x()) > std::abs(ndc.y()))
        {
            if (ndc.x() < 0.0f)
            {
                vx = -1.0f;
                vy = -ndc.y() / ndc.x();
            }
            else
            {
                vx = 1.0f;
                vy = ndc.y() / ndc.x();
            }
        }
        else
        {
            if (ndc.y() < 0.0f)
            {
                vy = -1.0f;
                vx = -ndc.x() / ndc.y();
            }
            else
            {
                vy = 1.0f;
                vx = ndc.x() / ndc.y();
            }
        }

        showOutOfViewIndicator = true;
    }
    else
    {
        // Project the point
        vx = ndc.x() / ndc.w();
        vy = ndc.y() / ndc.w();
    }

    // Compute the position in viewport coordinates
    float x = (vx * 0.5f + 0.5f) * viewport.width();
    float y = (vy * 0.5f + 0.5f) * viewport.height();

    Spectrum color = marker->color();
    if (marker->targetSizeThreshold() > 0.0f && !showOutOfViewIndicator)
    {
        // If there's a target size threshold set, we need to compute the projected size in
        // pixels of the target. If it exceeds the threshold, fade out the marker.
        double targetDistance = cameraRelativePosition.norm();
        double targetSize = 0.0;
        if (body->geometry())
        {
            targetSize = body->geometry()->boundingSphereRadius();
        }

        float tanFov = projection.top() / projection.nearDistance();
        double pixelSize = 2.0 * tanFov / viewport.height();
        double targetSizePix = targetSize / (pixelSize * targetDistance);
        float opacity = float(std::min(1.0, 1.0 - (targetSizePix - marker->targetSizeThreshold()) / marker->targetSizeThreshold()));
        if (opacity <= 0.0f)
        {
            return;
        }

        color = color * opacity;
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    if (showOutOfViewIndicator)
    {
        drawOutOfViewPointer(x, y, -Vector2f(vx, vy).normalized(), color, realTime);
    }
    else
    {
        glLineWidth(2.0f);

        switch (marker->style())
        {
        case Marker::Spin:
            {
                double period = 5.0;
                float theta = (fmod(realTime, period) / period * vesta::PI * 2.0);
                float arcLength = float(vesta::PI * 2.0 / 3.0);
                drawFadeArc(x, y, theta, theta + arcLength, marker->size(), color);
                drawFadeArc(x, y, theta + arcLength, theta + arcLength * 2, marker->size(), color);
                drawFadeArc(x, y, theta + arcLength * 2, theta + arcLength * 3, marker->size(), color);
            }
            break;

        case Marker::Pulse:
        default:
            {
                const unsigned int segments = 24;
                float t = float(std::fmod(realTime - marker->startTime(), marker->lifeTime()) / marker->lifeTime());
                float r = marker->size() * t;
                glColor4f(color.red(), color.green(), color.blue(), 1.0f - t);
                glBegin(GL_LINE_LOOP);
                for (unsigned int i = 0; i < segments; ++i)
                {
                    float theta = float(i * vesta::PI * 2.0) / segments;
                    glVertex2f(x + cos(theta) * r, y + sin(theta) * r);
                }
                glEnd();
            }
            break;
        }

        glLineWidth(1.0f);
    }
}


void
MarkerLayer::renderMarkers(const Vector3d& viewerPosition,
                           const Quaterniond& viewerOrientation,
                           const PlanarProjection& projection,
                           const Viewport& viewport,
                           double simulationTime,
                           double realTime)
{
    foreach (Marker* marker, m_markers)
    {
        renderMarker(viewerPosition, viewerOrientation, projection, viewport, simulationTime, realTime, marker);
    }
}


/** Eliminate all markers that expire before the specified time.
  */
void
MarkerLayer::expireMarkers(double realTime)
{
    for (int i = m_markers.size() - 1; i >= 0; i--)
    {
        if (m_markers.at(i)->isFinished(realTime))
        {
            delete m_markers.at(i);
            m_markers.removeAt(i);
        }
    }
}


void
MarkerLayer::removeAllMarkersOnBody(vesta::Entity* body)
{
    for (int i = m_markers.size() - 1; i >= 0; i--)
    {
        if (m_markers.at(i)->body() == body)
        {
            delete m_markers.at(i);
            m_markers.removeAt(i);
        }
    }
}


Marker::Marker() :
    m_startTime(0.0),
    m_lifeTime(1.0),
    m_style(Pulse),
    m_targetSizeThreshold(0.0f),
    m_directionIndicatorEnabled(false)
{
}


Marker::~Marker()
{
}


void
Marker::setBody(vesta::Entity* body)
{
    m_body = body;
}


void
Marker::setColor(const Spectrum& color)
{
    m_color = color;
}


void
Marker::setLifeTime(double lifeTime)
{
    m_lifeTime = lifeTime;
}


void
Marker::setStartTime(double startTime)
{
    m_startTime = startTime;
}


/** A marker may be configure to fade out when the target object reaches
  * some maximum screen size.
  *
  * \param the projected extent of the marked body in pixels at which fading begins
  */
void
Marker::setTargetSizeThreshold(float maximumSize)
{
    m_targetSizeThreshold = maximumSize;
}


/** When the direction indicator is enabled, a pointer will be shown at
  * the edge of the view when the target lies outside the view.
  */
void
Marker::setDirectionIndicatorEnabled(bool enabled)
{
    m_directionIndicatorEnabled = enabled;
}
