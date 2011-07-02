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

#include "UniverseCatalogObject.h"
#include <vesta/Arc.h>
#include <QDeclarativeEngine>
#include <QDebug>
#include <algorithm>

using namespace vesta;


/** Wrap a UniverseCatalog with a new UniverseCatalogObject. The lifetime of the UniverseCatalog
  * must be at least as long as the wrapper object (i.e. no management of the catalog pointer
  * is done.)
  */
UniverseCatalogObject::UniverseCatalogObject(UniverseCatalog* catalog, QObject* parent) :
    QObject(parent),
    m_catalog(catalog)
{
}


UniverseCatalogObject::~UniverseCatalogObject()
{
}


QStringList
UniverseCatalogObject::getMatchingNames(const QString& pattern) const
{
    return m_catalog->matchingNames(pattern);
}


/** Get a comma separated list of names that start with the specified string.
  */
QString
UniverseCatalogObject::getCompletionString(const QString& partialName, int maxNames) const
{
    QStringList matches = m_catalog->matchingNames(partialName + ".*");
    QString completionList;
    for (int i = 0; i < std::min(maxNames, matches.length()); ++i)
    {
        if (i != 0)
        {
            completionList += ", ";
        }
        completionList += matches.at(i);
    }

    return completionList;
}


BodyObject*
UniverseCatalogObject::getEarth() const
{
    BodyObject* o = new BodyObject(m_catalog->find("Earth"));
    QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
    return o;
}


BodyObject*
UniverseCatalogObject::getSun() const
{
    BodyObject* o = new BodyObject(m_catalog->find("Sun"));
    QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
    return o;
}


BodyObject*
UniverseCatalogObject::lookupBody(const QString& name) const
{
    Entity* body = m_catalog->find(name, Qt::CaseInsensitive);
    if (body)
    {
        BodyObject* o = new BodyObject(body);
        QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
        return o;
    }
    else
    {
        return NULL;
    }
}


static BodyInfo::Classification
guessClassification(const vesta::Entity* body)
{
    Geometry* geometry = body->geometry();
    if (geometry == NULL)
    {
        return BodyInfo::ReferencePoint;
    }

    float radius = geometry->boundingSphereRadius();
    if (radius < 1.0f)
    {
        return BodyInfo::Spacecraft;
    }

    // Special case for the sun
    if (body->name() == "Sun")
    {
        return BodyInfo::Star;
    }

    Entity* center = body->chronology()->firstArc()->center();
    if (center == NULL || center->name() == "Sun")
    {
        if (radius > 1500.0f)
        {
            return BodyInfo::Planet;
        }
        else if (radius > 400.0f)
        {
            return BodyInfo::DwarfPlanet;
        }
        else
        {
            return BodyInfo::Asteroid;
        }
    }
    else
    {
        return BodyInfo::Satellite;
    }
}


static QString
getDefaultDescription(const vesta::Entity* body, BodyInfo::Classification classification)
{
    float radius = 0.0f;
    Geometry* geometry = body->geometry();
    if (geometry)
    {
        radius = geometry->boundingSphereRadius();
    }

    QString description;

    switch (classification)
    {
    case BodyInfo::Star:
        description = "Star";
        break;

    case BodyInfo::ReferencePoint:
        description = "Reference Point";
        break;

    case BodyInfo::Planet:
        if (radius > 10000.0f)
        {
            description = "Planet (gas giant)";
        }
        else if (radius > 1500.0f)
        {
            description = "Planet (terrestrial)";
        }
        break;

    case BodyInfo::DwarfPlanet:
        description = "Dwarf Planet";
        break;

    case BodyInfo::Asteroid:
        description = "Asteroid";
        break;

    case BodyInfo::Spacecraft:
        description = "Spacecraft";
        break;

    case BodyInfo::Satellite:
        {
            Entity* center = body->chronology()->firstArc()->center();
            if (center)
            {
                description = QString("Moon of %1").arg(QString::fromUtf8(center->name().c_str(), center->name().length()));
            }
            else
            {
                description = "Moon";
            }
        }
        break;

    default:
        description = "Unknown object type";
        break;
    }

    return description;
}


/** Get a one-line description of the specified object.
  */
QString
UniverseCatalogObject::getDescription(QObject* bodyObj)
{
    const BodyObject* body = qobject_cast<const BodyObject*>(bodyObj);
    if (!body || !body->body())
    {
        return "";
    }

    BodyInfo* info = m_catalog->findInfo(body->body()->name().c_str());
    BodyInfo::Classification classification = BodyInfo::Other;

    if (info && !info->description.isEmpty())
    {
        return info->description;
    }
    else
    {
        if (!info || info->classification == BodyInfo::Other)
        {
            classification = guessClassification(body->body());
        }
        else
        {
            classification = info->classification;
        }
        return getDefaultDescription(body->body(), classification);
    }
}
