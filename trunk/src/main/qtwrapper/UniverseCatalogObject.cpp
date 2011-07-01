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
#include <algorithm>


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
