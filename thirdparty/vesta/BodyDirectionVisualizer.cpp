/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "BodyDirectionVisualizer.h"
#include "Entity.h"
#include "Arc.h"

using namespace vesta;
using namespace Eigen;


BodyDirectionVisualizer::BodyDirectionVisualizer(double size, Entity* body) :
    ArrowVisualizer(size),
    m_body(body)
{
    setLabelText( body->name() );
}


BodyDirectionVisualizer::~BodyDirectionVisualizer()
{
}


void
BodyDirectionVisualizer::setBody(Entity* body)
{
    m_body = body;
}


Vector3d
BodyDirectionVisualizer::direction(const Entity* parent, double t) const
{
    // We must return a unit vector; initialize dir to some arbitrary
    // unit vector in case the body direction is undefined.
    // TODO: Handle this better, by adding some means of reporting that
    // the direction is undefined.
    Vector3d dir(Vector3d::UnitX());

    if (parent && m_body.isValid())
    {
        Vector3d v = m_body->position(t) - parent->position(t);
        if (!v.isZero())
        {
            dir = v.normalized();
        }
    }

    return dir;
}
