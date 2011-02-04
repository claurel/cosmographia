/*
 * $Revision: 418 $ $Date: 2010-08-10 09:07:36 -0700 (Tue, 10 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "CelestialCoordinateGrid.h"
#include "RenderContext.h"
#include "Material.h"
#include "Units.h"
#include <GL/glew.h>
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

    // Draw the meridians
    for (unsigned int i = 0; i < longitudeSteps; ++i)
    {
        double phi = 2.0 * PI * (double) i / (double) longitudeSteps;
        double cosPhi = std::cos(phi);
        double sinPhi = std::sin(phi);

        glBegin(GL_LINE_STRIP);
        for (unsigned int j = 0; j <= circleSubdivisions; ++j)
        {
            double theta = PI * ((double) j / (double) circleSubdivisions - 0.5);
            double sinTheta = std::sin(theta);
            double cosTheta = std::cos(theta);
            Vector3d v(cosPhi * cosTheta, sinPhi * cosTheta, sinTheta);
            glVertex3dv(v.data());
        }
        glEnd();
    }

    // Draw the parallels
    for (unsigned int i = 1; i < latitudeSteps; ++i)
    {
        double theta = PI * ((double) i / (double) latitudeSteps - 0.5);
        double cosTheta = std::cos(theta);
        double sinTheta = std::sin(theta);

        glBegin(GL_LINE_LOOP);
        for (unsigned int j = 0; j < circleSubdivisions; ++j)
        {
            double phi = 2.0 * PI * (double) j / (double) circleSubdivisions;
            double sinPhi = std::sin(phi);
            double cosPhi = std::cos(phi);
            Vector3d v(cosPhi * cosTheta, sinPhi * cosTheta, sinTheta);
            glVertex3dv(v.data());
        }
        glEnd();
    }

    rc.popModelView();
}
