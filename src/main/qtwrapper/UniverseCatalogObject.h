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

#ifndef _QTWRAPPER_UNIVERSE_CATALOG_OBJECT_H_
#define _QTWRAPPER_UNIVERSE_CATALOG_OBJECT_H_

#include "../catalog/UniverseCatalog.h"
#include "BodyObject.h"

/** Qt wrapper for Cosmographia's UniverseCatalog class
  */
class UniverseCatalogObject : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE QStringList getMatchingNames(const QString& pattern) const;
    Q_INVOKABLE QString getCompletionString(const QString& partialName, int maxNames) const;

    Q_INVOKABLE BodyObject* getEarth() const;
    Q_INVOKABLE BodyObject* getSun() const;
    Q_INVOKABLE BodyObject* lookupBody(const QString& name) const;

    Q_INVOKABLE QString getDescription(QObject* bodyObj);

public:
    UniverseCatalogObject(UniverseCatalog* catalog, QObject* parent = NULL);
    ~UniverseCatalogObject();

    UniverseCatalog* universeCatalog() const
    {
        return m_catalog;
    }

private:
    UniverseCatalog* m_catalog;
};


#endif // _QTWRAPPER_UNIVERSE_CATALOG_OBJECT_H_
