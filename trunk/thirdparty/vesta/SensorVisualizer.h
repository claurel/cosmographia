/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_SENSOR_VISUALIZER_H_
#define _VESTA_SENSOR_VISUALIZER_H_

#include "Visualizer.h"
#include "Frame.h"
#include "Entity.h"
#include "Spectrum.h"


namespace vesta
{
class SensorFrustumGeometry;

class SensorVisualizer : public Visualizer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    enum FrustumShape
    {
        Elliptical = 0,
        Rectangular = 1,
    };

    SensorVisualizer();
    virtual ~SensorVisualizer();

    virtual Eigen::Quaterniond orientation(const Entity* parent, double t) const;

    Entity* source() const;
    void setSource(Entity* source);
    Entity* target() const;
    void setTarget(Entity* target);
    Spectrum color() const;
    void setColor(const Spectrum& color);
    float opacity() const;
    void setOpacity(float opacity);

    Eigen::Quaterniond sensorOrientation() const;
    void setSensorOrientation(const Eigen::Quaterniond& orientation);

    FrustumShape frustumShape() const;
    void setFrustumShape(SensorVisualizer::FrustumShape shape);
    void setFrustumAngles(double horizontal, double vertical);

    double range() const;
    void setRange(double range);

private:
    counted_ptr<SensorFrustumGeometry> m_frustum;
};

}

#endif // _VESTA_SENSOR_VISUALIZER_H_
