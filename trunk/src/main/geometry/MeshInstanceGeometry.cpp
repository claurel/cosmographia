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

#include "MeshInstanceGeometry.h"
#include <vesta/RenderContext.h>

using namespace vesta;
using namespace Eigen;


MeshInstanceGeometry::MeshInstanceGeometry(vesta::MeshGeometry* mesh) :
    m_mesh(mesh)
{
    setShadowReceiver(true);
    setShadowCaster(true);
}


MeshInstanceGeometry::~MeshInstanceGeometry()
{
}


void
MeshInstanceGeometry::render(RenderContext& rc,
                             double animationClock) const
{
    rc.pushModelView();
    rc.scaleModelView(Vector3f::Constant(m_scale));
    m_mesh->render(rc, animationClock);
    rc.popModelView();
}


void
MeshInstanceGeometry::renderShadow(RenderContext& rc,
                                   double animationClock) const
{
    rc.pushModelView();
    rc.scaleModelView(Vector3f::Constant(m_scale));
    m_mesh->renderShadow(rc, animationClock);
    rc.popModelView();
}


float
MeshInstanceGeometry::boundingSphereRadius() const
{
    return m_mesh->boundingSphereRadius() * m_scale;
}


bool
MeshInstanceGeometry::handleRayPick(const Vector3d& pickOrigin,
                                    const Vector3d& pickDirection,
                                    double clock,
                                    double* distance) const
{
    double invScale = 1.0 / m_scale;
    Vector3d origin = invScale * pickOrigin;
    Vector3d direction = (invScale * pickDirection).normalized();

    bool hit = m_mesh->rayPick(origin, direction, clock, distance);
    if (distance)
    {
        *distance *= m_scale;
    }

    return hit;
}
