/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "SensorVisualizer.h"
#include "RenderContext.h"
#include "WorldGeometry.h"
#include "Units.h"
#include "Intersect.h"
#include "Debug.h"
#include "GL/glew.h"
#include <vector>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


namespace vesta
{

// The SensorFrustumGeometry class is used exclusively by SensorVisualizer, and
// thus is not currently available to other classes.
//
// The sensor geometry has three parts:
//    Footprint - a ring or polygon showing the intersection of the sensor frustum
//                with the target body.
//    Frustum   - bounding surface of the frustum, truncated at the intersection with
//                the target body.
//    Grid      - grid lines drawn within the frustum to provide additional visual
//                about its three dimensional shape.
class SensorFrustumGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    SensorFrustumGeometry() :
        m_orientation(Quaterniond::Identity()),
        m_range(1.0),
        m_color(1.0f, 1.0f, 1.0f),
        m_opacity(1.0f),
        m_footprintOpacity(1.0f),
        m_gridOpacity(0.15f),
        m_frustumShape(SensorVisualizer::Elliptical),
        m_frustumHorizontalAngle(toRadians(5.0)),
        m_frustumVerticalAngle(toRadians(5.0))
    {
        setClippingPolicy(Geometry::SplitToPreventClipping);
    }

    ~SensorFrustumGeometry()
    {
    }

    virtual void render(RenderContext& rc,
                        double currentTime) const;

    float boundingSphereRadius() const
    {
        return float(m_range);
    }

    virtual bool isOpaque() const
    {
        return m_opacity > 0.99f;
    }

    Quaterniond sensorOrientation() const
    {
        return m_orientation;
    }

    void setSensorOrientation(const Eigen::Quaterniond& orientation)
    {
        m_orientation = orientation;
    }

    double range() const
    {
        return m_range;
    }

    void setRange(double range)
    {
        m_range = range;
    }

    Spectrum color() const
    {
        return m_color;
    }

    void setColor(const Spectrum& color)
    {
        m_color = color;
    }

    float opacity() const
    {
        return m_opacity;
    }

    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    Entity* source() const
    {
        return m_source.ptr();
    }

    void setSource(Entity* source)
    {
        m_source = source;
    }

    Entity* target() const
    {
        return m_target.ptr();
    }

    void setTarget(Entity* target)
    {
        m_target = target;
    }

    SensorVisualizer::FrustumShape frustumShape() const
    {
        return m_frustumShape;
    }

    void setFrustumShape(SensorVisualizer::FrustumShape shape)
    {
        m_frustumShape = shape;
    }

    void setFrustumAngles(double horizontal, double vertical)
    {
        m_frustumHorizontalAngle = horizontal;
        m_frustumVerticalAngle = vertical;
    }

private:
    Quaterniond m_orientation;

    double m_range;
    Spectrum m_color;
    float m_opacity;
    float m_footprintOpacity;
    float m_gridOpacity;
    counted_ptr<Entity> m_source;
    counted_ptr<Entity> m_target;
    SensorVisualizer::FrustumShape m_frustumShape;
    double m_frustumHorizontalAngle;
    double m_frustumVerticalAngle;

    // TODO: Eliminate this once streaming vertex array is exposed by
    // RenderContext.
    mutable vector<Vector3d> m_frustumPoints;
};

}


/** Create a new SensorVisualizer.
  */
SensorVisualizer::SensorVisualizer() :
    Visualizer(NULL)
{
    m_frustum = new SensorFrustumGeometry();
    setGeometry(m_frustum.ptr());
}


SensorVisualizer::~SensorVisualizer()
{
}


/** Set the sensor source; normally, this will be the object
  * that the visualizer is attached to.
  */
void
SensorVisualizer::setSource(Entity* source)
{
    m_frustum->setSource(source);
}


Entity*
SensorVisualizer::source() const
{
    return m_frustum->source();
}


void
SensorVisualizer::setTarget(Entity* target)
{
    m_frustum->setTarget(target);
}


Entity*
SensorVisualizer::target() const
{
    return m_frustum->target();
}


/** Get the sensor orientation. By default, the sensor is oriented so that
  * it points along the source body's z-axis, with the horizontal axis +x and
  * the vertical axis +y.
  */
Quaterniond
SensorVisualizer::sensorOrientation() const
{
    return m_frustum->sensorOrientation();
}


/** Set the sensor orientation. By default, the sensor is oriented so that
  * it points along the source body's z-axis, with the horizontal axis +x and
  * the vertical axis +y.
  */
void
SensorVisualizer::setSensorOrientation(const Quaterniond& orientation)
{
    m_frustum->setSensorOrientation(orientation);
}


/** Get the sensor range in kilometers.
  */
double SensorVisualizer::range() const
{
    return m_frustum->range();
}


/** Set the sensor range in kilometers. The sensor geometry extends from
  * the position of the source object out to the range unless it is blocked
  * by the target body.
  */
void
SensorVisualizer::setRange(double range)
{
    m_frustum->setRange(range);
}


Spectrum
SensorVisualizer::color() const
{
    return m_frustum->color();
}


void
SensorVisualizer::setColor(const Spectrum& color)
{
    m_frustum->setColor(color);
}


float
SensorVisualizer::opacity() const
{
    return m_frustum->opacity();
}


void
SensorVisualizer::setOpacity(float opacity)
{
    m_frustum->setOpacity(opacity);
}


/** Get the shape of the sensor frustum.
  */
SensorVisualizer::FrustumShape
SensorVisualizer::frustumShape() const
{
    return m_frustum->frustumShape();
}


/** Set the shape of the sensor frustum. Currently, only
  * elliptical frustums are supported.
  */
void
SensorVisualizer::setFrustumShape(FrustumShape shape)
{
    m_frustum->setFrustumShape(shape);
}


/** Set the frustum angles.
  */
void
SensorVisualizer::setFrustumAngles(double horizontal, double vertical)
{
    m_frustum->setFrustumAngles(horizontal, vertical);
}


// Implementation of Visualizer::orientation()
Eigen::Quaterniond
SensorVisualizer::orientation(const Entity* /* parent */, double /* t */) const
{
    return Quaterniond::Identity();
}


// Implementation of Visualizer::render()
void
SensorFrustumGeometry::render(RenderContext& rc,
                              double currentTime) const
{
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

        // Special handling for ellipsoidal objects, i.e. planets.
        // TODO: Should have a cleaner solution here than a dynamic_cast
        bool ellipsoidalTarget = false;
        Vector3d targetSemiAxes = Vector3d::Ones();
        if (dynamic_cast<WorldGeometry*>(target()->geometry()))
        {
            ellipsoidalTarget = true;
            targetSemiAxes = dynamic_cast<WorldGeometry*>(target()->geometry())->ellipsoidAxes().cast<double>() / 2.0;
        }

        Matrix3d m = (source()->orientation(currentTime) * m_orientation).toRotationMatrix();

        double horizontalSize = tan(m_frustumHorizontalAngle / 2.0);
        double verticalSize = tan(m_frustumVerticalAngle / 2.0);

        bool showInside = false;

        m_frustumPoints.clear();

        const unsigned int sections = 48;
        for (unsigned int i = 0; i < sections; ++i)
        {
            double t = (double) i / (double) sections;
            double theta = 2 * PI * t;
            Vector3d r = Vector3d(horizontalSize * cos(theta), verticalSize * sin(theta), 1.0).normalized();
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
            for (int i = (int) m_frustumPoints.size() - 1; i > 0; --i)
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

