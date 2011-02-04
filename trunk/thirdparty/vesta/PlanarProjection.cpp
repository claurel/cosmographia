/*
 * $Revision: 421 $ $Date: 2010-08-11 14:35:48 -0700 (Wed, 11 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "PlanarProjection.h"
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


/** Create a new planar projection. When called with type == Perspective, the constructor will create
  * a projection identical to OpenGL's glFrustum() function. With type == Orthographic, the projection
  * is equivalent to the one created with glOrtho().
  *
  * \param type either Perspective or Orthographic
  * \param left coordinate of the left vertical clipping plane
  * \param right coordinate of the right vertical clipping plane
  * \param bottom coordinate of the bottom horizontal clipping plane
  * \param top coordinate of the top horizontal clipping plane
  * \param nearDistance distance to the near clipping plane (always positive)
  * \param farDistance distance to the far clipping plane (always positive)
  */
PlanarProjection::PlanarProjection(ProjectionType type, float left, float right, float bottom, float top, float nearDistance, float farDistance) :
    m_type(type),
    m_left(left),
    m_right(right),
    m_bottom(bottom),
    m_top(top),
    m_nearDistance(nearDistance),
    m_farDistance(farDistance)
{
}


/** Get the 4x4 matrix that will apply the projection to a homogeneous coordinate.
  */
Matrix4f
PlanarProjection::matrix() const
{
    Matrix4f m;
    float x = m_right - m_left;
    float y = m_top - m_bottom;
    float z = m_farDistance - m_nearDistance;

    switch (m_type)
    {
    case Perspective:
        {
            float near2 = m_nearDistance * 2.0f;

            m << near2 / x, 0.0f,      (m_right + m_left) / x, 0.0f,
                 0.0f,      near2 / y, (m_top + m_bottom) / y, 0.0f,
                 0.0f,      0.0f,      -(m_farDistance + m_nearDistance) / z, -(2.0f * m_farDistance * m_nearDistance) / z,
                 0.0f,      0.0f,      -1.0f,                  0.0f;
        }
        break;

    case Orthographic:
        {
            m << 2.0f / x, 0.0f,      0.0f,     -(m_right + m_left) / x,
                 0.0f,     2.0f / y,  0.0f,     -(m_top + m_bottom) / y,
                 0.0f,     0.0f,     -2.0f / z, -(m_farDistance + m_nearDistance) / z,
                 0.0f,     0.0f,      0.0f,     1.0f;

        }
        break;

    default:
        m = Matrix4f::Identity();
        break;
    }

    return m;
}


/** Get the viewing frustum for this projection. The frustum volume will be a box for orthographic
  * projections and a truncated rectangular pyramid for perspective projections.
  */
Frustum
PlanarProjection::frustum() const
{
    Frustum f;

    f.nearZ = m_nearDistance;
    f.farZ = m_farDistance;

    float signX = m_left < m_right ? 1.0f : -1.0f;
    float signY = m_bottom < m_top ? 1.0f : -1.0f;

    switch (m_type)
    {
    case Perspective:
        f.planeNormals[0] = Vector3d( m_nearDistance, 0.0,  m_left * signX).normalized();
        f.planeNormals[1] = Vector3d(-m_nearDistance, 0.0, -m_right * signX).normalized();
        f.planeNormals[2] = Vector3d(0.0,  m_nearDistance,  m_bottom * signY).normalized();
        f.planeNormals[3] = Vector3d(0.0, -m_nearDistance, -m_top * signY).normalized();
        break;

    case Orthographic:
        f.planeNormals[0] = -Vector3d::UnitX();
        f.planeNormals[1] =  Vector3d::UnitX();
        f.planeNormals[2] = -Vector3d::UnitY();
        f.planeNormals[3] =  Vector3d::UnitY();
        break;
    }

    return f;
}


/** Get the ratio of width to height.
  */
float
PlanarProjection::aspectRatio() const
{
    return (m_right - m_left) / (m_top - m_bottom);
}


/** Get the vertical field of view in radians.
  */
float
PlanarProjection::fovY() const
{
    return atan((abs(m_top - m_bottom)) * 0.5f / m_nearDistance) * 2.0f;
}


/** Get the horizontal field of view in radians.
  */
float
PlanarProjection::fovX() const
{
    return atan((abs(m_right - m_left)) * 0.5f / m_nearDistance) * 2.0f;
}


/** Get the diagonal field of view in radians.
  */
float
PlanarProjection::fovDiagonal() const
{
    float diagonal = Vector2f(m_right - m_left, m_top - m_bottom).norm();
    return atan(diagonal * 0.5f / m_nearDistance) * 2.0f;
}


/** Create a new projection that is identical to this projection except for the near
  * and far planes.
  */
PlanarProjection
PlanarProjection::slice(float nearDistance, float farDistance) const
{
    if (m_type == Orthographic)
    {
        return PlanarProjection(m_type, m_left, m_right, m_bottom, m_top, nearDistance, farDistance);
    }
    else
    {
        double nearRatio = double(nearDistance) / double(m_nearDistance);
        return PlanarProjection(m_type,
                                float(m_left * nearRatio), float(m_right * nearRatio),
                                float(m_bottom * nearRatio), float(m_top * nearRatio),
                                nearDistance, farDistance);
    }
}


/** Create a right-handed symmetric perspective projection. This gives the
  * same projections as calling the OpenGL function gluPerspective(fovY, aspectRatio, near, far)
  *
  * \param fovY the vertical field of view in radians
  * \param aspectRatio ratio of width to height
  * \param nearDistance distance to the near plane
  * \param farDistance distance to the far plane
  */
PlanarProjection
PlanarProjection::CreatePerspective(float fovY, float aspectRatio, float nearDistance, float farDistance)
{
    float y = tan(0.5f * fovY) * nearDistance;
    float x = y * aspectRatio;
    return PlanarProjection(Perspective, -x, x, -y, y, nearDistance, farDistance);
}


/** Create a left-handed symmetric perspective projection.
  *
  * \param fovY the vertical field of view in radians
  * \param aspectRatio ratio of width to height
  * \param nearDistance distance to the near plane
  * \param farDistance distance to the far plane
  */
PlanarProjection
PlanarProjection::CreatePerspectiveLH(float fovY, float aspectRatio, float nearDistance, float farDistance)
{
    PlanarProjection proj = CreatePerspective(fovY, aspectRatio, nearDistance, farDistance);
    proj.m_left = -proj.m_left;
    proj.m_right = -proj.m_right;
    return proj;
}


/** Create an orthographic projection. This gives the same projection as calling
  * the OpenGL function glOrtho(left, right, bottom, top, near, far)
  */
PlanarProjection
PlanarProjection::CreateOrthographic(float left, float right, float bottom, float top, float nearDistance, float farDistance)
{
    return PlanarProjection(Orthographic, left, right, bottom, top, nearDistance, farDistance);
}


/** Create an orthographic projection appropriate for 2D rendering (i.e. on the z = 0 plane).
  * This is equivalent to calling CreateOrthographic with near = -1 and far = +1
  */
PlanarProjection
PlanarProjection::CreateOrthographic2D(float left, float right, float bottom, float top)
{
    return CreateOrthographic(left, right, bottom, top, -1.0f, 1.0f);
}
