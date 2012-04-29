// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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
