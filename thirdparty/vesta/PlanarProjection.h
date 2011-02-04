/*
 * $Revision: 421 $ $Date: 2010-08-11 14:35:48 -0700 (Wed, 11 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PLANAR_PROJECTION_H_
#define _VESTA_PLANAR_PROJECTION_H_

#include "Frustum.h"
#include <Eigen/Core>


namespace vesta
{

class PlanarProjection
{
public:
    enum ProjectionType
    {
        Perspective = 0,
        Orthographic = 1,
    };

    enum Chirality
    {
        LeftHanded  = 0,
        RightHanded = 1,
    };

    PlanarProjection(ProjectionType type, float left, float right, float bottom, float top, float nearDistance, float farDistance);

    /** Get the projection type (either Perspective or Orthographic)
      */
    ProjectionType type() const
    {
        return m_type;
    }

    /** Get the coordinate of the left vertical clipping plane.
      */
    float left() const
    {
        return m_left;
    }

    /** Get the coordinate of the right vertical clipping plane.
      */
    float right() const
    {
        return m_right;
    }

    /** Get the coordinate of the bottom horizontal clipping plane.
      */
    float bottom() const
    {
        return m_bottom;
    }

    /** Get the coordinate of the top horizontal clipping plane.
      */
    float top() const
    {
        return m_top;
    }

    /** Get the distance to the front clipping plane.
      */
    float nearDistance() const
    {
        return m_nearDistance;
    }

    /** Get the distance to the rear clipping plane.
      */
    float farDistance() const
    {
        return m_farDistance;
    }

    Chirality chirality() const
    {
        if ((m_right < m_left) ^ (m_top < m_bottom))
        {
            return LeftHanded;
        }
        else
        {
            return RightHanded;
        }
    }

    Eigen::Matrix4f matrix() const;

    Frustum frustum() const;

    float fovY() const;
    float fovX() const;
    float fovDiagonal() const;
    float aspectRatio() const;

    PlanarProjection slice(float nearDistance, float farDistance) const;

    static PlanarProjection CreatePerspective(float fovY, float aspectRatio, float nearDistance, float farDistance);
    static PlanarProjection CreatePerspectiveLH(float fovY, float aspectRatio, float nearDistance, float farDistance);
    static PlanarProjection CreateOrthographic(float left, float right, float bottom, float top, float nearDistance, float farDistance);
    static PlanarProjection CreateOrthographic2D(float left, float right, float bottom, float top);

private:
    ProjectionType m_type;
    float m_left;
    float m_right;
    float m_bottom;
    float m_top;
    float m_nearDistance;
    float m_farDistance;
};

};

#endif // _VESTA_PLANAR_PROJECTION_H_
