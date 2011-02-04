/*
 * $Revision: 420 $ $Date: 2010-08-10 17:01:21 -0700 (Tue, 10 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_FRUSTUM_H_
#define _VESTA_FRUSTUM_H_

#include "BoundingSphere.h"
#include <Eigen/Core>


namespace vesta
{

/** Frustum is six-sided convex volume containing the region visible through
  * a rectangular viewport. The view is assumed to be pointed along the -z
  * axis. Two of the bounding planes are z = -nearZ and z = -farZ. The other
  * four bounding planes are the planes containing the origin with normals
  * given in the planeNormals array.
  */
class Frustum
{
public:
    Frustum() :
       nearZ(0.0f),
       farZ(1.0f)
    {
    }

    /** Return true if the sphere intersects the frustum, false
      * if not.
      */
    bool intersects(const BoundingSphere<float>& sphere) const
    {
        // Test the near and far planes
        if (sphere.center().z() - sphere.radius() > -nearZ ||
            sphere.center().z() + sphere.radius() < -farZ)
        {
            return false;
        }

        for (unsigned int i = 0; i < 4; ++i)
        {
            if (planeNormals[i].cast<float>().dot(sphere.center()) <= -sphere.radius())
            {
                return false;
            }
        }

        return true;
    }

    // TODO: make the implementation of this class opaque
public:
    float nearZ;
    float farZ;
    Eigen::Vector3d planeNormals[4];
};

}

#endif // _VESTA_FRUSTUM_H_
