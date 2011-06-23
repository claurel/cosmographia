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
#include <QDebug>

using namespace vesta;


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
    return QString::fromUtf8(m_body->name().c_str(), m_body->name().length());
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


/** Return a one-line description of the object.
  */
QString
BodyObject::description() const
{
    if (m_body.isNull())
    {
        return "";
    }

    // TODO: Provide some means of customizing the text. At the moment,
    // we just guess based on object orbit and size
    Geometry* geometry = m_body->geometry();
    if (geometry == NULL)
    {
        return "Reference point";
    }

    float radius = geometry->boundingSphereRadius();

    if (radius < 1.0f)
    {
        return "Spacecraft";
    }

    // Special case for the sun
    if (m_body->name() == "Sun")
    {
        return "Star";
    }

    Entity* center = m_body->chronology()->firstArc()->center();
    if (center == NULL || center->name() == "Sun")
    {
        if (radius > 10000.0f)
        {
            return "Planet (gas giant)";
        }
        else if (radius > 1500.0f)
        {
            return "Planet (terrestrial)";
        }
        else if (radius > 400.0f)
        {
            return "Dwarf planet";
        }
        else
        {
            return "Asteroid";
        }
    }
    else
    {
        return QString("Moon of %1").arg(QString::fromUtf8(center->name().c_str(), center->name().length()));
    }
}
