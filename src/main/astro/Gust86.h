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

#ifndef _ASTRO_GUST86_H_
#define _ASTRO_GUST86_H_

#include <vesta/Trajectory.h>


class Gust86Orbit : public vesta::Trajectory
{
public:
    enum Satellite
    {
        Miranda       = 0,
        Ariel         = 1,
        Umbriel       = 2,
        Titania       = 3,
        Oberon        = 4,
    };

private:
    Gust86Orbit(Satellite satellite);

public:
    ~Gust86Orbit() {}

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

    static Gust86Orbit* Create(Satellite satellite);

private:
    Satellite m_satellite;
    double m_boundingRadius;
    double m_period;
};

#endif // _ASTRO_GUST86_H_
