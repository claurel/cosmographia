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

#include "ObserverAction.h"
#include "RotationUtility.h"
#include <vesta/InertialFrame.h>
#include <algorithm>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


static double smoothstep(double x)
{
    return x * x * (3 - 2 * x);
}


// Version of smoothstep that is second order continuous at x = 0 and x = 1
// From Ken Perlin
static double smoothstep2(double x)
{
    return x * x * x * (x* (x * 6 - 15) + 10);
}


CenterObserverAction::CenterObserverAction(Observer* observer,
                                           Entity* target,
                                           double duration,
                                           double realTime,
                                           double simulationTime) :
        m_duration(duration),
        m_startTime(realTime)
{
    m_startOrientation = observer->absoluteOrientation(simulationTime);

    Vector3d up = observer->absoluteOrientation(simulationTime) * Vector3d::UnitY();
    m_finalOrientation = LookRotation(observer->absolutePosition(simulationTime),
                                      target->position(simulationTime),
                                      up);
}


bool
CenterObserverAction::updateObserver(Observer* observer, double realTime, double simTime)
{
    double t;
    if (m_duration == 0.0)
    {
        t = 1.0;
    }
    else
    {
        t = min(1.0, (realTime - m_startTime) / m_duration);
    }
    t = smoothstep(t);

    Quaterniond q = m_startOrientation.slerp(t, m_finalOrientation);
    q = observer->pointingFrame()->orientation(simTime).conjugate() * q;
    observer->setOrientation(q);

    return t >= 1.0;
}


template<class T>
static double SolveBisection(const T& f, double lower, double upper, double tolerance)
{
    double x = (lower + upper) / 2.0;
    while (upper - lower > 2.0 * tolerance)
    {
        if (f(x) < 0.0)
        {
            lower = x;
        }
        else
        {
            upper = x;
        }
        x = (lower + upper) / 2.0;
    }

    return x;
}


class ExponentialAcceleration : public unary_function<double, double>
{
public:
    ExponentialAcceleration(double startSpeed, double accelerationFraction) :
        m_startSpeed(startSpeed),
        m_accelerationFraction(accelerationFraction)
    {
    }

    double operator()(double x) const
    {
        double c = exp(m_accelerationFraction * x);
        return (m_startSpeed / x) * (c - 1.0) + (1.0 - m_accelerationFraction) * m_startSpeed * c - 1.0;
    }

private:
    double m_startSpeed;
    double m_accelerationFraction;
};



// Exponential interpolation over the unit interval:
//   There are two phases: the acceleration phase and the constant velocity phase.
//   Distance increases exponentially during the acceleration phase and linearly
//   during the constant phase.
//
// We have the boundary conditions:
//   - x = 0 at t = 0
//   - x = 1 at t = 1
//   - velocity = v0 at t = 0
//   - velocity is continuous between the two phases
//
static double
expInterp(double t, double v0, double accelerationTime)
{
    // No analytical solution; solve numerically. We want to find b such that the interpolation function is 1 at t = 1
    double b = SolveBisection(ExponentialAcceleration(v0, accelerationTime), 1.0e-4, 100.0 / accelerationTime, 1.0e-12);
    double a = v0 / b;

    // Peak velocity is reached when t = acceleration time
    double peakVelocity = v0 * exp(b * accelerationTime);

    // dist1 is the distance covered in the acceleration phase
    double dist1 = a * (exp(b * min(t, accelerationTime)) - 1.0);

    // dist2 is the distance covered in the constant velocity phase
    double dist2 = max(0.0, t - accelerationTime) * peakVelocity;

    return dist1 + dist2;
}


// Exponential acceleration and deceleration in the unit interval
// This function essentially mirrors expInterp to achieve exponential
// ease-in and ease-out with a linear section in the middle. Acceleration
// time is a fraction of the half-interval, not the whole interval. Thus,
// acclerationTime = 0.5 means that one quarter of the time will be
// acceleration, half the time constant velocity, followed by deceleration
// in the final quarter.
static double smoothStepExp(double t, double v0, double accelerationTime)
{
    if (t < 0.5)
    {
        return 0.5 * expInterp(t * 2.0, v0, accelerationTime);
    }
    else
    {
        return 1.0 - 0.5 * expInterp((1.0 - t) * 2.0, v0, accelerationTime);
    }
}


static Vector3d slerp(double t, const Vector3d& v0, const Vector3d& v1)
{
    Vector3d n = v0.cross(v1);
    double sinTheta = n.norm();
    if (abs(sinTheta) > 1.0e-14)
    {
        double theta = asin(sinTheta);
        return ((v0 * sin((1.0 - t) * theta)) + (v1 * sin(t * theta))) / sinTheta;
    }
    else
    {
        return v1;
    }
}


GotoObserverAction::GotoObserverAction(Observer* observer,
                                       Entity* target,
                                       double duration,
                                       double realTime,
                                       double simulationTime,
                                       double finalDistanceFromTarget) :
    m_duration(duration),
    m_startTime(realTime),
    m_switchedFrames(false),
    m_target(target),
    m_finalDistanceFromTarget(finalDistanceFromTarget)
{
    m_startOrientation = observer->absoluteOrientation(simulationTime);
    m_startPosition = observer->absolutePosition(simulationTime);

    Vector3d up = observer->absoluteOrientation(simulationTime) * Vector3d::UnitY();
    m_finalOrientation = LookRotation(observer->absolutePosition(simulationTime),
                                      target->position(simulationTime),
                                      up);
}


