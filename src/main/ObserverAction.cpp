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

#include "ObserverAction.h"
#include <algorithm>

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


Quaterniond lookRotation(const Eigen::Vector3d& from,
                         const Eigen::Vector3d& to,
                         const Eigen::Vector3d& up)
{
    Vector3d lookDir = to - from;
    if (lookDir.isZero())
    {
        return Quaterniond::Identity();
    }

    Vector3d zAxis = -lookDir.normalized();

    // x-axis normal to both the z-axis and the up vector
    Vector3d xAxis = up.cross(zAxis);
    if (xAxis.isZero())
    {
        // Up vector is parallel to the look direction; choose instead an
        // arbitrary vector orthogonal to the look direction.
        xAxis = zAxis.cross(zAxis.unitOrthogonal());
    }

    xAxis.normalize();
    Vector3d yAxis = zAxis.cross(xAxis);

    Matrix3d m;
    m << xAxis, yAxis, zAxis;

    return Quaterniond(m);

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
    m_finalOrientation = lookRotation(observer->absolutePosition(simulationTime),
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


#if 0
class GotoObserverAction : public vesta::Object
{
public:
    GotoAction(Observer* observer, Entity* target, Frame* targetFrame, double targetDistance, double now) :
        m_target(target),
        m_targetFrame(targetFrame),
        m_duration(5.0)
    {
        m_startPosition = observer->position();

        Vector3d startPos = observer->absolutePosition(now);
        Vector3d endTargetPos = target->position(now + )

    }

    bool updateObserver(Observer* observer, double now, double tsec)
    {
        double t = tsec / m_duration;

        if (t > 0.5)
        {
            // Switch to the target frame
            observer->updateCenter(m_target.ptr(), now);
            observer->updatePositionFrame(m_targetFrame.ptr(), now);
            Vector3d position = observer->absolutePosition(now);
            Quaterniond orientation = observer->absoluteOrientation(now);
        }

        return t >= 1.0;
    }

private:
    counted_ptr<Entity> m_target;
    counted_ptr<Frame> m_targetFrame;
    Vector3d m_startPosition;
    Vector3d m_targetPosition;
    double m_duration;
};
#endif
