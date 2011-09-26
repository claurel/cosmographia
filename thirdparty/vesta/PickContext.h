/*
 * $Revision: 609 $ $Date: 2011-04-29 12:30:42 -0700 (Fri, 29 Apr 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PICK_CONTEXT_H_
#define _VESTA_PICK_CONTEXT_H_

#include "PlanarProjection.h"
#include <Eigen/Core>
#include <Eigen/Geometry>


namespace vesta
{

class PickContext
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    PickContext();
    ~PickContext();

    /** Get the origin of the pick ray in the standard coordinate system.
      */
    Eigen::Vector3d pickOrigin() const
    {
        return m_pickOrigin;
    }

    /** Set the origin of the pick ray in the standard coordinate system.
      */
    void setPickOrigin(const Eigen::Vector3d& origin)
    {
        m_pickOrigin = origin;
    }

    /** Get the pick ray direction in standard coordinates. The direction
      * vector is normalized.
      */
    Eigen::Vector3d pickDirection() const
    {
        return m_pickDirection;
    }

    /** Set the pick ray direction in standard coordinates. The direction
      * vector should be normalized.
      */
    void setPickDirection(const Eigen::Vector3d& direction)
    {
        m_pickDirection = direction;
    }

    /** Get the camera projection.
      */
    const PlanarProjection& projection() const
    {
        return m_projection;
    }

    /** Set the camera projection.
      */
    void setProjection(const PlanarProjection& projection)
    {
        m_projection = projection;
    }

    /** Get the camera orientation.
      */
    Eigen::Quaterniond cameraOrientation() const
    {
        return m_cameraOrientation;
    }

    /** Set the camera orientation. This is necessary for picking screen aligned
      * geometry such as text labels and billboards.
      */
    void setCameraOrientation(const Eigen::Quaterniond& orientation)
    {
        m_cameraOrientation = orientation;
    }

    /** Set the angle in radians subtended by a single pixel (assumes square pixels)
     */
    float pixelAngle() const
    {
        return m_pixelAngle;
    }

    /** Get the angle in radians subtended by a single pixel (assumes square pixels)
     */
    void setPixelAngle(float pixelAngle)
    {
        m_pixelAngle = pixelAngle;
    }

private:
    Eigen::Vector3d m_pickOrigin;
    Eigen::Vector3d m_pickDirection;
    PlanarProjection m_projection;
    Eigen::Quaterniond m_cameraOrientation;
    float m_pixelAngle;
};

}

#endif // _VESTA_PICK_CONTEXT_H_
