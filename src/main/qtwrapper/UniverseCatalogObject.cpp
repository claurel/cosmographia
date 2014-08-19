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
    else
    {
        return m_catalog->getDescription(body->body());
    }
}
