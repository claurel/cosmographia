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

#include "MeshInstanceGeometry.h"
#include <vesta/RenderContext.h>

using namespace vesta;
using namespace Eigen;


MeshInstanceGeometry::MeshInstanceGeometry(vesta::MeshGeometry* mesh) :
    m_mesh(mesh),
    m_scale(1.0f),
    m_meshOffset(Vector3f::Zero()),
    m_meshRotation(Quaternionf::Identity())
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
    rc.translateModelView(m_meshOffset);
    rc.rotateModelView(m_meshRotation);
    m_mesh->render(rc, animationClock);
    rc.popModelView();
}


void
MeshInstanceGeometry::renderShadow(RenderContext& rc,
                                   double animationClock) const
{
    rc.pushModelView();
    rc.scaleModelView(Vector3f::Constant(m_scale));
    rc.translateModelView(m_meshOffset);
    rc.rotateModelView(m_meshRotation);
    m_mesh->renderShadow(rc, animationClock);
    rc.popModelView();
}


float
MeshInstanceGeometry::boundingSphereRadius() const
{
    return (m_mesh->boundingSphereRadius() + m_meshOffset.norm()) * m_scale;
}


/** Get an axis-aligned box large enough to contain the geometry.
  */
vesta::BoundingBox
MeshInstanceGeometry::boundingBox() const
{
    if (m_mesh.isValid())
    {
        BoundingBox meshBox = m_mesh->meshBoundingBox();
        return BoundingBox(meshBox.minPoint() * m_scale + m_meshOffset, meshBox.maxPoint() * m_scale + m_meshOffset);
    }
    else
    {
        return BoundingBox();
    }
}


bool
MeshInstanceGeometry::handleRayPick(const Vector3d& pickOrigin,
                                    const Vector3d& pickDirection,
                                    double clock,
                                    double* distance) const
{
    Quaterniond q = m_meshRotation.cast<double>().conjugate();
    double invScale = 1.0 / m_scale;
    Vector3d origin = q * ((invScale * pickOrigin) - m_meshOffset.cast<double>());
    Vector3d direction = q * (invScale * pickDirection).normalized();

    bool hit = m_mesh->rayPick(origin, direction, clock, distance);
    if (distance)
    {
        *distance *= m_scale;
    }

    return hit;
}
