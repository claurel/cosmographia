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

#include "TimeSwitchedGeometry.h"
#include <vesta/RenderContext.h>
#include <algorithm>

using namespace vesta;


TimeSwitchedGeometry::TimeSwitchedGeometry() :
    m_boundingRadius(0.0),
    m_opaque(true)
{
}


TimeSwitchedGeometry::~TimeSwitchedGeometry()
{
}


/** \reimpl
  */
void
TimeSwitchedGeometry::render(RenderContext& rc, double clock) const
{
    Geometry* geometry = activeGeometry(clock);
    if (geometry)
    {
        geometry->render(rc, clock);
    }
}


Geometry*
TimeSwitchedGeometry::geometry(unsigned int index) const
{
    if (index < m_geometries.size())
    {
        return m_geometries[index].ptr();
    }
    else
    {
        return NULL;
    }
}


double
TimeSwitchedGeometry::startTime(unsigned int index) const
{
    if (index < m_times.size())
    {
        return m_times[index];
    }
    else
    {
        return 0.0;
    }
}


/** Return the geometry that should be visible at the specified time.
  */
Geometry*
TimeSwitchedGeometry::activeGeometry(double tdb) const
{
    for (unsigned int i = 1; i < m_geometries.size(); ++i)
    {
        if (tdb >= m_times[i - 1] && tdb < m_times[i])
        {
            return m_geometries[i - 1].ptr();
        }
    }

    if (!m_geometries.empty())
    {
        return m_geometries.back().ptr();
    }
    else
    {
        return NULL;
    }
}


/** Add a geometry and time tag. It is legal for the geometry
  * to be NULL, which just indicates that nothing is to be rendered.
  */
void
TimeSwitchedGeometry::addGeometry(double startTime, Geometry* geometry)
{
    m_geometries.push_back(counted_ptr<Geometry>(geometry));
    m_times.push_back(startTime);
    if (geometry)
    {
        m_boundingRadius = std::max(m_boundingRadius, geometry->boundingSphereRadius());

        // Set the shadow caster and receiver properties to true if *any* geometry
        // in sequence has them set to true.
        if (geometry->isShadowCaster())
        {
            setShadowCaster(true);
        }

        if (geometry->isShadowReceiver())
        {
            setShadowReceiver(true);
        }
    }
}
