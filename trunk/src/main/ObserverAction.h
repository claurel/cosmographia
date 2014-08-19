// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
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
