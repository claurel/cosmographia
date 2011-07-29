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

#ifndef _TIME_SWITCHED_GEOMETRY_H_
#define _TIME_SWITCHED_GEOMETRY_H_

#include <vesta/Geometry.h>
#include <vector>

/** TimeSwitchedGeometry contains a time-tagged sequence of geometry objects.
  * Only one geometry object will be shown at a time.
  */
class TimeSwitchedGeometry : public vesta::Geometry
{
public:
    TimeSwitchedGeometry();
    virtual ~TimeSwitchedGeometry();

    void render(vesta::RenderContext& rc, double clock) const;

    /** \reimp */
    float boundingSphereRadius() const
    {
        return m_boundingRadius;
    }

    /** \reimp */
    virtual bool isOpaque() const
    {
        return m_opaque;
    }

    vesta::Geometry* geometry(unsigned int index) const;
    double startTime(unsigned int index) const;
    void addGeometry(double startTime, vesta::Geometry* label);

    vesta::Geometry* activeGeometry(double tdb) const;

private:
    std::vector<double> m_times;
    std::vector<vesta::counted_ptr<vesta::Geometry> > m_geometries;
    float m_boundingRadius;
    bool m_opaque;
};

#endif // _TIME_SWITCHED_GEOMETRY_H_