bool
GotoObserverAction::updateObserver(Observer* observer, double realTime, double simTime)
{
    double t;
    if (m_duration == 0.0)
    {
        t = 1.0;
    }
    else
    {
        t = min(1.0, (realTime - m_startTime) / m_duration);
    }

    Vector3d targetPosition = m_target->position(simTime);
    Vector3d startToTarget = targetPosition - m_startPosition;
    double distanceFromStart = startToTarget.norm();
    double travelDistance = distanceFromStart - m_finalDistanceFromTarget;

    // Interpolation factor for rotation
    double rt = smoothstep2(min(1.0, t * 4.0));

    // Interpolation factor for position
    double pt = smoothStepExp(t, 0.1 / travelDistance, 0.5);

    Quaterniond q = m_startOrientation.slerp(rt, m_finalOrientation);
    q = observer->pointingFrame()->orientation(simTime).conjugate() * q;
    observer->setOrientation(q);


    Vector3d currentPosition = m_startPosition + (pt * (travelDistance / distanceFromStart)) * startToTarget;

    // Transform the current position into the observer frame
    Vector3d p = currentPosition - observer->center()->position(simTime);
    p = observer->positionFrame()->orientation(simTime).conjugate() * p;
    observer->setPosition(p);

    if (t > 0.5 && !m_switchedFrames)
    {
        // Switch to the target frame
        observer->updateCenter(m_target.ptr(), simTime);
        m_switchedFrames = true;
    }

    return t >= 1.0;
}


/** OrbitGoto is a specialized observer action that does the following:
  *   - Zooms away from the current center object
  *   - Orients the observer to point at the target
  *   - Orbits around the target so that the observer is above and
  *     facing down at the target.
  *
  * This class makes assumptions about the final offset from the target
  * object; in it's current state, it is really only suitable for the
  * the UniverseView::gotoHom function.
  */
OrbitGotoObserverAction::OrbitGotoObserverAction(Observer* observer,
                                                 Entity* target,
                                                 double duration,
                                                 double realTime,
                                                 double simulationTime,
                                                 double finalDistanceFromTarget) :
    m_duration(duration),
    m_startTime(realTime),
    m_switchedFrames(false),
    m_target(target),
    m_finalDistanceFromTarget(finalDistanceFromTarget),
    m_startDistance(0.0),
    m_up(Vector3d::UnitZ())
{
    m_startOrientation = observer->absoluteOrientation(simulationTime);
    m_startPosition = observer->absolutePosition(simulationTime);

    Vector3d targetPosition = target->position(simulationTime);
    m_startDistance = observer->position().norm();

    Vector3d up = observer->absoluteOrientation(simulationTime) * Vector3d::UnitY();
    m_finalOrientation = LookRotation(observer->absolutePosition(simulationTime),
                                      targetPosition,
                                      up);
}


bool
OrbitGotoObserverAction::updateObserver(Observer* observer, double realTime, double simTime)
{
    double t;
    if (m_duration == 0.0)
    {
        t = 1.0;
    }
    else
    {
        t = min(1.0, (realTime - m_startTime) / m_duration);
    }

    Vector3d targetPosition = m_target->position(simTime);

    if (t <= 0.5)
    {
        double u = smoothStepExp(t * 2, 0.1 / m_startDistance, 0.5);
        double distance = (1.0 - u) * m_startDistance + u * m_finalDistanceFromTarget * 0.1;
        observer->setPosition(observer->position().normalized() * distance);
    }
    else
    {
        if (!m_switchedFrames)
        {
            // Switch to the target frame
            observer->updateCenter(m_target.ptr(), simTime);
            m_startPosition = observer->position();
            m_startDistance = observer->position().norm();
            m_up = observer->absoluteOrientation(simTime) * Vector3d::UnitY();
            m_switchedFrames = true;
        }

        double u = smoothstep2((t - 0.5) * 2);
        double distance = (1.0 - u) * m_startDistance + u * m_finalDistanceFromTarget;
        Vector3d finalPosition = InertialFrame::eclipticJ2000()->orientation() * Vector3d::UnitZ() * m_finalDistanceFromTarget;
        Vector3d v0 = m_startPosition.normalized();
        Vector3d v1 = finalPosition.normalized();
        Vector3d w = slerp(u, v0, v1);
        observer->setPosition(distance * w);
    }

    double rt = smoothstep2(min(1.0, t * 4.0));
    Vector3d up = Vector3d::UnitZ();
    Quaterniond finalOrientation = LookRotation(observer->absolutePosition(simTime),
                                                targetPosition,
                                                up);

    Quaterniond q = m_startOrientation.slerp(rt, finalOrientation);
    q = observer->pointingFrame()->orientation(simTime).conjugate() * q;
    observer->setOrientation(q);

    return t >= 1.0;
}
