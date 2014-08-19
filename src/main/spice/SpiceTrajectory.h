// SpiceTrajectory.h
//
// Copyright (C) 2013 Chris Laurel <claurel@gmail.com>
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

#ifndef _SPICE_TRAJECTORY_H_
#define _SPICE_TRAJECTORY_H_

#include <vesta/Trajectory.h>
#include <SpiceUsr.h>

class SpiceTrajectory : public vesta::Trajectory
{
public:
    SpiceTrajectory(SpiceInt targetID, SpiceInt centerID, const char* spiceFrame);
    ~SpiceTrajectory();

    virtual vesta::StateVector state(double tdbSec) const;
    virtual double boundingSphereRadius() const;
    virtual bool isPeriodic() const;
    virtual double period() const;

    void setPeriod(double period);

private:
    SpiceInt m_targetID;
    SpiceInt m_centerID;
    std::string m_spiceFrame;
    double m_period;
};

#endif // _SPICE_TRAJECTORY_H_
