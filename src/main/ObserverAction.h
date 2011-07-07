// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
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

#ifndef _OBSERVER_ACTION_H_
#define _OBSERVER_ACTION_H_

#include <vesta/Observer.h>


class ObserverAction : public vesta::Object
{
public:
    ObserverAction()
    {
    }

    virtual bool updateObserver(vesta::Observer* observer, double realTime, double simTime) = 0;
};


class CenterObserverAction : public ObserverAction
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    CenterObserverAction(vesta::Observer* observer, vesta::Entity* target, double duration, double realTime, double simulationTime);
    virtual bool updateObserver(vesta::Observer* observer, double realTime, double simTime);

private:
    double m_duration;
    double m_startTime;
    Eigen::Quaterniond m_startOrientation;
    Eigen::Quaterniond m_finalOrientation;
};


class GotoObserverAction : public ObserverAction
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    GotoObserverAction(vesta::Observer* observer,
                       vesta::Entity* target,
                       double duration,
                       double realTime,
                       double simulationTime,
                       double finalDistanceFromTarget);
    virtual bool updateObserver(vesta::Observer* observer, double realTime, double simTime);
    vesta::Entity* target() const
    {
        return m_target.ptr();
    }

private:
    double m_duration;
    double m_startTime;
    Eigen::Quaterniond m_startOrientation;
    Eigen::Quaterniond m_finalOrientation;
    Eigen::Vector3d m_startPosition;
    bool m_switchedFrames;
    vesta::counted_ptr<vesta::Entity> m_target;
    double m_finalDistanceFromTarget;
};


class OrbitGotoObserverAction : public ObserverAction
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    OrbitGotoObserverAction(vesta::Observer* observer,
                       vesta::Entity* target,
                       double duration,
                       double realTime,
                       double simulationTime,
                       double finalDistanceFromTarget);
    virtual bool updateObserver(vesta::Observer* observer, double realTime, double simTime);
    vesta::Entity* target() const
    {
        return m_target.ptr();
    }

private:
    double m_duration;
    double m_startTime;
    Eigen::Quaterniond m_startOrientation;
    Eigen::Quaterniond m_finalOrientation;
    Eigen::Vector3d m_startPosition;
    bool m_switchedFrames;
    vesta::counted_ptr<vesta::Entity> m_target;
    double m_finalDistanceFromTarget;
    double m_startDistance;
    Eigen::Vector3d m_up;
};

#endif // _OBSERVER_ACTION_H_
