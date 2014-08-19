// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

#include "CompositeTrajectory.h"
#include <cassert>
#include <algorithm>

using namespace vesta;
using namespace std;


// Construct is private; Create class method should be used to construct
// CompositeTrajectories.
CompositeTrajectory::CompositeTrajectory(const vector<Trajectory*>& segments,
                                         const vector<double>& segmentDurations,
                                         double startTime) :
    m_startTime(startTime),
    m_period(0.0),
    m_boundingRadius(0.0)
{
    assert(segments.size() > 0);
    assert(segments.size() == segmentDurations.size());

    // True if all segments are periodic. The period reported is the average of all
    // the segments' periods (which works reasonably well; the trajectory period is
    // just a hint for trajectory plotting.)
    bool isPeriodic = true;
    double periodSum = 0.0;

    for (unsigned int i = 0; i < segments.size(); ++i)
    {
        m_segments.push_back(counted_ptr<Trajectory>(segments[i]));
        m_segmentDurations.push_back(segmentDurations[i]);

        m_boundingRadius = max(m_boundingRadius, segments[i]->boundingSphereRadius());

        if (!segments[i]->isPeriodic())
        {
            isPeriodic = false;
        }

        periodSum += segments[i]->period();
    }

    if (isPeriodic)
    {
        m_period = periodSum / segments.size();
    }
}


CompositeTrajectory::~CompositeTrajectory()
{
}


StateVector
CompositeTrajectory::state(double tdbSec) const
{
    if (tdbSec <= m_startTime)
    {
        // Clamp to start time
        return m_segments.front()->state(m_startTime);
    }

    double segmentStartTime = m_startTime;
    for (unsigned int i = 0; i < m_segments.size(); ++i)
    {
        if (tdbSec <= segmentStartTime + m_segmentDurations[i])
        {
            return m_segments[i]->state(tdbSec);
        }
        segmentStartTime += m_segmentDurations[i];
    }

    // Time is after all segments; clamp to end time
    return m_segments.back()->state(segmentStartTime);
}


CompositeTrajectory*
CompositeTrajectory::Create(const vector<Trajectory*>& segments,
                            const vector<double>& segmentDurations,
                            double startTime)
{
    // Must have an equal number of segments and durations
    if (segments.size() != segmentDurations.size())
    {
        return NULL;
    }

    if (segments.empty())
    {
        return NULL;
    }

    return new CompositeTrajectory(segments, segmentDurations, startTime);
}
