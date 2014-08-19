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
