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

#ifndef _QTWRAPPER_UNIVERSE_CATALOG_OBJECT_H_
#define _QTWRAPPER_UNIVERSE_CATALOG_OBJECT_H_

#include "../catalog/UniverseCatalog.h"


/** Qt wrapper for Cosmographia's UniverseCatalog class
  */
class UniverseCatalogObject : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE QStringList getMatchingNames(const QString& pattern) const;
    Q_INVOKABLE QString getCompletionString(const QString& partialName, int maxNames) const;

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
