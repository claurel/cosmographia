// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
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

#include "MarkerLayer.h"
#include <vesta/Units.h>
#include <QGLWidget>
#include <QDebug>
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
    marker->setStartTime(startTime);
    marker->setLifeTime(lifeTime);

    m_markers << marker;
}


void
MarkerLayer::renderMarkers(const Vector3d& viewerPosition,
                           const Quaterniond& viewerOrientation,
                           const PlanarProjection& projection,
                           const Viewport& viewport,
                           double simulationTime,
                           double realTime)
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    foreach (Marker* marker, m_markers)
    {
        Entity* body = marker->body();
        Vector3d position = body->position(simulationTime);
        Vector3d cameraRelativePosition = position - viewerPosition;
        Vector3d cameraSpacePosition = viewerOrientation.conjugate() * cameraRelativePosition;

        // Project the location of the body into screen coordinates
        Vector4d p;
        p.start<3>() = cameraSpacePosition;
        p.w() = 1.0;
        Vector4f ndc = projection.matrix() * p.cast<float>();

        if (ndc.w() > 0)
        {
            float x = ((ndc.x() / ndc.w()) * 0.5f + 0.5f) * viewport.width();
            float y = ((ndc.y() / ndc.w()) * 0.5f + 0.5f) * viewport.height();
            unsigned int segments = 24;
            float t = float(std::fmod(realTime - marker->startTime(), marker->lifeTime()) / marker->lifeTime());
            float r = marker->size() * t;

            Spectrum color = marker->color();
            glColor4f(color.red(), color.green(), color.blue(), 1.0f - t);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            for (unsigned int i = 0; i < segments; ++i)
            {
                float theta = float(i * vesta::PI * 2.0) / segments;
                glVertex2f(x + cos(theta) * r, y + sin(theta) * r);
            }
            glEnd();
            glLineWidth(1.0f);
        }
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



MarkerLayer::Marker::Marker() :
    m_startTime(0.0),
    m_lifeTime(1.0)
{
}


MarkerLayer::Marker::~Marker()
{
}


void
MarkerLayer::Marker::setBody(vesta::Entity* body)
{
    m_body = body;
}


void
MarkerLayer::Marker::setColor(const Spectrum& color)
{
    m_color = color;
}


void
MarkerLayer::Marker::setLifeTime(double lifeTime)
{
    m_lifeTime = lifeTime;
}


void
MarkerLayer::Marker::setStartTime(double startTime)
{
    m_startTime = startTime;
}
