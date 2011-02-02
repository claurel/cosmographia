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


Entity* UniverseCatalog::find(const QString& name) const
{
#if 0
    if (!m_bodies.contains(name))
    {
        return NULL;
    }
#endif
    return m_bodies.value(name).ptr();
}


void UniverseCatalog::removeBody(const QString& name)
{
    m_bodies.remove(name);
}


void UniverseCatalog::addBody(const QString& name, vesta::Entity* body)
{
    m_bodies[name] = counted_ptr<Entity>(body);
}
