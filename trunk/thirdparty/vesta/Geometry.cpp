/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Geometry.h"
#include "Intersect.h"

using namespace vesta;
using namespace Eigen;



/** Test whether this geometry is intersected by the given pick
  * ray. The pickOrigin and pickDirection are in the model space.
  *
  * \param pickOrigin origin of the pick ray in model space
  * \param pickDirection direction of the pick ray in model space (must be normalized)
  * \param clock time in seconds used for time-driven animation
  * \param distance filled in with the distance to the geometry if the ray hits
  */
bool
Geometry::rayPick(const Vector3d& pickOrigin,
                  const Vector3d& pickDirection,
                  double clock,
                  double* distance) const
{
    Vector3d c = Vector3d::Zero();
    if (TestRaySphereIntersection(pickOrigin, pickDirection, c, double(boundingSphereRadius())))
    {
        return handleRayPick(pickOrigin, pickDirection, clock, distance);
    }
    else
    {
        return false;
    }
}
