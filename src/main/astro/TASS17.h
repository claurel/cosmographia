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

#ifndef _ASTRO_TASS17_H_
#define _ASTRO_TASS17_H_

#include <vesta/Trajectory.h>


class TASS17Orbit : public vesta::Trajectory
{
public:
    enum Satellite
    {
        Mimas       = 0,
        Enceladus   = 1,
        Tethys      = 2,
        Dione       = 3,
        Rhea        = 4,
        Titan       = 5,
        Iapetus     = 6,
        Hyperion    = 7,
    };

private:
    TASS17Orbit(Satellite satellite);

public:
    ~TASS17Orbit() {}

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

    static TASS17Orbit* Create(Satellite satellite);

private:
    Satellite m_satellite;
    double m_boundingRadius;
    double m_period;
};

#endif // _ASTRO_TASS17_H_
