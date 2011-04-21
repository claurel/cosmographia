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
#include <vesta/InertialFrame.h>

using namespace vesta;
using namespace Eigen;


Viewpoint::Viewpoint(vesta::Entity* centerBody, double distance) :
    m_centerBody(centerBody),
    m_referenceBody(NULL),
    m_centerDistance(distance)
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

    Vector3d up = m_centerBody->orientation(tdbSec) * Vector3d::UnitZ();

    observer->setPositionFrame(InertialFrame::icrf());
    observer->setPointingFrame(InertialFrame::icrf());
    observer->setPosition(toRefDir * m_centerDistance);
}
