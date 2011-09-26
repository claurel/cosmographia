/*
 * $Revision: 624 $ $Date: 2011-09-26 14:20:33 -0700 (Mon, 26 Sep 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Visualizer.h"
#include "Entity.h"
#include "PickContext.h"

using namespace vesta;
using namespace Eigen;


/** Create a new visualizer with the specified geometry. The newly constructed
  * visualizer is visible by default.
  */
Visualizer::Visualizer(Geometry* geometry) :
    m_visible(true),
    m_geometry(geometry),
    m_depthAdjustment(NoAdjustment)
{
}


Visualizer::~Visualizer()
{
}


/** Get the orientation of this visualizer within the global coordinate
  * system. Subclasses should override this method. The default implementation
  * returns the identity orientation (i.e. no rotation with respect to the
  * fundamental coordinate system.)
  */
Eigen::Quaterniond
Visualizer::orientation(const Entity* /* parent */, double /* t */) const
{
    return Quaterniond::Identity();
}


/** Return true if the given ray intersects the visualizer. The ray origin
  * and direction are in the local coordinate system of the body that the
  * visualizer is attached to. The pixel angle parameter is required for
  * testing intersections with visualizers such as labels that have a fixed
  * size on screen.
  *
  * \param pickOrigin origin of the ray
  * \param pickDirection normalized ray direction
  * \param pixelAngle angle in radians subtended by a pixel
  */
bool
Visualizer::rayPick(const PickContext* pc, const Vector3d& pickOrigin, double t) const
{
    return handleRayPick(pc, pickOrigin, t);
}


/** handleRayPick is called to test whether a visualizer is intersected
  * by the pick geometry. It should be overridden by any pickable visualizer.
  * This method supercedes the version that accepts two vector parameters; for
  * compatibility with older Visualizer code, the default behavior is to call
  * the older version of handleRayPick.
  *
  * \param pc a pick context that specifies the
  * \param pixelAngle angle in radians subtended by one pixel of the viewport
  * \param t the current time
  */
bool
Visualizer::handleRayPick(const PickContext* pc, const Vector3d& pickOrigin, double /* t */) const
{
    return handleRayPick(pickOrigin, pc->pickDirection(), pc->pixelAngle());
}


/** handleRayPick is called to test whether a visualizer is intersected
  * by a pick ray. It should be overridden by any pickable visualizer. A
  * new version of handlePickRay accepts a pick context and thus allows
  * more flexibility. New Visualizer subclasses should override this version
  * of handlePickRay instead.
  *
  * \param pickOrigin origin of the pick ray in local coordinates
  * \param pickDirection the normalized pick ray direction, in local coordinates
  * \param pixelAngle angle in radians subtended by one pixel of the viewport
  */
bool
Visualizer::handleRayPick(const Eigen::Vector3d& pickOrigin,
                          const Eigen::Vector3d& pickDirection,
                          double pixelAngle) const
{
    if (!m_geometry.isNull())
    {
        // For geometry with a fixed apparent size, test to see if the pick ray is
        // within apparent size / 2 pixels of the center.
        if (m_geometry->hasFixedApparentSize())
        {
            double cosAngle = pickDirection.dot(-pickOrigin.normalized());
            if (cosAngle > 0.0)
            {

                if (cosAngle >= 1.0 || acos(cosAngle) < m_geometry->apparentSize() / 2.0 * pixelAngle)
                {
                    return true;
                }
            }
        }
    }

    return false;
}
