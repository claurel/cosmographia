// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "CelBodyFixedFrame.h"

using namespace vesta;
using namespace Eigen;


/** Create a new Celestia-compatible body-fixed frame. In Celestia, the
  * body-fixed frame has an extra 180 degree rotation about the y-axis.
  */
CelBodyFixedFrame::CelBodyFixedFrame(Entity* body) :
    m_body(body)
{
}


CelBodyFixedFrame::~CelBodyFixedFrame()
{
}


/** \reimp
  */
Quaterniond
CelBodyFixedFrame::orientation(double tdbSec) const
{
    return m_body->orientation(tdbSec) * Quaterniond(0.0, 0.0, 0.0, 1.0);
}


/** \reimp
  */
Vector3d
CelBodyFixedFrame::angularVelocity(double tdbSec) const
{
    return m_body->angularVelocity(tdbSec);
}


/** Get the body to which this frame is fixed.
  */
Entity*
CelBodyFixedFrame::body() const
{
    return m_body.ptr();
}
