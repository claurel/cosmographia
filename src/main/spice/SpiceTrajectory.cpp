// SpiceTrajectory.cpp
//
// Copyright (C) 2013 Chris Laurel <claurel@gmail.com>
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

#include "SpiceTrajectory.h"
#include <algorithm>

using namespace vesta;
using namespace Eigen;


SpiceTrajectory::SpiceTrajectory(SpiceInt targetID, SpiceInt observerID, const char* spiceFrame) :
    m_targetID(targetID),
    m_observerID(observerID),
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
    spkgeo_c(m_targetId, et, spiceFrame.c_str(), m_centerID, sv, &lightTime);
    if (failed_c())
    {
        char errorMsg[1024];
        getmsg_c("long", sizeof(errorMsg), errorMsg);
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
