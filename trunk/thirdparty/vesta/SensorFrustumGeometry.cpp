/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "SensorFrustumGeometry.h"
#include "Material.h"
#include "RenderContext.h"
#include "Intersect.h"

using namespace vesta;
using namespace Eigen;
using namespace std;


/** Create a new sensor frustum. The default settings are:
  *   shape: elliptical
  *   angles: 5 degrees
  *   opacity: 100%
  *   color: white
  *   grid opacity: 15%
  */
SensorFrustumGeometry::SensorFrustumGeometry() :
       m_orientation(Quaterniond::Identity()),
       m_range(1.0),
       m_color(1.0f, 1.0f, 1.0f),
       m_opacity(1.0f),
       m_footprintOpacity(1.0f),
       m_gridOpacity(0.15f),
       m_frustumShape(Elliptical),
       m_frustumHorizontalAngle(toRadians(5.0)),
       m_frustumVerticalAngle(toRadians(5.0))
{
   setClippingPolicy(Geometry::SplitToPreventClipping);
}


SensorFrustumGeometry::~SensorFrustumGeometry()
{
}


/** Render the sensor frustum.
  */
void
SensorFrustumGeometry::render(RenderContext& rc,
                              double currentTime) const
{
#ifndef VESTA_OGLES2
    Material material;
    material.setDiffuse(m_color);
    material.setOpacity(m_opacity);

    rc.setVertexInfo(VertexSpec::Position);
    rc.bindMaterial(&material);

    if (source() && target())
    {
        Vector3d p = target()->position(currentTime) - source()->position(currentTime);

        // Get the position of the source in the local coordinate system of the target
        Matrix3d targetRotation = target()->orientation(currentTime).conjugate().toRotationMatrix();
        Vector3d p2 = targetRotation * -p;

        // Special handling for ellipsoidal target objects, i.e. planets.
        bool ellipsoidalTarget = false;
        Vector3d targetSemiAxes = Vector3d::Ones();
        if (target()->geometry() && target()->geometry()->isEllipsoidal())
        {
            ellipsoidalTarget = true;
            targetSemiAxes = target()->geometry()->ellipsoid().semiAxes();
        }

        Quaterniond rotation = source()->orientation(currentTime);
        Matrix3d m = (rotation * m_orientation).toRotationMatrix();

        double horizontalSize = tan(m_frustumHorizontalAngle / 2.0);
        double verticalSize = tan(m_frustumVerticalAngle / 2.0);

        bool showInside = false;

        rc.pushModelView();
        rc.rotateModelView(rotation.cast<float>().conjugate());

        m_frustumPoints.clear();

        const unsigned int sideDivisions = 12;
        const unsigned int sections = 4 * sideDivisions;
        for (unsigned int i = 0; i < sections; ++i)
        {
            Vector3d r;
            if (frustumShape() == Elliptical)
            {
                double t = (double) i / (double) sections;
                double theta = 2 * PI * t;

                r = Vector3d(horizontalSize * cos(theta), verticalSize * sin(theta), 1.0).normalized();
            }
            else
            {
                if (i < sideDivisions)
                {
                    double t = i / double(sideDivisions);
                    r = Vector3d((t - 0.5) * horizontalSize, -verticalSize * 0.5, 1.0).normalized();
                }
                else if (i < sideDivisions * 2)
                {
                    double t = (i - sideDivisions) / double(sideDivisions);
                    r = Vector3d(horizontalSize * 0.5, (t - 0.5) * verticalSize, 1.0).normalized();
                }
                else if (i < sideDivisions * 3)
                {
                    double t = (i - sideDivisions * 2) / double(sideDivisions);
                    r = Vector3d((0.5 - t) * horizontalSize, verticalSize * 0.5, 1.0).normalized();
                }
                else
                {
                    double t = (i - sideDivisions * 3) / double(sideDivisions);
                    r = Vector3d(-horizontalSize * 0.5, (0.5 - t) * verticalSize, 1.0).normalized();
                }
            }
            r = m * r;

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
            unsigned int rayCount = frustumShape() == Rectangular ? 4 : 8;

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

        rc.popModelView();
    }
#endif
}
