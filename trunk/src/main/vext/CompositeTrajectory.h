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

#ifndef _COMPOSITE_TRAJECTORY_H_
#define _COMPOSITE_TRAJECTORY_H_

#include <vesta/Trajectory.h>
#include <vector>


class CompositeTrajectory : public vesta::Trajectory
{
private:
    CompositeTrajectory(const std::vector<vesta::Trajectory*>& segments,
                        const std::vector<double>& segmentDurations,
                        double startTime);

public:
    ~CompositeTrajectory();

    virtual vesta::StateVector state(double tdbSec) const;

    virtual double boundingSphereRadius() const
    {
        return m_boundingRadius;
    }

    virtual bool isPeriodic() const
    {
        return m_period > 0.0;
    }

    virtual double period() const
    {
        return m_period;
    }

    static CompositeTrajectory* Create(const std::vector<vesta::Trajectory*>& segments,
                                       const std::vector<double>& segmentDurations,
                                       double startTime);
private:
    double m_startTime;
    std::vector<double> m_segmentDurations;
    std::vector< vesta::counted_ptr<vesta::Trajectory> > m_segments;
    double m_period;
    double m_boundingRadius;
};

#endif // _COMPOSITE_TRAJECTORY_H_
