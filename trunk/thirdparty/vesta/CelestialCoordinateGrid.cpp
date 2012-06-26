/*
 * $Revision: 674 $ $Date: 2012-05-22 16:35:37 -0700 (Tue, 22 May 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "CelestialCoordinateGrid.h"
#include "RenderContext.h"
#include "GeometryBuffer.h"
#include "Material.h"
#include "Units.h"
#include "OGLHeaders.h"
#include <cmath>

using namespace vesta;
using namespace Eigen;


CelestialCoordinateGrid::CelestialCoordinateGrid() :
    m_frame(InertialFrame),
    m_orientation(Quaterniond::Identity()),
    m_longitudeUnits(Hours),
    m_color(1.0f, 1.0f, 1.0f),
    m_style(LabeledGrid)
{
}


CelestialCoordinateGrid::~CelestialCoordinateGrid()
{
}


void
CelestialCoordinateGrid::render(RenderContext& rc)
{
    const unsigned int circleSubdivisions = 100;
    const unsigned int longitudeStepSec = 10 * 3600;
    const unsigned int latitudeStepSec = 10 * 3600;
    unsigned int longitudeSteps = (360 * 3600) / longitudeStepSec;
    unsigned int latitudeSteps = (180 * 3600) / latitudeStepSec;

    rc.setVertexInfo(VertexSpec::Position);

    Material material;
    material.setDiffuse(m_color);
    rc.bindMaterial(&material);
    glDepthMask(GL_FALSE);

    rc.pushModelView();
    rc.rotateModelView(orientation().cast<float>());

    if (gridStyle() == EquatorOnly)
    {
        longitudeSteps = 0;
        latitudeSteps = 2;
    }

    GeometryBuffer geo(&rc);

    // Draw the meridians
    for (unsigned int i = 0; i < longitudeSteps; ++i)
    {
        double phi = 2.0 * PI * (double) i / (double) longitudeSteps;
        double cosPhi = std::cos(phi);
        double sinPhi = std::sin(phi);

        geo.beginLineStrip();
        for (unsigned int j = 0; j <= circleSubdivisions; ++j)
        {
            double theta = PI * ((double) j / (double) circleSubdivisions - 0.5);
            double sinTheta = std::sin(theta);
            double cosTheta = std::cos(theta);
            Vector3d v(cosPhi * cosTheta, sinPhi * cosTheta, sinTheta);
            geo.vertex(v);
        }
        geo.end();
    }

    // Draw the parallels
    for (unsigned int i = 1; i < latitudeSteps; ++i)
    {
        double theta = PI * ((double) i / (double) latitudeSteps - 0.5);
        double cosTheta = std::cos(theta);
        double sinTheta = std::sin(theta);

        geo.beginLineStrip();
        for (unsigned int j = 0; j <= circleSubdivisions; ++j)
        {
            double phi = 2.0 * PI * (double) j / (double) circleSubdivisions;
            double sinPhi = std::sin(phi);
            double cosPhi = std::cos(phi);
            Vector3d v(cosPhi * cosTheta, sinPhi * cosTheta, sinTheta);
            geo.vertex(v);
        }
        geo.end();
    }

    rc.popModelView();
}
