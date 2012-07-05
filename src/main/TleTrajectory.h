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

#ifndef _TLE_TRAJECTORY_H_
#define _TLE_TRAJECTORY_H_

#include <vesta/Trajectory.h>
#include <vesta/OrbitalElements.h>
#include <noradtle/norad.h>


class TleTrajectory : public vesta::Trajectory
{
private:
    TleTrajectory(tle_t* tle);

public:
    ~TleTrajectory();

    virtual vesta::StateVector state(double tsec) const;
    virtual double boundingSphereRadius() const;
    virtual bool isPeriodic() const;
    virtual double period() const;

    double epoch() const
    {
        return m_epoch;
    }

    void copy(TleTrajectory* other);

    void setKeplerianApproximationLimit(double tsec);

    static TleTrajectory* Create(const std::string& line1, const std::string& line2);

private:
    vesta::StateVector tleState(double tsec) const;

private:
    tle_t* m_tle;
    double m_epoch;
    int m_ephemerisType;
    double m_satParams[N_SAT_PARAMS];

    double m_keplerianApproxLimit;
    vesta::OrbitalElements m_keplerianBefore;
    vesta::OrbitalElements m_keplerianAfter;
};

#endif // _TLE_TRAJECTORY_H_
