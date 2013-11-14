// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "BodyObject.h"
#include "vesta/AxesVisualizer.h"
#include "vesta/VelocityVisualizer.h"
#include "vesta/Arc.h"
#include "vesta/WorldGeometry.h"
#include "vesta/PlanetGridLayer.h"
#include <QDebug>

using namespace vesta;
using namespace Eigen;


BodyObject::BodyObject(vesta::Entity* body, QObject* parent) :
    QObject(parent),
    m_body(body)
{
}


BodyObject::~BodyObject()
{
}


QString
BodyObject::name() const
{
    if (m_body.isValid())
    {
        return QString::fromUtf8(m_body->name().c_str(), m_body->name().length());
    }
    else
    {
        return "";
    }
}


bool
BodyObject::isEllipsoid() const
{
    return (m_body.isValid() && m_body->geometry() != NULL && m_body->geometry()->isEllipsoidal());
}


bool
BodyObject::bodyAxes() const
{
    return m_body->visualizer("body axes") != NULL;
}


void
BodyObject::setBodyAxes(bool enabled)
{
    if (enabled)
    {
        AxesVisualizer* axes = new AxesVisualizer(AxesVisualizer::BodyAxes, visualizerSize());
        axes->setLabelEnabled(true, ArrowGeometry::AllAxes);
        axes->setVisibility(true);
        //axes->arrows()->setMinimumScreenSize(100.0f);
        m_body->setVisualizer("body axes", axes);
    }
    else
    {
        m_body->removeVisualizer("body axes");
    }
}


bool
BodyObject::frameAxes() const
{
    return m_body->visualizer("frame axes") != NULL;
}


void
BodyObject::setFrameAxes(bool enabled)
{
    if (enabled)
    {
        AxesVisualizer* axes = new AxesVisualizer(AxesVisualizer::FrameAxes, visualizerSize());
        axes->setLabelEnabled(true, ArrowGeometry::AllAxes);
        axes->setVisibility(true);
        axes->arrows()->setOpacity(0.3f);
        m_body->setVisualizer("frame axes", axes);
    }
    else
    {
        m_body->removeVisualizer("frame axes");
    }
}


bool
BodyObject::velocityArrow() const
{
    return m_body->visualizer("velocity direction") != NULL;
}


void
BodyObject::setVelocityArrow(bool enabled)
{
    if (enabled)
    {
        VelocityVisualizer* arrow = new VelocityVisualizer(visualizerSize());
        arrow->setVisibility(true);
        arrow->setColor(Spectrum(0.25f, 1.0f, 1.0f));
        arrow->setLabelEnabled(true);
        arrow->setLabelText("Velocity");
        m_body->setVisualizer("velocity direction", arrow);
    }
    else
    {
        m_body->removeVisualizer("velocity direction");
    }
}


bool
BodyObject::longLatGrid() const
{
    WorldGeometry* world = dynamic_cast<WorldGeometry*>(m_body->geometry());
    return world != NULL && world->layer("long lat grid") != NULL;
}


void
BodyObject::setLongLatGrid(bool enabled)
{
    WorldGeometry* world = dynamic_cast<WorldGeometry*>(m_body->geometry());
    if (enabled)
    {
        if (world != NULL)
        {
            PlanetGridLayer* grid = new PlanetGridLayer();
            grid->setVisibility(true);
            world->setLayer("long lat grid", grid);
        }
    }
    else
    {
        world->removeLayer("long lat grid");
    }
}


bool
BodyObject::hasVisualizer(const QString& name) const
{
    return m_body->visualizer(name.toUtf8().data()) != NULL;
}


void
BodyObject::removeVisualizer(const QString& name)
{
    m_body->removeVisualizer(name.toUtf8().data());
}


void
BodyObject::setVisualizer(const QString& name, VisualizerObject* visualizer)
{
    if (!visualizer || !visualizer->visualizer())
    {
        removeVisualizer(name);
    }
    else
    {
        m_body->setVisualizer(name.toUtf8().data(), visualizer->visualizer());
    }
}


float
BodyObject::visualizerSize() const
{
    float size = 1.0f;
    if (m_body.isValid() && m_body->geometry())
    {
        size = m_body->geometry()->boundingSphereRadius() * 2.0f;
    }

    return size;
}


/** Get the distance between this body and another at the specified time.
  *
  * This function returns 0 if either object is null, or if one of the objects
  * doesn't exist at the specified time.
  *
  * \returns distance between the two bodies in kilometers
  */
double
BodyObject::distanceTo(BodyObject* other, double t)
{
    Vector3d thisPosition = Vector3d::Zero();
    Vector3d otherPosition = Vector3d::Zero();

    if (m_body.isValid() && other->body())
    {
        if (m_body->chronology()->includesTime(t) && other->body()->chronology()->includesTime(t))
        {
            thisPosition = m_body->position(t);
            otherPosition = other->body()->position(t);
        }
    }

    return (thisPosition - otherPosition).norm();
}


double
BodyObject::relativeSpeed(BodyObject* other, double t)
{
    Vector3d thisVelocity = Vector3d::Zero();
    Vector3d otherVelocity = Vector3d::Zero();

    if (m_body.isValid() && other->body())
    {
        if (m_body->chronology()->includesTime(t) && other->body()->chronology()->includesTime(t))
        {
            thisVelocity = m_body->state(t).velocity();
            otherVelocity = other->body()->state(t).velocity();
        }
    }

    return (thisVelocity - otherVelocity).norm();
}
