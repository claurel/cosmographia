// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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
