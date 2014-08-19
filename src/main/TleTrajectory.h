// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
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
