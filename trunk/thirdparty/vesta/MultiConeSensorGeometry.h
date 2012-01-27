/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_MULTI_CONE_SENSOR_GEOMETRY_H_
#define _VESTA_MULTI_CONE_SENSOR_GEOMETRY_H_

#include "Geometry.h"
#include "Units.h"
#include <Eigen/Core>
#include <Eigen/Geometry>


namespace vesta
{

/** MultiConeSensorGeometry class is used by SensorVisualizer for drawing
 *  spacecraft sensor volumes with multiple conical beams. The beams are
 *  truncated by a limiting cone.
 *
 *  The limit cone and beam cones all share the same origin. The cones are
 *  defined in a local coordinate system where the vertex of the limit
 *  cone is the origin and the axis of the limit cone is the z-axis.
 *
 *  The geometry of each beam cone is defined by the following properties:
 *    - coneAngle
 *      the vertex angle of the cone
 *    - elevation
 *      the angle between the axis of the beam cone and the axis of limit cone
 *    - azimuth
 *      the rotation of the beam cone about the local z-axis (i.e. the angle
 *      of the beam cone axis projected into the xy-plane.)
 *
 *
 *  The sensor geometry has three parts:
 *    Footprint - a ring or polygon showing the intersection of the sensor frustum
 *                with the target body.
 *    Frustum   - bounding surface of the frustum, truncated at the intersection with
 *                the target body.
 *    Grid      - grid lines drawn within the frustum to provide additional visual
 *                about its three dimensional shape.
 */
class MultiConeSensorGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    MultiConeSensorGeometry();
    ~MultiConeSensorGeometry();

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

    void addBeam(double elevation, double azimuth, double coneAngle, const Spectrum& color);

    /** Get the vertex angle (in radians) of the limit cone/
      */
    double limitConeAngle() const
    {
        return m_limitConeAngle;
    }

    /** Set the vertex angle (in radians) of the limit cone.
      */
    void setLimitConeAngle(double radians)
    {
        m_limitConeAngle = radians;
    }

private:
    struct SensorCone
    {
        float m_coneAngle;
        float m_elevation;
        float m_azimuth;
        Eigen::Vector3f m_color;
    };

private:
    Eigen::Quaterniond m_orientation;

    double m_range;
    float m_opacity;
    float m_footprintOpacity;
    float m_gridOpacity;
    counted_ptr<Entity> m_source;
    counted_ptr<Entity> m_target;

    double m_limitConeAngle;

    std::vector<SensorCone> m_cones;

    mutable std::vector<Eigen::Vector3d> m_frustumPoints;
};

}

#endif // _VESTA_MULTI_CONE_SENSOR_GEOMETRY_H_
