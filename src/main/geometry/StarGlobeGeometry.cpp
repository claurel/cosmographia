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

#include "StarGlobeGeometry.h"
#include <vesta/RenderContext.h>

using namespace vesta;
using namespace Eigen;


StarGlobeGeometry::StarGlobeGeometry()
{
}


StarGlobeGeometry::~StarGlobeGeometry()
{
}


float
StarGlobeGeometry::boundingSphereRadius() const
{
    return m_ellipsoidAxes.maxCoeff() * 0.5f;
}


void
StarGlobeGeometry::render(RenderContext& rc, double clock)
{
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        return;
    }


}


