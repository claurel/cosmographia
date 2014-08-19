// SpiceTrajectory.cpp
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

#include "SpiceTrajectory.h"
#include <algorithm>
#include <iostream>

using namespace vesta;
using namespace Eigen;
using namespace std;


SpiceTrajectory::SpiceTrajectory(SpiceInt targetID, SpiceInt centerID, const char* spiceFrame) :
    m_targetID(targetID),
    m_centerID(centerID),
    m_spiceFrame(spiceFrame),
    m_period(0.0)
{
}


SpiceTrajectory::~SpiceTrajectory()
{
}


StateVector
SpiceTrajectory::state(double tdbSec) const
{
    // Clamp time to valid range
    double et = std::max(startTime(), std::min(endTime(), tdbSec));

    SpiceDouble sv[6];
    SpiceDouble lightTime;
    spkgeo_c(m_targetID, et, m_spiceFrame.c_str(), m_centerID, sv, &lightTime);
    if (failed_c())
    {
        char errorMessage[1024];
        getmsg_c("long", sizeof(errorMessage), errorMessage);
        cerr << errorMessage << std::endl;
        reset_c();
        return StateVector(Vector3d::Zero(), Vector3d::Zero());
    }
    else
    {
        return StateVector(Vector3d(sv[0], sv[1], sv[2]), Vector3d(sv[3], sv[4], sv[5]));
    }
}


double
SpiceTrajectory::period() const
{
    return m_period;
}


/** Set the period of an orbit. This is only used for determining how best to plot
 *  the orbit. Setting the period to 0 indicates a non-repeating trajectory.
 */
void
SpiceTrajectory::setPeriod(double periodSeconds)
{
    m_period = periodSeconds;
}


bool
SpiceTrajectory::isPeriodic() const
{
    return m_period > 0.0;
}


double
SpiceTrajectory::boundingSphereRadius() const
{
    return 1.0e12;
}
