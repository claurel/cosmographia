// JPLEphemeris.h
//
// Copyright (C) 2010-2013 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

    double earthMoonMassRatio() const
    {
        return m_earthMoonMassRatio;
    }

private:
    vesta::counted_ptr<ChebyshevPolyTrajectory> m_trajectories[int(ObjectCount)];
    double m_earthMoonMassRatio;
};

#endif // _JPL_EPHEMERIS_H_
