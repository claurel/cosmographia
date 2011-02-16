/*
 * $Revision: 559 $ $Date: 2010-12-13 06:33:07 -0800 (Mon, 13 Dec 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "VelocityVisualizer.h"
#include "Entity.h"
#include "Arc.h"
#include "Trajectory.h"

using namespace vesta;
using namespace Eigen;


VelocityVisualizer::VelocityVisualizer(double size) :
    ArrowVisualizer(size)
{
    setLabelText("Velocity");
}


VelocityVisualizer::~VelocityVisualizer()
{
}


/** Returns the velocity direction within the trajectory frame
  * of the object to which the visualizer is attached.
  */
Vector3d
VelocityVisualizer::direction(const Entity* parent, double t) const
{
    Vector3d velocity = Vector3d::Zero();

    Arc* arc = parent->chronology()->activeArc(t);
    if (arc)
    {
        velocity = arc->trajectory()->state(t).velocity();
    }

    if (velocity.isZero())
    {
        // Still need to return a unit vector
        // TODO: Find a better solution here. When the velocity is zero,
        // the velocity vector proably shouldn't be shown.
        return Vector3d::UnitX();
    }
    else
    {
        return velocity.normalized();
    }
}
