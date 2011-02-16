/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_SENSOR_FRUSTUM_GEOMETRY_H_
#define _VESTA_SENSOR_FRUSTUM_GEOMETRY_H_

#include "Geometry.h"
#include "Units.h"
#include <Eigen/Core>
#include <Eigen/Geometry>


namespace vesta
{

/** SensorFrustumGeometry class is used by SensorVisualizer for drawing
 *  spacecraft sensor volumes.
 *
 *  The sensor geometry has three parts:
 *    Footprint - a ring or polygon showing the intersection of the sensor frustum
 *                with the target body.
 *    Frustum   - bounding surface of the frustum, truncated at the intersection with
 *                the target body.
 *    Grid      - grid lines drawn within the frustum to provide additional visual
 *                about its three dimensional shape.
 */
class SensorFrustumGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    enum FrustumShape
    {
        Elliptical = 0,
        Rectangular = 1,
    };

    SensorFrustumGeometry();
    ~SensorFrustumGeometry();

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

    Eigen::Quaterniond sensorOrientation() const
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

    FrustumShape frustumShape() const
    {
        return m_frustumShape;
    }

    void setFrustumShape(FrustumShape shape)
    {
        m_frustumShape = shape;
    }

    void setFrustumAngles(double horizontal, double vertical)
    {
        m_frustumHorizontalAngle = horizontal;
        m_frustumVerticalAngle = vertical;
    }

private:
    Eigen::Quaterniond m_orientation;

    double m_range;
    Spectrum m_color;
    float m_opacity;
    float m_footprintOpacity;
    float m_gridOpacity;
    counted_ptr<Entity> m_source;
    counted_ptr<Entity> m_target;
    FrustumShape m_frustumShape;
    double m_frustumHorizontalAngle;
    double m_frustumVerticalAngle;

    // TODO: Eliminate this once streaming vertex array is exposed by
    // RenderContext.
    mutable std::vector<Eigen::Vector3d> m_frustumPoints;
};

}

#endif // _VESTA_SENSOR_FRUSTUM_GEOMETRY_H_
