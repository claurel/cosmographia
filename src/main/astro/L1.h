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

#ifndef _ASTRO_L1_H_
#define _ASTRO_L1_H_

#include <vesta/Trajectory.h>


class L1Orbit : public vesta::Trajectory
{
public:
    enum Satellite
    {
        Io       = 0,
        Europa   = 1,
        Ganymede = 2,
        Callisto = 3,
    };

private:
    L1Orbit(Satellite satellite);

public:
    ~L1Orbit() {}

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

    static L1Orbit* Create(Satellite satellite);

private:
    Satellite m_satellite;
    double m_boundingRadius;
    double m_period;
};

#endif // _ASTRO_L1_H_
