// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "Viewpoint.h"
#include "RotationUtility.h"
#include <vesta/Units.h>
#include <vesta/InertialFrame.h>

using namespace vesta;
using namespace Eigen;


Viewpoint::Viewpoint(vesta::Entity* centerBody, double distance) :
    m_centerBody(centerBody),
    m_referenceBody(NULL),
    m_centerDistance(distance),
    m_azimuth(0.0),
    m_elevation(0.0),
    m_upDirection(CenterNorth)
{
}


Viewpoint::~Viewpoint()
{
}


void
Viewpoint::positionObserver(vesta::Observer* observer, double tdbSec)
{
    if (m_centerBody.isNull() || m_referenceBody.isNull())
    {
        return;
    }

    Vector3d toRef = m_referenceBody->position(tdbSec) - m_centerBody->position(tdbSec);
    Vector3d toRefDir = toRef.normalized();

    Vector3d up;
    switch (m_upDirection)
    {
    case EclipticNorth:
        up = InertialFrame::eclipticJ2000()->orientation() * Vector3d::UnitZ();
        break;
    case EclipticSouth:
        up = InertialFrame::eclipticJ2000()->orientation() * -Vector3d::UnitZ();
        break;
    case CenterSouth:
        up = m_centerBody->orientation(tdbSec) * -Vector3d::UnitZ();
        break;
    case CenterNorth:
    default:
        up = m_centerBody->orientation(tdbSec) * Vector3d::UnitZ();
        break;
    }

    // Compute the vector w, which points in the direction of up and is perpendicular to the
    // center to reference direction.
    Vector3d v = toRefDir.cross(up).normalized();
    Vector3d w = v.cross(toRefDir);
    Vector3d u = toRefDir.cross(w);
    Vector3d position = AngleAxisd(toRadians(m_azimuth), w).toRotationMatrix() *
                        AngleAxisd(toRadians(m_elevation), u).toRotationMatrix() *
                        (toRefDir * m_centerDistance);

    observer->setCenter(m_centerBody.ptr());
    observer->setPositionFrame(InertialFrame::icrf());
    observer->setPointingFrame(InertialFrame::icrf());
    observer->setPosition(position);
    observer->setOrientation(LookRotation(position, Vector3d::Zero(), up));
}
