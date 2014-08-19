// ChebyshevPolyTrajectory.h
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
