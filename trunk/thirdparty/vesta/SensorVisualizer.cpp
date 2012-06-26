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
#include "SensorFrustumGeometry.h"
#include "RenderContext.h"
#include "WorldGeometry.h"
#include "Units.h"
#include "Intersect.h"
#include "Debug.h"
#include "OGLHeaders.h"
#include <vector>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;



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
    // For version compatibility reasons, we need to cast between
    // the two versions of FrustumShape
    return (FrustumShape) m_frustum->frustumShape();
}


/** Set the shape of the sensor frustum. Currently, only
  * elliptical and rectangular frustums are supported.
  */
void
SensorVisualizer::setFrustumShape(FrustumShape shape)
{
    // For version compatibility reasons, we need to cast between
    // the two versions of FrustumShape
    m_frustum->setFrustumShape((SensorFrustumGeometry::FrustumShape) shape);
}


/** Set the frustum angles.
  *
  * \param horizontal horizontal frustum angle in radians
  * \param vertical vertical frustum angle in radians
  */
void
SensorVisualizer::setFrustumAngles(double horizontal, double vertical)
{
    m_frustum->setFrustumAngles(horizontal, vertical);
}


// Implementation of Visualizer::orientation()
Eigen::Quaterniond
SensorVisualizer::orientation(const Entity* parent, double t) const
{
    return parent->orientation(t);
}
