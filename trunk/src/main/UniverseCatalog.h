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

#ifndef _UNIVERSE_CATALOG_H_
#define _UNIVERSE_CATALOG_H_

#include <vesta/Entity.h>
#include <QString>
#include <QMap>


class UniverseCatalog
{
public:
    UniverseCatalog();
    ~UniverseCatalog();

    void removeBody(const QString& name);
    void addBody(const QString& name, vesta::Entity* body);
    vesta::Entity* find(const QString& name) const;
    bool contains(const QString& name) const;

private:
    QMap<QString, vesta::counted_ptr<vesta::Entity> > m_bodies;
};

#endif // _UNIVERSE_CATALOG_H_
