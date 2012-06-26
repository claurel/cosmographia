/*
 * $Revision: 668 $ $Date: 2012-04-24 12:40:30 -0700 (Tue, 24 Apr 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "ConeGeometry.h"
#include "RenderContext.h"
#include "Material.h"
#include "Units.h"
#include "OGLHeaders.h"
#include <Eigen/Geometry>
#include <algorithm>
#include <cmath>

using namespace vesta;
using namespace Eigen;


ConeGeometry::ConeGeometry(double apexAngle, double height) :
    m_apexAngle(apexAngle),
    m_height(height),
    m_color(Spectrum(1.0f, 1.0f, 1.0f)),
    m_opacity(1.0f),
    m_axis(Eigen::Vector3d::UnitZ())
{
}


ConeGeometry::~ConeGeometry()
{
}


void
ConeGeometry::render(RenderContext& rc, double /* clock */) const
{
    rc.setVertexInfo(VertexSpec::Position);

    unsigned int radialSubdivision = 30;
    unsigned int axialSubdivision = 5;
    Vector3f axis = m_axis.cast<float>();
    rc.drawCone(m_apexAngle, axis * float(m_height), m_color, m_opacity, radialSubdivision, axialSubdivision);

    unsigned int ringCount = 6;
    float baseRadius = float(std::tan(m_apexAngle / 2.0) * m_height);

    for (unsigned int ring = 1; ring <= ringCount; ++ring)
    {
        float t = float(ring) / float(ringCount);
        rc.drawCircle(t * baseRadius, float(m_height) * t * axis, axis, m_color, 0.6f, 40);
    }
}


float
ConeGeometry::boundingSphereRadius() const
{
    double baseRadius = tan(m_apexAngle / 2.0) * m_height;
    return (float) std::max(m_height, baseRadius);
}

