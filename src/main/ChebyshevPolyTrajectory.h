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
