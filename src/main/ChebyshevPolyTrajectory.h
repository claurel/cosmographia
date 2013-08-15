// ChebyshevPolyTrajectory.h
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

#ifndef _CHEBYSHEV_POLY_TRAJECTORY_H_
#define _CHEBYSHEV_POLY_TRAJECTORY_H_

#include <vesta/Trajectory.h>


class ChebyshevPolyTrajectory : public vesta::Trajectory
{
public:
    ChebyshevPolyTrajectory(const double coeffs[],
                            unsigned int degree,
                            double granuleCount,
                            double startTimeTdbSec,
                            double granuleLengthSec);

    ~ChebyshevPolyTrajectory();

    virtual vesta::StateVector state(double tdbSec) const;
    virtual double boundingSphereRadius() const;
    virtual bool isPeriodic() const;
    virtual double period() const;

    void setPeriod(double period);

    static const unsigned int MaxChebyshevDegree = 32;

private:
    double* m_coeffs;
    unsigned int m_degree;
    unsigned int m_granuleCount;
    double m_startTime;
    double m_granuleLength;
    double m_period;
    double m_boundingRadius;
};

#endif // _CHEBYSHEV_POLY_TRAJECTORY_H_
