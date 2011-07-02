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

#include "UniverseCatalog.h"
#include "../Viewpoint.h"
#include <QRegExp>

using namespace vesta;


UniverseCatalog::UniverseCatalog()
{
}


UniverseCatalog::~UniverseCatalog()
{
}


bool UniverseCatalog::contains(const QString& name) const
{
    return m_bodies.contains(name);
}


/** Lookup the VESTA body with the specified name.
  */
Entity* UniverseCatalog::find(const QString& name, Qt::CaseSensitivity caseSensitivity) const
{
    Entity* body = NULL;

    if (caseSensitivity == Qt::CaseSensitive)
    {
        body = m_bodies.value(name).ptr();
    }
    else
    {
        foreach (QString s, m_bodies.keys())
        {
            if (name.compare(s, Qt::CaseInsensitive) == 0)
            {
                body = m_bodies.value(s).ptr();
                break;
            }
        }
    }

    return body;
}


/** Look up the extra (i.e. non-VESTA) information for
  * named body. This method returns null if the named
  * body isn't found or if it doesn't have any extra
  * information.
  */
BodyInfo* UniverseCatalog::findInfo(const QString& name) const
{
    return m_info.value(name).ptr();
}


/** Look up the extra (i.e. non-VESTA) information for
  * named body. This method returns null if it doesn't have any extra
  * information.
  */
BodyInfo* UniverseCatalog::findInfo(const Entity* body) const
{
    return m_info.value(QString::fromUtf8(body->name().c_str())).ptr();
}


void UniverseCatalog::removeBody(const QString& name)
{
    m_bodies.remove(name);
    m_info.remove(name);
}


void UniverseCatalog::addBody(const QString& name, vesta::Entity* body, BodyInfo* info)
{
    m_bodies[name] = counted_ptr<Entity>(body);
    m_info[name] = info;
}


/** Set the addition information record for a body. This has
  * no effect if the named object doesn't exist in the catalog.
  */
void UniverseCatalog::setBodyInfo(const QString& name, BodyInfo* info)
{
    if (m_bodies.contains(name))
    {
        m_info[name] = info;
    }
}


/** Return a list of the names of all objects in the catalog.
  */
QStringList UniverseCatalog::names() const
{
    return m_bodies.keys();
}


/** Return a list of the names of all objects in the catalog that match the specified
  * regular expression.
  */
QStringList
UniverseCatalog::matchingNames(const QString& pattern) const
{
    QRegExp regex(pattern, Qt::CaseInsensitive);

    QStringList matches;
    foreach (QString name, m_bodies.keys())
    {
        if (regex.exactMatch(name))
        {
            matches << name;
        }
    }

    return matches;
}


/** Look up the viewpoint with the specified name.
  */
Viewpoint*
UniverseCatalog::findViewpoint(const QString& name)
{
    return m_viewpoints.value(name).ptr();
}


void
UniverseCatalog::addViewpoint(const QString& name, Viewpoint* viewpoint)
{
    m_viewpoints.insert(name, counted_ptr<Viewpoint>(viewpoint));
}


void
UniverseCatalog::removeViewpoint(const QString& name)
{
    m_viewpoints.remove(name);
}


/** Return the names of all viewpoints in the catalog.
  */
QStringList
UniverseCatalog::viewpointNames() const
{
    return m_viewpoints.keys();
}


struct ClassificationName
{
    BodyInfo::Classification classification;
    const char* name;
};

static ClassificationName classificationNames[] =
{
    { BodyInfo::Planet, "planet" },
    { BodyInfo::DwarfPlanet, "dwarf planet" },
    { BodyInfo::Satellite, "satellite" },
    { BodyInfo::Spacecraft, "spacecraft" },
    { BodyInfo::Asteroid, "asteroid" },
    { BodyInfo::ReferencePoint, "reference point" },
    { BodyInfo::Star, "star" },
    { BodyInfo::Other, "other" }
};


BodyInfo::Classification
BodyInfo::parseClassification(const QString& classificationName)
{
    for (unsigned int i = 0; i < sizeof(classificationNames) / sizeof(classificationNames[0]); ++i)
    {
        if (classificationName == classificationNames[i].name)
        {
            return classificationNames[i].classification;
        }
    }
    return BodyInfo::Other;
}
