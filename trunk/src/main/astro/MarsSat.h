// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
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

#ifndef _ASTRO_MARSSAT_H_
#define _ASTRO_MARSSAT_H_

#include <vesta/Trajectory.h>


class MarsSatOrbit : public vesta::Trajectory
{
public:
    enum Satellite
    {
        Phobos       = 0,
        Deimos       = 1,
    };

private:
    MarsSatOrbit(Satellite satellite);

public:
    ~MarsSatOrbit() {}

    virtual vesta::StateVector state(double tdbSec) const;

    virtual double boundingSphereRadius() const
    {
        return m_boundingRadius;
    }

    virtual bool isPeriodic() const
    {
        return true;
    }

    virtual double period() const
    {
        return m_period;
    }

    static MarsSatOrbit* Create(Satellite satellite);

private:
    Satellite m_satellite;
    double m_boundingRadius;
    double m_period;
};

#endif // _ASTRO_MARSSAT_H_
