/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "MultiConeSensorGeometry.h"
#include "Material.h"
#include "RenderContext.h"
#include "WorldGeometry.h"
#include "Intersect.h"

using namespace vesta;
using namespace Eigen;
using namespace std;


/** Create a new sensor frustum. The default settings are:
  *   angles: 5 degrees
  *   opacity: 100%
  *   color: white
  *   grid opacity: 15%
  *   limit cone angle: 180 degrees
  */
MultiConeSensorGeometry::MultiConeSensorGeometry() :
       m_orientation(Quaterniond::Identity()),
       m_range(1.0),
       m_opacity(1.0f),
       m_footprintOpacity(1.0f),
       m_gridOpacity(0.15f),
       m_limitConeAngle(vesta::PI)
{
   setClippingPolicy(Geometry::SplitToPreventClipping);
}


MultiConeSensorGeometry::~MultiConeSensorGeometry()
{
}


// Calculate the points of intersection between a line and a cone with its
// vertex at the origin. The cone is defined by the specified matrix, which
// is computed as:
//     A * At - I * cos(theta/2)^2
//
// Where:
//     A is the cone axis vector
//     At is the transpose of A
//     theta is the cone's vertex angle
//
// The line is defined by linePoint and lineDirection, with a point X(t) on
// the line given by linePoint + t * lineDirection.
static bool
TestLineConeIntersection(const Vector3d& linePoint,
                         const Vector3d& lineDirection,
                         const Matrix3d& coneMatrix,
                         double* t0,
                         double* t1)
{
    // Solve quadratic formula to find points of intersection with
    // the cone
    double a = (lineDirection.transpose() * coneMatrix * lineDirection)[0];
    double b = 2.0 * (lineDirection.transpose() * coneMatrix * linePoint)[0];
    double c = (linePoint.transpose() * coneMatrix * linePoint)[0] - 0.0;
    double disc = b * b - 4 * a * c;

    if (disc > 0.0)
    {
        double sdisc = sqrt(disc);
        if (t0)
        {
            *t0 = (-b - sdisc) / (2 * a);
        }

        if (t1)
        {
            *t1 = (-b + sdisc) / (2 * a);
        }

        return true;
    }
    else
    {
        return false;
    }
}


// Calculate the points of intersection between a ray and a cone with its
// vertex at the origin. The cone is defined by the specified matrix, which
// is computed as:
//     A * At - I * cos(theta/2)^2
//
// Where:
//     A is the cone axis vector
//     At is the transpose of A
//     theta is the cone's vertex angle
//
// The ray is defined by origin and direction, with a point X(t) on
// the ray given by origin + t * direction, t >= 0.
static bool
TestRayConeIntersection(const Vector3d& linePoint,
                        const Vector3d& lineDirection,
                        const Matrix3d& coneMatrix,
                        double* t)
{
    // Solve quadratic formula to find points of intersection with
    // the cone
    double a = (lineDirection.transpose() * coneMatrix * lineDirection)[0];
    double b = 2.0 * (lineDirection.transpose() * coneMatrix * linePoint)[0];
    double c = (linePoint.transpose() * coneMatrix * linePoint)[0] - 0.0;
    double disc = b * b - 4 * a * c;

    if (disc > 0.0)
    {
        double sdisc = sqrt(disc);
        double t0 = (-b - sdisc) / (2 * a);
        double t1 = (-b + sdisc) / (2 * a);

        if (t0 > 0.0)
        {
            if (t)
            {
                *t = t0;
            }
            return true;
        }
        else if (t1 > 0.0)
        {
            if (t)
            {
                *t = t1;
            }
            return true;
        }
        else
        {
            // Intersection is behind the origin
            return false;
        }
    }
    else
    {
        return false;
    }
}


/** Render the sensor frustum.
  */
