// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
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

#ifndef _JPL_EPHEMERIS_H_
#define _JPL_EPHEMERIS_H_

#include "ChebyshevPolyTrajectory.h"
#include <string>


class JPLEphemeris
{
public:
    JPLEphemeris();
    ~JPLEphemeris();

    enum JplObjectId
    {
        Mercury = 0,
        Venus = 1,
        EarthMoonBarycenter = 2,
        Mars = 3,
        Jupiter = 4,
        Saturn = 5,
        Uranus = 6,
        Neptune = 7,
        Pluto = 8,
        Moon = 9,
        Sun = 10,
        Earth = 11,
        ObjectCount = 12,
    };

    ChebyshevPolyTrajectory* trajectory(JplObjectId id) const;
    void setTrajectory(JplObjectId id, ChebyshevPolyTrajectory* trajectory);

    static JPLEphemeris* load(const std::string& filename);

private:
    vesta::counted_ptr<ChebyshevPolyTrajectory> m_trajectories[int(ObjectCount)];
};

#endif // _JPL_EPHEMERIS_H_
