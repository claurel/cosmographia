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

#define L1_IO            0
#define L1_EUROPA        1
#define L1_GANYMEDE      2
#define L1_CALLISTO      3

#if 0
void GetL1Coor(double jd,int body,double *xyz);
  /* Return the rectangular coordinates of the given satellite
     and the given julian date jd expressed in dynamical time (TAI+32.184s).
     The origin of the xyz-coordinates is the center of the planet.
     The reference frame is "dynamical equinox and ecliptic J2000",
     which is the reference frame in VSOP87 and VSOP87A.
  */

void GetL1OsculatingCoor(double jd0,double jd,int body,double *xyz);
  /* The oculating orbit of epoch jd0, evatuated at jd, is returned.
  */
#endif

#endif // _ASTRO_L1_H_
