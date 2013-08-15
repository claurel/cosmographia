// JPLEphemeris.h
//
// Copyright (C) 2010-2013 Chris Laurel <claurel@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 
//
//    1. Redistributions of source code must retain the above copyright notice, this
//       list of conditions and the following disclaimer. 
//    2. Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
