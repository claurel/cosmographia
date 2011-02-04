/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "NadirVisualizer.h"
#include "Entity.h"
#include "Arc.h"

using namespace vesta;
using namespace Eigen;


NadirVisualizer::NadirVisualizer(double size) :
    ArrowVisualizer(size)
{
}


NadirVisualizer::~NadirVisualizer()
{
}


Vector3d
NadirVisualizer::direction(const Entity* parent, double t) const
{
    // We must return a unit vector; initialize dir to some arbitrary
    // unit vector in case the nadir direction is undefined.
    // TODO: Handle this better, by adding some means of reporting that
    // the direction is undefined.
    Vector3d dir(Vector3d::UnitX());

    // TODO: The true nadir direction doesn't necessarily point toward
    // the center of central body (when the central body is not spherical.)

    Arc* arc = parent->chronology()->activeArc(t);
    if (arc)
    {
        Vector3d centerPosition(Vector3d::Zero());
        if (arc->center())
        {
            centerPosition = arc->center()->position(t);
        }

        Vector3d toCenter = centerPosition - parent->position(t);
        if (!toCenter.isZero())
        {
            dir = toCenter.normalized();
        }
    }

    return dir;
}