void
MultiConeSensorGeometry::render(RenderContext& rc,
                                double currentTime) const
{
#ifndef VESTA_NO_IMMEDIATE_MODE_3D
    Material material;
    material.setOpacity(m_opacity);

    rc.setVertexInfo(VertexSpec::Position);
    rc.bindMaterial(&material);

    if (source() && target())
    {
        Vector3d p = target()->position(currentTime) - source()->position(currentTime);

        // Get the position of the source in the local coordinate system of the target
        Matrix3d targetRotation = target()->orientation(currentTime).conjugate().toRotationMatrix();
        Vector3d p2 = targetRotation * -p;

        bool ellipsoidalTarget = false;
        Vector3d targetSemiAxes = Vector3d::Ones();
        if (target()->geometry() && target()->geometry()->isEllipsoidal())
        {
            ellipsoidalTarget = true;
            targetSemiAxes = target()->geometry()->ellipsoid().semiAxes();
        }

        Quaterniond rotation = source()->orientation(currentTime);
        bool showInside = false;

        rc.pushModelView();
        rc.rotateModelView(rotation.cast<float>().conjugate());

        // Axis of the limit cone in ICRF frame
        Vector3d limitConeAxis = (rotation * m_orientation) * Vector3d::UnitZ();
        double cosLimitConeAngle = cos(m_limitConeAngle / 2.0);

        // The matrix A defines the limit cone. A point X lies on the limit cone surface
        // when Xt * A * X = 0, where Xt is the transpose of X
        Matrix3d limitConeMatrix = limitConeAxis * limitConeAxis.transpose() - Matrix3d::Identity() * pow(cos(m_limitConeAngle / 2.0), 2.0);

        for (vector<SensorCone>::const_iterator iter = m_cones.begin(); iter != m_cones.end(); ++iter)
        {
            const SensorCone& cone = *iter;

            Quaterniond coneRotation = AngleAxisd(cone.m_azimuth, Vector3d::UnitZ()) * AngleAxisd(cone.m_elevation, Vector3d::UnitX());

            Matrix3d m = (rotation * m_orientation * coneRotation).toRotationMatrix();
            Vector3d coneAxis = m * Vector3d::UnitZ();
            Vector3d coneBaseCenter = coneAxis * cos(cone.m_coneAngle / 2.0);

            double baseSize = tan(cone.m_coneAngle / 2.0);

            material.setDiffuse(Spectrum(cone.m_color.x(), cone.m_color.y(), cone.m_color.z()));

            // Only draw the beam cone when at least some part of it lies within the limit cone
            bool beamOutsideLimitCone = cone.m_elevation - cone.m_coneAngle / 2.0 > m_limitConeAngle / 2.0;
            bool beamIntersectsLimitCone = cone.m_elevation + cone.m_coneAngle / 2.0 > m_limitConeAngle / 2.0;

            if (!beamOutsideLimitCone)
            {
                m_frustumPoints.clear();

                // Compute 'center'. This is normally the beam cone center. But, if the
                // beam cone intersects the limit cone, we''ll adjust the center so that
                // it lies within the region of the beam cone that lies *inside* the
                // limit cone.
                Vector3d center = coneBaseCenter;
                if (beamIntersectsLimitCone)
                {
                    // Adjust the center when the beam cone intersects the limit
                    // cone. We'll use a point midway between the inner edge of the
                    // beam and the limit cone.
                    Vector3d r = m * Vector3d(0.0, baseSize, 1.0).normalized();
                    Vector3d d = (r - coneBaseCenter);

                    double t0 = 0.0;
                    double t1 = 0.0;
                    TestLineConeIntersection(r, d, limitConeMatrix, &t0, &t1);
                    center = r + d * (max(t0, t1) * 0.5);
                }

                const unsigned int sideDivisions = 24;
                const unsigned int sections = 4 * sideDivisions;
                for (unsigned int i = 0; i < sections; ++i)
                {
                    double t = (double) i / (double) sections;
                    double theta = 2 * PI * t;
                    Vector3d r = m * Vector3d(baseSize * cos(theta), baseSize * sin(theta), 1.0).normalized();

                    // If the point on the beam cone base lies outside the limit cone,
                    // we trim so that it lies on the limit cone
                    if (r.dot(limitConeAxis) < cosLimitConeAngle)
                    {
                        Vector3d rayOrigin = center;
                        Vector3d rayDirection = (r - center);

                        // Compute the intersection of ray from the center of the beam cone with
                        // the limit cone.
                        double t = 0.0;
                        TestRayConeIntersection(rayOrigin, rayDirection, limitConeMatrix, &t);

                        r = center + t * rayDirection;
                    }

                    double intersectDistance = m_range;
                    if (TestRayEllipsoidIntersection(p2, targetRotation * r, targetSemiAxes, &intersectDistance))
                    {
                        // Reduce the intersect distance slightly to reduce depth precision problems
                        // when drawing the sensor footprint on a planet surface.
                        intersectDistance *= 0.9999;
                    }
                    m_frustumPoints.push_back(r * min(m_range, intersectDistance));
                }

                if (m_opacity > 0.0f)
                {
                    // Draw the frustum
                    material.setOpacity(m_opacity);
                    rc.bindMaterial(&material);

                    if (showInside)
                    {
                        glDisable(GL_CULL_FACE);
                    }

                    glBegin(GL_TRIANGLE_FAN);
                    glVertex3d(0.0, 0.0, 0.0);
                    for (int i = (int) m_frustumPoints.size() - 1; i >= 0; --i)
                    {
                        glVertex3dv(m_frustumPoints[i].data());
                    }
                    glVertex3dv(m_frustumPoints.back().data());
                    glEnd();

                    glEnable(GL_CULL_FACE);
                }

                if (m_footprintOpacity > 0.0f)
                {
                    material.setOpacity(1.0f);
                    rc.bindMaterial(&material);

                    glBegin(GL_LINE_STRIP);
                    for (unsigned int i = 0; i < m_frustumPoints.size(); ++i)
                    {
                        glVertex3dv(m_frustumPoints[i].data());
                    }
                    glVertex3dv(m_frustumPoints[0].data());
                    glEnd();
                }

                if (m_gridOpacity > 0.0f)
                {
                    // Draw grid lines
                    unsigned int ringCount = 8;
                    unsigned int rayCount = 8;

                    material.setOpacity(m_gridOpacity);
                    rc.bindMaterial(&material);

                    for (unsigned int i = 1; i < ringCount; ++i)
                    {
                        double t = (double) i / (double) ringCount;
                        glBegin(GL_LINE_LOOP);
                        for (unsigned int j = 0; j < m_frustumPoints.size(); ++j)
                        {
                            Vector3d v = m_frustumPoints[j] * t;
                            glVertex3dv(v.data());
                        }
                        glEnd();
                    }

                    unsigned int rayStep = sections / rayCount;

                    glBegin(GL_LINES);
                    for (unsigned int i = 0; i < sections; i += rayStep)
                    {
                        glVertex3d(0.0, 0.0, 0.0);
                        glVertex3dv(m_frustumPoints[i].data());
                    }
                    glEnd();
                }
            }
        }

        rc.popModelView();
    }
#endif
}


void
MultiConeSensorGeometry::addBeam(double elevation, double azimuth, double coneAngle, const Spectrum& color)
{
    SensorCone cone;
    cone.m_coneAngle = float(coneAngle);
    cone.m_elevation = float(elevation);
    cone.m_azimuth = float(azimuth);
    cone.m_color = Vector3f(color.red(), color.green(), color.blue());

    m_cones.push_back(cone);
}
